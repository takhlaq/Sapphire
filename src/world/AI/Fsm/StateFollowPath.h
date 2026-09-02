#include <cstdint>
#include <functional>

#include "ForwardsZone.h"
#include "Actor/BNpc.h"
#include "State.h"

#include <Vector3.h>

#pragma once

namespace Sapphire::World::AI::Fsm
{
  class StateFollowPath : public State
  {
  public:
    StateFollowPath( const std::function< void( Common::Vector3& ) >& onPointReachCb = {},
                    const std::function< void( Common::Vector3& ) >& onDestReachCb = {} )
    {
      m_onPointReachCb = onPointReachCb;
      m_onDestReachCb = onDestReachCb;
    }
    virtual ~StateFollowPath() = default;

    void onUpdate( Entity::GameObjectPtr& pEntity, uint64_t tickCount ) override;
    void onEnter( Entity::GameObjectPtr& pEntity ) override;
    void onExit( Entity::GameObjectPtr& pEntity ) override;

  private:
    std::function< void( Common::Vector3& ) > m_onPointReachCb;
    std::function< void( Common::Vector3& ) > m_onDestReachCb;
  };
}