#include <cstdint>

#include "ForwardsZone.h"
#include "Actor/BNpc.h"
#include "StateMachine.h"
#include "State.h"

using namespace Sapphire;
using namespace Sapphire::World;

AI::Fsm::StatePtr AI::Fsm::StateMachine::addState( Fsm::StatePtr state )
{
  m_states.push_back( state );
  return state;
}

AI::Fsm::StatePtr AI::Fsm::StateMachine::getCurrentState()
{
  return m_pCurrentState;
}

void AI::Fsm::StateMachine::setCurrentState( Fsm::StatePtr state )
{
  m_pCurrentState = state;
}

void AI::Fsm::StateMachine::update( uint64_t tickCount )
{
  if( !m_pCurrentState )
    return;

  // make sure we run at least once
  if( !m_pCurrentState->hasInitialised() )
  {
    m_pCurrentState->onEnter( m_pOwner );
    m_pCurrentState->setInitialised( true );
  }

  m_pCurrentState->onUpdate( m_pOwner, tickCount );

  TransitionPtr transition = m_pCurrentState->getTriggeredTransition( m_pOwner );
  volatile TransitionPtr pTransition2 = transition;

  if( transition )
  {
    m_pCurrentState->onExit( m_pOwner );
    m_pCurrentState->setInitialised( false );

    m_pPrevState = m_pCurrentState;
    m_pCurrentState = transition->getTargetState();

    m_pCurrentState->onEnter( m_pOwner );
    m_pCurrentState->setInitialised( true );
    m_pCurrentState->onUpdate( m_pOwner, tickCount );
  }
}
