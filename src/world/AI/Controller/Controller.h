#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include <ForwardsZone.h>
#include <Vector3.h>
#include <Util/Util.h>

#include "../GambitPack.h"
#include "../Fsm/StateMachine.h"
#include "Actor/GameObject.h"

namespace Sapphire::World::AI
{
  class Controller :
    public std::enable_shared_from_this< Controller >
  {
  public:
    Controller( Entity::GameObjectPtr pEntity ) :
      m_pOwner( pEntity ),
      m_lastTick( Common::Util::getTimeMs() ),
      m_stateMachine( pEntity )
    {
    }

    virtual bool tryAggro( uint32_t targetId ) { return false; }
    virtual void aggro( uint32_t targetId, uint32_t hateAmount = 1 ) {}
    virtual void autoAttack( uint32_t targetId ) {}
    virtual void rangedAutoAttack( uint32_t targetId ) {}
    virtual void deaggro() {}
    virtual void clearEnmityList() {}
    virtual void pathTo( Common::Vector3& pos, const std::function< void( Common::Vector3& ) >& onReachPoint = {}, const std::function< void() >& onReachDestination = {} ) {}
    virtual void followPath( const std::vector< Common::Vector3 >& path, const std::function< void( Common::Vector3& ) >& onReachPoint = {}, const std::function< void() > onReachDestination = {} ) {}

    // todo: process gambits here instead of through BNpc::processGambit in StateCombat?
    virtual void processGambits( uint64_t tick ) {}

    GambitPackPtr getGambitPack() const
    {
      return m_pGambitPack;
    }

    void setGambitPack( GambitPackPtr& pGambitPack )
    {
      m_pGambitPack = pGambitPack;
    }

    virtual void update( uint64_t tick )
    {
      m_stateMachine.update( tick );
      m_lastTick = Common::Util::getTimeMs();
    }

  protected:
    uint64_t m_lastTick;
    Fsm::StateMachine m_stateMachine;
    GambitPackPtr m_pGambitPack;
    Entity::GameObjectPtr m_pOwner;
  };

  using ControllerPtr = std::shared_ptr< Controller >;
};