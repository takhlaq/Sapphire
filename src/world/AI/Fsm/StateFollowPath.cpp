#include "StateFollowPath.h"
#include "Actor/GameObject.h"
#include "Actor/BNpc.h"
#include "Logging/Logger.h"
#include <Service.h>
#include <Manager/TerritoryMgr.h>

#include <Territory/Territory.h>
#include <Navi/NaviProvider.h>

#include <AI/Controller/Controller.h>
#include <AI/Controller/BNpcOverworldController.h>

using namespace Sapphire::World;

void AI::Fsm::StateFollowPath::onUpdate( Entity::GameObjectPtr& pEntity, uint64_t tickCount )
{
  auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
  auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );
  auto pNaviProvider = pZone->getNaviProvider();

  if( auto pController = pEntity->getController() )
  {
    auto elapsed = Common::Util::getTimeMs() - m_lastTick;

    // todo: support pathing for non bnpc?
    auto pBNpc = pEntity->getAsBNpc();

    if( !pBNpc )
      return;

    auto& path = pController->getPath();
    // dont spam path recalc
    if( elapsed < 250 )
      return;

    if( !path.m_active )
    {
      //Logger::error( "Path is not active!" );
      return;
    }

    Common::Vector3 targetPos = pBNpc->getPos();

    // path to target
    if( path.m_type == AI::Controller::PathType::TargetId )
    {
      auto pEntity = pZone->getEntityById( path.m_targetId );
      if( pEntity )
        targetPos = pEntity->getPos();
    }

    if( path.m_type == AI::Controller::PathType::FixedPos )
      targetPos = path.m_targetPos;

    if( path.m_type == AI::Controller::PathType::TargetId || path.m_type == AI::Controller::PathType::FixedPos )
    {
      if( pNaviProvider )
      {
        auto points = pNaviProvider->findFollowPath( pBNpc->getPos(), targetPos );

        auto distance = Common::Util::distance( pBNpc->getPos(), path.m_targetPos );

        if( pBNpc->moveTo( targetPos ) )
          m_onDestReachCb();

        // todo: on point reached cb?
      }
    }
    // follow predefined path
    else if( path.m_type == AI::Controller::PathType::PointList || path.m_type == AI::Controller::PathType::ServerPath )
    {
      auto currPoint = path.m_currPointIndex;
      auto pathSize = path.m_points.size();

      Logger::info( "FollowPath: Pre-adjustment targetPos {} {} {}", targetPos.x, targetPos.y, targetPos.z );

      if( currPoint >= 0 && currPoint < pathSize )
        targetPos = path.m_points[ currPoint ];
      else if( currPoint >= pathSize && pathSize > 0 )
        targetPos = path.m_points[ path.m_points.size() - 1 ];
      else
        targetPos = path.m_points[ 0 ];

      Logger::info( "FollowPath: Post-adjustment targetPos {} {} {}", targetPos.x, targetPos.y, targetPos.z );
      auto currPos = pBNpc->getPos();

      if( pBNpc->moveTo( targetPos ) )
      {
        Logger::info( "FollowPath: Arrived at pos {} {} {}", targetPos.x, targetPos.y, targetPos.z );
        Logger::info( "FollowPath: currPoint {} pathSize {}", currPoint, pathSize );

        if( currPoint <= pathSize && !path.m_isReversePath )
        {
          path.m_currPointIndex++;

          Logger::info( "FollowPath: Advanced currPoint" );

          if( path.m_flags & AI::Controller::PathFlags::CanReversePath && currPoint + 1 >= pathSize )
          {
            path.m_isReversePath = true;
            path.m_currPointIndex = pathSize - 1;
            Logger::info( "FollowPath: Reversing path" );
          }

          if( currPoint + 1 >= pathSize )
          {
            m_onDestReachCb();
            Logger::info( "FollowPath: Reached destination" );
          }
        }
        else if( currPoint >= 0 && path.m_isReversePath )
        {
          path.m_currPointIndex = currPoint - 1;

          if( currPoint - 1 <= 0 )
          {
            path.m_isReversePath = false;
            path.m_currPointIndex = 0;
          }
        }
        targetPos = path.m_points[ path.m_currPointIndex ];

        if( currPoint < pathSize && path.m_prevPointIndex != currPoint )
          m_onPointReachCb( path.m_points[ currPoint ] );

        path.m_prevPointIndex = currPoint;
      }
    }

    // dont move if immobile
    if( pBNpc->hasFlag( Entity::NoRoam ) || pBNpc->hasFlag( Entity::Immobile ) || !pBNpc->pathingActive() )
    {
      pBNpc->setRoamTargetReached( true );
      return;
    }

    // request follow path from navi
    if( pNaviProvider )
    {
      // targetPos = pNaviProvider->findNearestPosition( targetPos.x, targetPos.z );
      pBNpc->moveTo( targetPos );
      pNaviProvider->setMoveTarget( pBNpc->getAgentId(), targetPos );
    }
    pBNpc->setRoamTargetPos( targetPos );
  }

  /*
  if( auto pBNpc = pEntity->getAsBNpc() )
  {
    auto& bnpc = *pBNpc;

    if( bnpc.hasFlag( Entity::NoRoam ) || bnpc.hasFlag( Entity::Immobile ) || !bnpc.pathingActive() )
    {
      bnpc.setRoamTargetReached( true );
      return;
    }

    if( pNaviProvider )
      pNaviProvider->setMoveTarget( bnpc.getAgentId(), bnpc.getRoamTargetPos() );

    auto path = bnpc.getActiveServerPath();
    if( !path )
      onEnter( pEntity );


    if( bnpc.moveTo( bnpc.getRoamTargetPos() ) )
    {
      uint8_t currPoint = bnpc.getActiveServerPathPointIndex();
      auto pathSize = bnpc.getActiveServerPath()->points.size();

      if( currPoint < pathSize && !bnpc.isReversePath() )
      {
        // go to next point
        bnpc.setActiveServerPathPointIndex( currPoint + 1 );

        // Check if we've reached the last point, if so reverse the path
        if( currPoint + 1 >= pathSize )
        {
          // reached end, reverse
          bnpc.setReversePath( true );
          bnpc.setActiveServerPathPointIndex( pathSize - 1 );
        }
      }
      else if( currPoint > 0 && bnpc.isReversePath() )
      {
        // to previous point
        bnpc.setActiveServerPathPointIndex( currPoint - 1 );

        // Check if we've reached the first point, if so reverse the path
        if( currPoint - 1 <= 0 )
        {
          // reached start, reverse
          bnpc.setReversePath( false );
          bnpc.setActiveServerPathPointIndex( 0 );
        }
      }

      if( bnpc.getActiveServerPathPointIndex() >= bnpc.getActiveServerPath()->points.size() )
        return;

      auto path = bnpc.getActiveServerPath();
      if( !path )
        return;

      // set target
      bnpc.setRoamTargetPos( { path->position.x + path->points[ bnpc.getActiveServerPathPointIndex() ].Translation.x,
                               path->position.y + path->points[ bnpc.getActiveServerPathPointIndex() ].Translation.y,
                               path->position.z + path->points[ bnpc.getActiveServerPathPointIndex() ].Translation.z } );
    }
  }
  //*/
  m_lastTick = Common::Util::getTimeMs();
}

