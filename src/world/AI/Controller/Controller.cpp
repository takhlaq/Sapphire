#include "Controller.h"

#include "../GambitPack.h"
#include "../Fsm/StateMachine.h"

#include <Actor/GameObject.h>
#include <Actor/Chara.h>
#include <Actor/BNpc.h>
#include <Actor/Player.h>

#include <AI/Fsm/StateCombat.h>
#include <AI/Fsm/StateDead.h>
#include <AI/Fsm/StateFollowPath.h>
#include <AI/Fsm/StateFollowTarget.h>

#include <Navi/NaviProvider.h>

#include <Service.h>
#include <Manager/TerritoryMgr.h>
#include <Territory/Territory.h>

#include <Logging/Logger.h>

namespace Sapphire::World::AI
{

  bool Controller::tryAggro( uint32_t targetId )
  {
    return false;
  }

  void Controller::aggro( uint32_t targetId, uint32_t hateAmount )
  {
  }

  void Controller::autoAttack( uint32_t targetId )
  {
  }

  void Controller::rangedAutoAttack( uint32_t targetId )
  {
  }

  void Controller::deaggro()
  {
  }

  void Controller::clearEnmityList()
  {
  }

  void Controller::pathTo( const Common::Vector3& pos, PathFlags flags, const std::function< void( Common::Vector3 ) >& onReachPoint, const std::function< void() >& onReachDestination )
  {
    auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
    auto pTeri = teriMgr.getTerritoryByGuId( m_pOwner->getTerritoryId() );

    if( !pTeri )
    {
      // todo:
      return;
    }

    auto pNavi = pTeri->getNaviProvider();
    if( !pNavi )
    {
      // todo:
      return;
    }

    m_path.reset();

    m_path.m_type = PathType::FixedPos;
    m_path.m_targetPos = pos;
    m_path.m_flags = flags;
    m_path.m_points = pNavi->findFollowPath( m_pOwner->getPos(), pos );
    m_path.m_active = true;

    if( !( flags & PathFlags::IgnoreNavmesh ) )
    {
      Logger::info( "Pre-adjustment TargetPos: {} {} {}", m_path.m_targetPos.x, m_path.m_targetPos.y, m_path.m_targetPos.z );
      m_path.m_targetPos = pNavi->findNearestPosition( pos.x, pos.y, pos.z );
      Logger::info( "TargetPos: {} {} {}", m_path.m_targetPos.x, m_path.m_targetPos.y, m_path.m_targetPos.z );
    }

    auto pCurrState = m_stateMachine.getCurrentState();
    auto pPathState = std::make_shared< Fsm::StateFollowPath >( onReachPoint, onReachDestination );

    // transition back to current state on reaching destination
    auto pTransition = std::make_shared< Fsm::Transition >( pCurrState, std::make_shared< Fsm::PathDestinationReachedCondition >() );
    pPathState->addTransition( pTransition );

    if( flags & PathFlags::Interruptible )
    {
      pPathState->addTransition( std::make_shared< Fsm::Transition >( pCurrState, std::make_shared< Fsm::HateListHasEntriesCondition >() ) );
    }
    m_stateMachine.setCurrentState( pPathState );
  }

