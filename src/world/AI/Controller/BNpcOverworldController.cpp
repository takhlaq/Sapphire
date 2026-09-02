#include "BNpcOverworldController.h"

#include <Territory/Territory.h>
#include <Manager/TerritoryMgr.h>

#include <AI/Fsm/State.h>
#include <AI/Fsm/StateCombat.h>
#include <AI/Fsm/StateFollowPath.h>
#include <AI/Fsm/StateResumePath.h>
#include <AI/Fsm/StateRetreat.h>
#include <AI/Fsm/StateIdle.h>
#include <AI/Fsm/StateRoam.h>
#include <AI/Fsm/StateDead.h>

#include <AI/GambitPack.h>

#include <Actor/BNpc.h>

namespace Sapphire::World::AI
{
  BNpcOverworldController::BNpcOverworldController( Entity::GameObjectPtr pEntity ) :
    Controller( pEntity )
  {
    using namespace AI::Fsm;

    auto pBNpc = pEntity->getAsBNpc();
    auto pBNpcInfo = pBNpc->getInstanceObjectInfo();

    auto stateIdle = make_StateIdle();
    auto stateCombat = make_StateCombat();
    auto stateDead = make_StateDead();

    auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
    auto pZone = teriMgr.getTerritoryByGuId( pBNpc->getTerritoryId() );

    if( pBNpcInfo->ServerPathId != 0 && pZone && pZone->getServerPath( pBNpcInfo->ServerPathId ) )
    {
      auto statePath = make_StateFollowPath();
      auto stateResumePath = make_StateResumePath();
      statePath->addTransition( stateCombat, make_HateListHasEntriesCondition() );
      statePath->addTransition( stateDead, make_IsDeadCondition() );

      stateCombat->addTransition( stateDead, make_IsDeadCondition() );
      stateCombat->addTransition( stateResumePath, make_HateListEmptyCondition() );
      stateResumePath->addTransition( statePath, make_RoamTargetReachedCondition() );

      m_stateMachine.addState( statePath );

      m_stateMachine.setCurrentState( statePath );
    }
    else
    {
      if( !pBNpc->hasFlag( Entity::BNpcFlag::Immobile ) && !pBNpc->hasFlag( Entity::BNpcFlag::NoRoam ) )
      {
        auto stateRoam = make_StateRoam();
        stateIdle->addTransition( stateRoam, make_RoamNextTimeReachedCondition() );
        stateRoam->addTransition( stateIdle, make_RoamTargetReachedCondition() );
        stateRoam->addTransition( stateCombat, make_HateListHasEntriesCondition() );
        stateRoam->addTransition( stateDead, make_IsDeadCondition() );

        m_stateMachine.addState( stateRoam );
      }
      stateIdle->addTransition( stateCombat, make_HateListHasEntriesCondition() );
      //stateCombat->addTransition( stateIdle, make_HateListEmptyCondition() );
      stateIdle->addTransition( stateDead, make_IsDeadCondition() );
      stateCombat->addTransition( stateDead, make_IsDeadCondition() );

      m_stateMachine.addState( stateIdle );

      if( !pBNpc->hasFlag( Entity::BNpcFlag::NoDeaggro ) )
      {
        auto stateRetreat = make_StateRetreat();
        stateCombat->addTransition( stateRetreat, make_SpawnPointDistanceGtMaxDistanceCondition() );
        stateCombat->addTransition( stateRetreat, make_HateListEmptyCondition() );
        stateRetreat->addTransition( stateIdle, make_RoamTargetReachedCondition() );
      }
      m_stateMachine.setCurrentState( stateIdle );
    }
  }

  void BNpcOverworldController::update( uint64_t tick )
  {
    // todo: handle gambits here instead of BNpc::processGambits called by StateCombat?
    /*
    if( m_pGambitPack )
      m_pGambitPack->update( *m_pOwner->getAsBNpc(), tick );
    */
    auto pBNpc = m_pOwner->getAsBNpc();
    pBNpc->checkAggro();
    m_stateMachine.update( tick );
  }

  bool BNpcOverworldController::tryAggro( uint32_t targetId )
  {
    // todo:
    return false;
  }

  void BNpcOverworldController::aggro( uint32_t targetId, uint32_t hateAmount )
  {
    // todo:
  }

  void BNpcOverworldController::autoAttack( uint32_t targetId )
  {
    // todo:
  }

  void BNpcOverworldController::rangedAutoAttack( uint32_t targetId )
  {
    // todo:
  }

  void BNpcOverworldController::deaggro()
  {
    // todo:
  }

  void BNpcOverworldController::clearEnmityList()
  {
    // todo:
  }

  void BNpcOverworldController::pathTo( Common::Vector3& pos, const std::function< void(Common::Vector3&) >& onReachPoint, const std::function< void() >& onReachDestination )
  {
    // todo:
  }

  void BNpcOverworldController::followPath( const std::vector< Common::Vector3 >& path, const std::function< void(Common::Vector3&) >& onReachPoint, const std::function< void() > onReachDestination )
  {
    // todo:
  }

}