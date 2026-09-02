#include <cstdint>

#include <Util/Util.h>
#include <Util/UtilMath.h>

#include "ForwardsZone.h"

#include <Actor/GameObject.h>
#include <Actor/BNpc.h>
#include <Actor/Player.h>

#pragma once

namespace Sapphire::World::AI::Fsm
{
  class Condition
  {
  public:
    Condition() = default;
    virtual ~Condition() = default;

    virtual bool isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const = 0;
    virtual bool update( Sapphire::Entity::GameObjectPtr& pEntity, float time )
    {
      if( isConditionMet( pEntity ) )
        return true;
      return false;
    };
  };

  class RoamNextTimeReachedCondition : public Condition
  {
  public:
    bool isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const override
    {
      auto pBNpc = pEntity->getAsBNpc();
      if( pBNpc && ( Common::Util::getTimeSeconds() - pBNpc->getLastRoamTargetReachedTime() ) > 20 )
        return true;
      return false;
    }
  };

  class RoamTargetReachedCondition : public Condition
  {
  public:
    bool isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const override
    {
      auto pBNpc = pEntity->getAsBNpc();
      if( pBNpc && pBNpc->isRoamTargetReached() )
        return true;
      return false;
    }
  };

  class HateListEmptyCondition : public Condition
  {
  public:
    bool isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const override
    {
      auto pBNpc = pEntity->getAsBNpc();
      if( pBNpc && pBNpc->hateListGetHighest() )
        return false;

      auto pPlayer = pEntity->getAsPlayer();
      if( pPlayer && !pPlayer->getHateList().empty() )
        return false;
      return true;
    }
  };

  class HateListHasEntriesCondition : public Condition
  {
  public:
    bool isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const override
    {
      auto pBNpc = pEntity->getAsBNpc();
      if( pBNpc && pBNpc->hateListGetHighest() )
        return true;

      auto pPlayer = pEntity->getAsPlayer();
      if( pPlayer && !pPlayer->getHateList().empty() )
        return true;

      return false;
    }
  };

  class SpawnPointDistanceGtMaxDistanceCondition : public Condition
  {
  public:
    bool isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const override
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
  };

  class IsDeadCondition : public Condition
  {
  public:
    bool isConditionMet( Sapphire::Entity::GameObjectPtr& pEntity ) const override
    {
      auto pChara = pEntity->getAsChara();
      if( !pChara->isAlive() )
        return true;

      return false;
    }
  };

}