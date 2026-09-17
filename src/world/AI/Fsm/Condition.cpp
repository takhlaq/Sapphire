#include "Condition.h"

#include <cstdint>

#include <Util/Util.h>
#include <Util/UtilMath.h>

#include "ForwardsZone.h"

#include <Actor/GameObject.h>
#include <Actor/BNpc.h>
#include <Actor/Player.h>

#include <AI/Controller/Controller.h>
#include <AI/Fsm/StateCombat.h>
#include <AI/Fsm/StateMachine.h>

#include <Service.h>
#include <Manager/TerritoryMgr.h>
#include <Territory/Territory.h>
#include <Territory/InstanceContent.h>


namespace Sapphire::World::AI::Fsm
{
  bool RoamNextTimeReachedCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    auto pBNpc = pEntity->getAsBNpc();
    if( pBNpc && ( Common::Util::getTimeSeconds() - pBNpc->getLastRoamTargetReachedTime() ) > 20 )
      return true;
    return false;
  }

  bool RoamTargetReachedCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    auto pBNpc = pEntity->getAsBNpc();
    if( pBNpc && pBNpc->isRoamTargetReached() )
      return true;
    return false;
  }

  bool HateListEmptyCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    auto pBNpc = pEntity->getAsBNpc();
    if( pBNpc && pBNpc->hateListGetHighest() )
      return false;

    auto pPlayer = pEntity->getAsPlayer();
    if( pPlayer && !pPlayer->getHateList().empty() )
      return false;
    return true;
  }

  bool HateListHasEntriesCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    auto pBNpc = pEntity->getAsBNpc();
    if( pBNpc && pBNpc->hateListGetHighest() )
      return true;

    auto pPlayer = pEntity->getAsPlayer();
    if( pPlayer && !pPlayer->getHateList().empty() )
      return true;

    return false;
  }

  bool SpawnPointDistanceGtMaxDistanceCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    auto pBNpc = pEntity->getAsBNpc();

    if( pBNpc )
    {
      auto distanceOrig = Common::Util::distance( pBNpc->getPos(), pBNpc->getSpawnPos() );
      if( distanceOrig > 40 )
        return true;
    }
    return false;
  }

  bool IsDeadCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    auto pChara = pEntity->getAsChara();
    if( !pChara->isAlive() )
      return true;

    return false;
  }

  bool PathDestinationReachedCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    auto pController = pEntity->getController();
    if( pController )
    {
      auto pBNpc = pEntity->getAsBNpc();
      if( !pBNpc )
        return true;

      const auto& path = pController->getPath();
      if( path.m_type == Controller::PathType::PointList && path.m_currPointIndex >= path.m_points.size() )
        return true;
      else if( path.m_type != Controller::PathType::PointList && Common::Util::distance( pEntity->getPos(), path.m_targetPos ) <= pBNpc->getNaviTargetReachedDistance() )
        return true;
    }

    return false;
  }

  bool FollowTargetReachedCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    if( pEntity->getFollowTargetId() == Common::INVALID_GAME_OBJECT_ID )
      return true;

    if( auto pController = pEntity->getController() )
    {
      auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
      auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );

      if( !pZone )
        return true;

      volatile float maxDist = 2.f;
      if( auto pBNpc = pEntity->getAsBNpc() )
        maxDist = pBNpc->getNaviTargetReachedDistance();

      maxDist = maxDist < 2.f ? 2.f : maxDist;

      if( auto pTarget = pZone->getEntityById( pEntity->getFollowTargetId() ) )
      {
        volatile auto dist = Common::Util::distance2D( pEntity->getPos().x, pEntity->getPos().z, pTarget->getPos().x, pTarget->getPos().z );
        return dist <= maxDist;
      }
    }
    return true;
  }

  bool FollowTargetInvalidCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    if( pEntity->getFollowTargetId() == Common::INVALID_GAME_OBJECT_ID )
      return true;

    if( auto pController = pEntity->getController() )
    {
      auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
      auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );

      if( !pZone )
        return true;

      if( auto pTarget = pZone->getEntityById( pEntity->getFollowTargetId() ) )
        return false;
    }
    return true;
  }

  bool ShouldFollowTargetAlwaysCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    if( pEntity->getFollowTargetId() == Common::INVALID_GAME_OBJECT_ID )
      return false;

    if( auto pController = pEntity->getController() )
    {
      auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
      auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );

      if( !pZone )
        return true;

      float maxDist = 2.f;
      if( auto pBNpc = pEntity->getAsBNpc() )
        maxDist = pBNpc->getNaviTargetReachedDistance();

      maxDist = maxDist < 2.f ? 2.f : maxDist;

      if( auto pTarget = pZone->getEntityById( pEntity->getFollowTargetId() ) )
      {
        volatile auto dist = Common::Util::distance2D( pEntity->getPos().x, pEntity->getPos().z, pTarget->getPos().x, pTarget->getPos().z );
        return dist > maxDist;
      }
    }
    return false;
  }

  bool ShouldFollowTargetOutOfCombatCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    if( pEntity->getFollowTargetId() == Common::INVALID_GAME_OBJECT_ID )
      return false;

    if( auto pController = pEntity->getController() )
    {
      // todo: proper in combat flags?
      if( auto isCombatState = pController->isCurrentState< StateCombat >() )
        return false;

      auto& teriMgr = Common::Service< World::Manager::TerritoryMgr >::ref();
      auto pZone = teriMgr.getTerritoryByGuId( pEntity->getTerritoryId() );

      if( !pZone )
        return true;

      volatile float maxDist = 3.f;
      if( auto pBNpc = pEntity->getAsBNpc() )
        maxDist = pBNpc->getNaviTargetReachedDistance();

      maxDist = maxDist < 3.f ? 3.f : maxDist;

      if( auto pTarget = pZone->getEntityById( pEntity->getFollowTargetId() ) )
      {
        volatile auto dist = Common::Util::distance2D( pEntity->getPos().x, pEntity->getPos().z, pTarget->getPos().x, pTarget->getPos().z );
        return dist > maxDist;
      }
    }
    return false;
  }

  bool ShouldFollowPathCondition::isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const
  {
    if( auto pController = pEntity->getController() )
    {
      const auto& path = pController->getPath();

      return path.m_type != Controller::PathType::None && path.m_type != Controller::PathType::TargetId && Common::Util::distance( pEntity->getPos(), path.m_targetPos ) > 2.f;
    }
    return false;
  }
};// namespace Sapphire::World::AI::Fsm