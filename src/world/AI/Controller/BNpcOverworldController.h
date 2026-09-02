#pragma once

#include "Controller.h"

namespace Sapphire::World::AI
{
  class BNpcOverworldController : public Controller
  {
  public:
    BNpcOverworldController( Entity::GameObjectPtr pEntity );


    bool tryAggro( uint32_t targetId ) override;
    void aggro( uint32_t targetId, uint32_t hateAmount = 1 ) override;
    void autoAttack( uint32_t targetId ) override;
    void rangedAutoAttack( uint32_t targetId ) override;
    void deaggro() override;
    void clearEnmityList() override;
    void pathTo( Common::Vector3& pos, const std::function< void( Common::Vector3& ) >& onReachPoint = {}, const std::function< void() >& onReachDestination = {} ) override;
    void followPath( const std::vector< Common::Vector3 >& path, const std::function< void( Common::Vector3& ) >& onReachPoint = {}, const std::function< void() > onReachDestination = {} ) override;

    void update( uint64_t tick ) override;

  private:
    Entity::BNpcPtr m_pBNpc;
    uint64_t m_deathTime{ 0 };
    uint64_t m_respawnTime{ 0 };
  };

  using BNpcOverworldControllerPtr = std::shared_ptr< BNpcOverworldController >;
}
