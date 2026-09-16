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
  if( !pZone )
    return;

  auto pNaviProvider = pZone->getNaviProvider();

  if( auto pController = pEntity->getController() )
  {
    // todo: support pathing for non bnpc?
    auto pBNpc = pEntity->getAsBNpc();

    if( !pBNpc )
      return;

    auto& path = pController->getPath();
    if( !path.m_active )
      return;

    const auto now = Common::Util::getTimeMs();
    const auto elapsed = now - m_lastTick;
    const bool ignoreNavmesh = path.m_flags & AI::Controller::PathFlags::IgnoreNavmesh;

    // Navi target updates do not need to be requested every actor tick. Direct
    // paths do, because the controller itself advances the actor position.
    if( !ignoreNavmesh && elapsed < 250 )
      return;

    if( pBNpc->hasFlag( Entity::NoRoam ) || pBNpc->hasFlag( Entity::Immobile ) || !pBNpc->pathingActive() )
    {
      pBNpc->setRoamTargetReached( true );
      return;
    }

    if( !ignoreNavmesh && ( !pNaviProvider || pBNpc->getAgentId() == -1 ) )
      return;

    const auto moveDirectly = [ & ]( const Common::Vector3& destination )
    {
      const auto currentPos = pBNpc->getPos();
      const auto delta = destination - currentPos;
      const auto distance = delta.length();
      const auto step = pBNpc->getCurrentSpeed() * static_cast< float >( elapsed ) / 1000.f;

      pBNpc->face( destination );
      if( distance <= pBNpc->getNaviTargetReachedDistance() || step >= distance )
      {
        pBNpc->setPos( destination );
        return true;
      }

      if( step > 0.f )
        pBNpc->setPos( currentPos + delta.normalize() * step );
      return false;
    };

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

    bool reachedTarget = false;
    if( path.m_type == AI::Controller::PathType::TargetId || path.m_type == AI::Controller::PathType::FixedPos )
    {
      reachedTarget = ignoreNavmesh ? moveDirectly( targetPos ) : pBNpc->moveTo( targetPos );
      if( reachedTarget )
      {
        path.m_active = false;
        if( m_onDestReachCb )
          m_onDestReachCb();
      }
    }
    // follow predefined path
    else if( path.m_type == AI::Controller::PathType::PointList || path.m_type == AI::Controller::PathType::ServerPath )
    {
      const auto pathSize = path.m_points.size();
      if( pathSize == 0 || path.m_currPointIndex >= pathSize )
      {
        path.m_active = false;
        path.m_currPointIndex = static_cast< uint32_t >( pathSize );
        return;
      }

      const auto currPoint = path.m_currPointIndex;

      Logger::info( "FollowPath: Pre-adjustment targetPos {} {} {}", targetPos.x, targetPos.y, targetPos.z );
      targetPos = path.m_points[ currPoint ];

      Logger::info( "FollowPath: Post-adjustment targetPos {} {} {}", targetPos.x, targetPos.y, targetPos.z );
      reachedTarget = ignoreNavmesh ? moveDirectly( targetPos ) : pBNpc->moveTo( targetPos );
      if( reachedTarget )
      {
        Logger::info( "FollowPath: Arrived at pos {} {} {}", targetPos.x, targetPos.y, targetPos.z );
        Logger::info( "FollowPath: currPoint {} pathSize {}", currPoint, pathSize );

        if( m_onPointReachCb && path.m_prevPointIndex != currPoint )
          m_onPointReachCb( path.m_points[ currPoint ] );
        path.m_prevPointIndex = currPoint;

        if( !path.m_isReversePath )
        {
          if( currPoint + 1 < pathSize )
            path.m_currPointIndex = currPoint + 1;
          else if( path.m_flags & AI::Controller::PathFlags::CanReversePath && pathSize > 1 )
          {
            path.m_isReversePath = true;
            path.m_currPointIndex = static_cast< uint32_t >( pathSize - 2 );
            Logger::info( "FollowPath: Reversing path" );
          }
          else
          {
            path.m_currPointIndex = static_cast< uint32_t >( pathSize );
            path.m_active = false;
            if( m_onDestReachCb )
              m_onDestReachCb();
            Logger::info( "FollowPath: Reached destination" );
            m_lastTick = now;
            return;
          }
        }
        else if( currPoint > 0 )
          path.m_currPointIndex = currPoint - 1;
        else
        {
          path.m_isReversePath = false;
          path.m_currPointIndex = pathSize > 1 ? 1 : 0;
        }

        targetPos = path.m_points[ path.m_currPointIndex ];
      }
    }

    if( !ignoreNavmesh && !reachedTarget )
      pNaviProvider->setMoveTarget( pBNpc->getAgentId(), targetPos );

    pBNpc->setRoamTargetPos( targetPos );
    m_lastTick = now;
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
}

void AI::Fsm::StateFollowPath::onEnter( Entity::GameObjectPtr& pEntity )
{
  auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
  auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );
  if( !pZone )
    return;

  auto pNaviProvider = pZone->getNaviProvider();

  auto pController = pEntity->getController();
  if( !pController )
    return;

  if( m_initialPathType == static_cast< uint32_t >( Controller::PathType::None ) )
    m_initialPathType = static_cast< uint32_t >( pController->getPath().m_type );

  if( auto pBNpc = pEntity->getAsBNpc() )
  {
    auto& bnpc = *pBNpc;

    auto& path = pController->getPath();

    bnpc.setNaviIsPathing( true );
    bnpc.setPathingActive( true );
    bnpc.setRoamTargetPos( path.m_targetPos );

    const bool ignoreNavmesh = path.m_flags & Controller::PathFlags::IgnoreNavmesh;
    if( ( !ignoreNavmesh && !pNaviProvider ) || bnpc.hasFlag( Entity::NoRoam ) || bnpc.hasFlag( Entity::Immobile ) )
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