void AI::Fsm::StateFollowPath::onEnter( Entity::GameObjectPtr& pEntity )
{
  auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
  auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );
  auto pNaviProvider = pZone->getNaviProvider();

  auto pController = pEntity->getController();

  if( m_initialPathType == static_cast< uint32_t >( Controller::PathType::None ) )
    m_initialPathType = static_cast< uint32_t >( pController->getPath().m_type );

  if( auto pBNpc = pEntity->getAsBNpc() )
  {
    auto& bnpc = *pBNpc;

    auto& path = pController->getPath();

    bnpc.setNaviIsPathing( true );
    bnpc.setPathingActive( true );
    bnpc.setRoamTargetPos( path.m_targetPos );

    if( !pNaviProvider || bnpc.hasFlag( Entity::NoRoam ) || bnpc.hasFlag( Entity::Immobile ) )
    {
      bnpc.setRoamTargetReached( true );
      return;
    }

    if( bnpc.getEnemyType() == Common::Friendly )
    {
      path.reset();

      path.m_type = AI::Controller::PathType::FixedPos;
      path.m_targetPos = bnpc.getSpawnPos();
      path.m_active = true;

      bnpc.setRoamTargetPos( bnpc.getSpawnPos() );
    }
    else if( auto serverPath = pZone->getServerPath( pBNpc->getInstanceObjectInfo()->ServerPathId ) )
    {
      if( path.m_type != Controller::PathType::ServerPath )
        return;

      // restore server path
      if( serverPath->points.empty() )
        return;

      path.reset();

      path.m_active = true;
      path.m_type = AI::Controller::PathType::ServerPath;
      path.m_targetPos = { serverPath->position.x + serverPath->points[ 0 ].Translation.x,
                           serverPath->position.y + serverPath->points[ 0 ].Translation.y,
                           serverPath->position.z + serverPath->points[ 0 ].Translation.z
      };

      for( const auto& p : serverPath->points )
        path.m_points.push_back( {
                serverPath->position.x + p.Translation.x,
                serverPath->position.y + p.Translation.y,
                serverPath->position.z + p.Translation.z
        } );

      bnpc.setRoamTargetPos( path.m_targetPos );
    }
  }
}

void AI::Fsm::StateFollowPath::onExit( Entity::GameObjectPtr& pEntity )
{
  if( auto pBNpc = pEntity->getAsBNpc() )
  {
    pBNpc->setRoamTargetReached( false );
    pBNpc->setNaviIsPathing( false );

    if( auto pController = pBNpc->getController() )
    {
      auto& path = pController->getPath();

      // path.reset();

      // todo: this is a dumb hacky workaround to restore server path..
      if( static_cast< Controller::PathType >( m_initialPathType ) == Controller::PathType::ServerPath )
        path.m_type = static_cast< Controller::PathType >( m_initialPathType );
    }
  }
}
