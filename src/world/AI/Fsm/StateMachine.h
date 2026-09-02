#include <cstdint>
#include <vector>
#include <queue>

#include "ForwardsZone.h"

#include "State.h"

#include "Actor/GameObject.h"
#include "Actor/BNpc.h"

#pragma once

namespace Sapphire::World::AI::Fsm
{
  class StateMachine
  {
  public:
    StateMachine( Entity::GameObjectPtr& pOwner ) :
      m_pOwner( pOwner )
    {
    }
    ~StateMachine() = default;

    StatePtr addState( StatePtr state );

    StatePtr getCurrentState();
    void setCurrentState( StatePtr state );

    void forceChangeState( StatePtr pState )
    {
      m_stateStack.emplace_front( pState );
    }

    virtual void update( uint64_t tickCount );

  protected:
    std::deque< StatePtr > m_stateStack;
    Entity::GameObjectPtr m_pOwner;
    std::vector< StatePtr > m_states;
    StatePtr m_pRestoreState;
    StatePtr m_pCurrentState;
  };
}