#pragma once

#include <memory>

namespace Sapphire::World::Encounter
{
  class Selector;
  class TimelineActor;

  class Schedule;
  class Timepoint;

  class Phase;
  class Trigger;
  class TriggerAction;
  class TriggerCondition;

  using PhasePtr = std::shared_ptr< Phase >;
  using TriggerPtr = std::shared_ptr< Trigger >;
  using TriggerActionPtr = std::shared_ptr< TriggerAction >;
  using TriggerConditionPtr = std::shared_ptr< TriggerCondition >;

  class Encounter;
  class TimelinePack;

  using EncounterPtr = std::shared_ptr< Encounter >;
  using TimelinePackPtr = std::shared_ptr< TimelinePack >;

  namespace Mechanic
  {
    class Mechanic;
    using MechanicPtr = std::shared_ptr< Mechanic >;
  }// namespace Mechanic
}