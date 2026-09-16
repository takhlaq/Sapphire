#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

namespace Sapphire::World::Encounter::Mechanic
{
  enum class MechanicStatus : uint32_t
  {
    Inactive,
    Armed,
    Interrupted,
    Complete
  };

  class MechanicState
  {
  public:
    MechanicState() {}
    ~MechanicState() {}

    void arm()
    {
      m_status = MechanicStatus::Armed;
      m_startTime = Common::Util::getTimeMs();
    }

    void complete()
    {
      m_status = MechanicStatus::Complete;
      m_completeTime = Common::Util::getTimeMs();
    }

    void interrupt( bool overrideIfComplete = false )
    {
      if( m_status == MechanicStatus::Complete )
      {
        if( overrideIfComplete )
        {
          m_status = MechanicStatus::Interrupted;
          m_interruptTime = Common::Util::getTimeMs();
        }
        return;
      }
      m_status = MechanicStatus::Interrupted;
      m_interruptTime = Common::Util::getTimeMs();
    }

    void setLastTick( uint64_t tick )
    {
      m_lastTick = tick;
    }

    MechanicStatus getStatus() const
    {
      return m_status;
    }

    uint64_t getStartTime() const
    {
      return m_startTime;
    }

    uint64_t getLastTick() const
    {
      return m_lastTick;
    }

    uint64_t getInterruptTime() const
    {
      return m_interruptTime;
    }

    uint64_t getCompleteTime() const
    {
      return m_completeTime;
    }

  protected:
    MechanicStatus m_status{ MechanicStatus::Inactive };
    uint64_t m_startTime{ 0 };
    uint64_t m_lastTick{ 0 };
    uint64_t m_interruptTime{ 0 };
    uint64_t m_completeTime{ 0 };
  };
};