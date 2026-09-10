#pragma once

#include <cstdint>

#include "State.h"

namespace Sapphire::World::AI::Fsm
{
  class StateFollowTarget : public State
  {
  public:
    StateFollowTarget( uint32_t targetId ) :
      m_targetId( targetId )
    {
    }

    void onEnter( Entity::GameObjectPtr& pEntity ) override;
    void onUpdate( Entity::GameObjectPtr& pEntity, uint64_t tick ) override;
    void onExit( Entity::GameObjectPtr& pEntity ) override;

    void setTargetId( uint32_t targetId );

  private:
    uint32_t m_targetId{ 0xE0000000 };
  };
}