  void Controller::followPath( const std::vector< Common::Vector3 >& path, PathFlags flags, const std::function< void( Common::Vector3 ) >& onReachPoint, const std::function< void() >& onReachDestination )
  {
    if( path.empty() )
      return;

    auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
    auto pTeri = teriMgr.getTerritoryByGuId( m_pOwner->getTerritoryId() );

    if( !pTeri )
    {
      // todo:
      return;
    }

    auto pNavi = pTeri->getNaviProvider();
    if( !pNavi )
    {
      // todo:
      return;
    }

    m_path.reset();

    m_path.m_type = PathType::PointList;
    m_path.m_targetPos = path[ path.size() - 1 ];
    m_path.m_flags = flags;
    m_path.m_points = path;
    m_path.m_active = true;

    Logger::info( "BNpc {} X:{} Y:{} Z:{}", m_pOwner->getId(), m_pOwner->getPos().x, m_pOwner->getPos().y, m_pOwner->getPos().z );

    float radius = m_pOwner->isBattleNpc() ? m_pOwner->getAsBNpc()->getRadius() : 1.f;

    if( !( flags & PathFlags::IgnoreNavmesh ) )
    {
      for( auto& pos : m_path.m_points )
      {

        Logger::info( "Pre-adjustment pos: X:{} Y:{} Z:{}", pos.x, pos.y, pos.z );
        auto pos2 = pNavi->findNearestPosition( pos.x, pos.y, pos.z );
        auto path = pNavi->findFollowPath( m_pOwner->getPos(), pos, radius );
        auto pos3 = path[ path.size() - 1 ];
        Logger::info( "FindFollowPath {} {} {}", pos3.x, pos3.y, pos3.z );
        Logger::info( "Post-adjustment pos: X:{} Y:{} Z:{}", pos2.x, pos2.y, pos2.z );
        pos = pos2;
      }
      m_path.m_targetPos = m_path.m_points[ m_path.m_points.size() - 1 ];

      Logger::info( "TargetPos: {} {} {}", m_path.m_targetPos.x, m_path.m_targetPos.y, m_path.m_targetPos.z );
    }

    auto pCurrState = m_stateMachine.getCurrentState();
    auto pPathState = std::make_shared< Fsm::StateFollowPath >( onReachPoint, onReachDestination );

    // transition back to current state on reaching destination
    auto pTransition = std::make_shared< Fsm::Transition >( pCurrState, std::make_shared< Fsm::PathDestinationReachedCondition >() );
    pPathState->addTransition( pTransition );

    if( flags & PathFlags::Interruptible )
    {
      pPathState->addTransition( std::make_shared< Fsm::Transition >( pCurrState, std::make_shared< Fsm::HateListHasEntriesCondition >() ) );
    }

    m_stateMachine.setCurrentState( pPathState );
  }

  void Controller::followTarget( uint32_t targetId, bool followDuringCombat )
  {
    // todo: follow target state

    m_pOwner->setFollowTargetId( targetId );

    if( targetId == Common::INVALID_GAME_OBJECT_ID )
      return;

    auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
    auto pZone = teriMgr.getTerritoryByGuId( m_pOwner->getTerritoryId() );

    if( !pZone )
      return;

    auto pTarget = pZone->getEntityById( targetId );
    if( !pTarget )
      return;

    auto pCurrState = m_stateMachine.getCurrentState();
    if( pCurrState && !pCurrState->hasTransitionToState< Fsm::StateFollowTarget >() )
    {
      bool isCombatState = m_stateMachine.isCurrentState< Fsm::StateCombat >();
      bool isDeathState = m_stateMachine.isCurrentState< Fsm::StateDead >();
      bool isPathState = m_stateMachine.isCurrentState< Fsm::StateFollowPath >();
      bool isFollowState = m_stateMachine.isCurrentState< Fsm::StateFollowTarget >();

      bool createTransition = false;
      if( !isDeathState && !isPathState && !isFollowState && !pCurrState->hasTransitionToState< Fsm::StateFollowTarget >() )
        createTransition = true;

      // todo: disallow following during combat?
      // if( createTransition )
      if( !isFollowState )
      {
        auto pFollowState = std::make_shared< Fsm::StateFollowTarget >( targetId );
        auto pTransitionBack = std::make_shared< Fsm::Transition >( pCurrState, std::make_shared< Fsm::FollowTargetReachedCondition >() );

        // transition back to current state on reaching target
        pFollowState->addTransition( pTransitionBack );

        // leash if too far from spawnpoint
        // todo: dont leash if pet or in quest battle/fate/dungeon
        pTransitionBack = std::make_shared< Fsm::Transition >( pCurrState, std::make_shared< Fsm::SpawnPointDistanceGtMaxDistanceCondition >() );
        pFollowState->addTransition( pTransitionBack );

        m_stateMachine.setCurrentState( pFollowState );
      }
    }
  }

  void Controller::processGambits( uint64_t tick )
  {
  }

  Controller::Path& Controller::getPath()
  {
    return m_path;
  }

  void Controller::setPath( const Controller::Path& path )
  {
    m_path = path;
  }

  GambitPackPtr Controller::getGambitPack() const
  {
    return m_pGambitPack;
  }

  void Controller::setGambitPack( GambitPackPtr& pGambitPack )
  {
    m_pGambitPack = pGambitPack;
  }

  Fsm::StatePtr Controller::getCurrentState()
  {
    return m_stateMachine.getCurrentState();
  }

  void Controller::update( uint64_t tick )
  {
    auto elapsed = Common::Util::getTimeMs() - m_lastTick;

    m_stateMachine.update( tick );

    if( m_pOwner->getFollowTargetId() != Common::INVALID_GAME_OBJECT_ID )
      followTarget( m_pOwner->getFollowTargetId(), true );
    
    m_lastTick = Common::Util::getTimeMs();
  }


}// namespace Sapphire::World::AI