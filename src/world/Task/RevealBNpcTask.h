#pragma once

#include "Task.h"

namespace Sapphire::World
{
  class RevealBNpcTask : public Task
  {
  public:
    RevealBNpcTask( uint64_t delayTime, Entity::BNpcPtr pBNpc );
    void onQueue() override;
    void execute() override;
    std::string toString() override;

  private:
    Entity::BNpcPtr m_pBNpc;
  };
}
