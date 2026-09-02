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
  auto& pCurrentState = m_pCurrentState;

  // todo: support forcing new state, then resuming old one (e.g. follow scripted path during combat, reach dest, transition back to combat)
  /*
  if( !m_stateStack.empty() )
  {
    pCurrentState = m_stateStack.front();
    m_stateStack.pop_front();
  }
  //*/
  if( !pCurrentState )
    return;

  TransitionPtr transition = pCurrentState->getTriggeredTransition( m_pOwner );

  if( transition )
  {
    pCurrentState->onExit( m_pOwner );
    pCurrentState = transition->getTargetState();
    pCurrentState->onEnter( m_pOwner );
  }

  pCurrentState->onUpdate( m_pOwner, tickCount );
}
