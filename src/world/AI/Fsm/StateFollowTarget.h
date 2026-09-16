#pragma once

#include <cstdint>

#include "State.h"

namespace Sapphire::World::AI::Fsm
{
  class StateFollowTarget : public State
  {
  public:
    StateFollowTarget() {}

    void onEnter( Entity::GameObjectPtr& pEntity ) override;
    void onUpdate( Entity::GameObjectPtr& pEntity, uint64_t tick ) override;
    void onExit( Entity::GameObjectPtr& pEntity ) override;
  };
}