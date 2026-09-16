#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace Sapphire::World::Encounter::Mechanic
{
  enum class MechanicStatus : uint32_t
  {
    Inactive,
    Armed,
    Interrupted,
    Complete
  };
  
  // todo: support vectors instead of one type per arg?
  enum class MechanicArgType : uint32_t
  {
    None,
    UInt64,
    Int64,
    Float,
    String
  };

  struct MechanicArg
  {
    void setUInt( uint64_t val )
    {
      m_type = MechanicArgType::UInt64;
      m_paramUInt = val;
    }

    void setInt( int64_t val )
    {
      m_type = MechanicArgType::Int64;
      m_paramInt = val;
    }

    void setFloat( float val )
    {
      m_type = MechanicArgType::Float;
      m_paramFloat = val;
    }

    void setString( const std::string& val )
    {
      m_type = MechanicArgType::String;
      m_paramStr = val;
    }

    MechanicArgType getType() const
    {
      return m_type;
    }

  protected:
    MechanicArgType m_type{ MechanicArgType::None };
    uint64_t m_paramUInt{ 0 };
    int64_t m_paramInt{ 0 };
    float m_paramFloat{ 0 };
    std::string m_paramStr;
  };

  class MechanicState
  {
  public:
    MechanicState() {}
    ~MechanicState() {}

    void arm( const std::vector< MechanicArg >& args )
    {
      m_args = args;
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

    std::vector< MechanicArg >& getArgs()
    {
      return m_args;
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
    std::vector< MechanicArg > m_args;
  };
};