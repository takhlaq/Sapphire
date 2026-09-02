#include <cstdint>

#include <Util/Util.h>

#include "ForwardsZone.h"

#include "Actor/BNpc.h"
#include "Actor/GameObject.h"
#include "Transition.h"


#pragma once

namespace Sapphire::World::AI::Fsm
{
  class State
  {
  public:
    State() :
      m_lastTick( Common::Util::getTimeMs() )
    {

    }
    virtual ~State() = default;

    virtual void onUpdate( Entity::GameObjectPtr& pEntity, uint64_t tickCount ) { }
    virtual void onEnter( Entity::GameObjectPtr& pEntity ) { }
    virtual void onExit( Entity::GameObjectPtr& pEntity ) { }

    void addTransition( TransitionPtr transition )
    {
      m_transitions.push_back( transition );
    }

    void addTransition( StatePtr targetState, ConditionPtr condition )
    {
      m_transitions.push_back( make_Transition( targetState, condition ) );
    }


    TransitionPtr getTriggeredTransition( Entity::GameObjectPtr& pObject )
    {
      for( auto& transition : m_transitions )
      {
        if( transition->hasTriggered( pObject ) )
          return transition;
      }
      return nullptr;
    }

  private:
    uint64_t m_lastTick;
    std::vector< TransitionPtr > m_transitions;
  };
}