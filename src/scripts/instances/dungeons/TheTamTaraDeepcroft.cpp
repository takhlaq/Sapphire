#include <ScriptObject.h>
#include <Territory/InstanceContent.h>
#include <Encounter/Encounter.h>
#include <Actor/EventObject.h>
#include <Actor/BNpc.h>
#include <Actor/Player.h>
#include <Util/UtilMath.h>
#include <Manager/EventMgr.h>
#include <Manager/WarpMgr.h>
#include <array>

using namespace Sapphire;
using namespace Sapphire::World::Encounter;

class TheTamTaraDeepcroft : public Sapphire::ScriptAPI::InstanceContentScript
{
private:
  enum DirectorVar : uint8_t
  {
    SharedGroups, Rosary, BrokenBarrier, DestroyedOrbs, VisibleObjectives
  };

  enum InstanceVar : uint32_t
  {
    CurrentAltar, SoulcountersDefeated, RosaryGuardsDefeated, WipePending, GalvanthScene
  };

  struct Altar
  {
    const char* key;
    std::initializer_list< uint32_t > guards;
    uint32_t soulcounter;
    uint32_t placeName;
    uint32_t seal;
    uint32_t marker;
  };

  static constexpr std::array< Altar, 3 > Altars = {{
    { "Altar1", { 1334952, 1334953, 1334954 }, 3859972, 535, 2001699, 2001700 },
    { "Altar2", { 1334955, 1334956, 1334957, 1334959 }, 3859980, 539, 2001701, 2001702 },
    { "Altar3", { 1334960, 1334961, 1334962, 1334963, 1334965 }, 3859984, 541, 2001703, 2001704 }
  }};
  static constexpr std::array< uint32_t, 4 > RosaryGuards = { 2373665, 2373666, 2373667, 2380947 };
  static constexpr std::array< uint32_t, 4 > OrbIds = { 2000061, 2000062, 2000063, 2000067 };
  static constexpr uint32_t BossIntro = 26;
  static constexpr uint32_t BossEnding = 27;
  static constexpr uint32_t Galvanth = 1332316;

  void announce( InstanceContent& instance, uint32_t message, std::initializer_list< uint32_t > args = {} )
  {
    const auto& players = instance.getPlayers();
    if( !players.empty() )
      instance.sendEventLogMessage( *players.begin()->second, instance, message, args );
  }

  void publishObjectives( InstanceContent& instance )
  {
    for( const auto& entry : instance.getPlayers() )
      instance.sendDirectorVars( *entry.second );
  }

  void setGate( InstanceContent& instance, uint32_t gateId, bool closed )
  {
    auto gate = instance.getEObjByBaseId( gateId );
    gate->setPermissionInvisibility( closed ? 0 : 7 );
    gate->setCollisionEnabled( closed );
  }

  void setEncounterEntrance( EncounterDefinition& def, InstanceContent& instance, uint32_t gateId, uint32_t placeName )
  {
    auto gate = instance.getEObjByBaseId( gateId );
    def.hasLockout = true;
    def.placeName = placeName;
    def.entrances = {
      { gate->getName(), gateId, gate->getBoundInstanceId(), gate->getInstanceId(), gate->getState(),
        gate->getPos(), gate->getScale(), gate->getRot(), 7, gate->getAnimation(), false, EncounterEntityRemoveFlag::None }
    };
  }

  void registerAltar( InstanceContent& instance, const Altar& altar )
  {
    EncounterDefinition def{};
    def.key = altar.key;
    def.timeline = "dungeons/tamtara/" + def.key;
    def.shape = EncounterShape::POLYGON;
    def.shapeFileName = def.timeline;

    for( auto layoutId : altar.guards )
      def.participants.push_back( { layoutId, layoutId == *altar.guards.begin() ? 516u : 577u,
                                    Common::BNpcType::Enemy, 0, EncounterBNpcCompletionRole::Required } );

    def.bgmInCombat = 37;
    setEncounterEntrance( def, instance, altar.seal, altar.placeName );
    instance.registerEncounterDefinition( def, instance.getAsInstanceContent() );
  }

  Entity::BNpcPtr getEngagedGuard( const World::Encounter::Encounter& encounter, const Altar& altar )
  {
    for( auto layoutId : altar.guards )
    {
      auto guard = encounter.getBNpc( layoutId );
      if( guard && ( !guard->isAlive() || guard->hateListGetHighestValue() > 0 ) )
        return guard;
    }
    return nullptr;
  }

  void updateAltars( InstanceContent& instance, uint64_t tickCount )
  {
    auto current = instance.getCustomVar( CurrentAltar );
    if( !current )
    {
      for( size_t i = 0; i < Altars.size(); ++i )
      {
        const auto& altar = Altars[ i ];
        auto encounter = instance.getEncounter( altar.key );
        if( encounter->getStatus() != EncounterStatus::IDLE )
          continue;

        if( auto guard = getEngagedGuard( *encounter, altar ) )
        {
          if( auto target = guard->hateListGetHighest() )
            encounter->onEnterRange( target );
          encounter->start();
          current = i + 1;
          instance.setCustomVar( CurrentAltar, current );
          break;
        }
      }
    }
    if( current )
    {
      auto encounter = instance.getEncounter( Altars[ current - 1 ].key );
      encounter->update( tickCount );
      if( encounter->getStatus() != EncounterStatus::ACTIVE )
      {
        instance.setCustomVar( CurrentAltar, 0 );
        instance.setCurrentBGM( 0 );
      }
    }
  }

  void updateGalvanthScene( InstanceContent& instance )
  {
    auto scene = instance.getCustomVar( GalvanthScene );
    if( !scene )
      return;

    auto director = instance.getDirectorId();
    bool finished = false;
    bool pending = false;
    for( const auto& [ id, player ] : instance.getPlayers() )
    {
      if( instance.getCustomVar( id ) == scene )
      {
        if( !player->getEvent( director ) )
          finished = true;
        continue;
      }
      if( player->getEventCount() )
      {
        pending = true;
        continue;
      }

      instance.setCustomVar( id, scene );
      eventMgr().eventStart( *player, id, director, Event::EventHandler::GameProgress, 1, 1 );
      if( scene == BossIntro )
        eventMgr().playScene( *player, director, 3, 0x08502403, { BossIntro, 1, 38, 1 }, nullptr );
      else
        eventMgr().playScene( *player, director, 3, 0x08502403, { BossEnding, 1, 18, 0 }, nullptr );
    }

    if( scene == BossEnding || !finished )
      return;

    if( !pending )
      instance.setCustomVar( GalvanthScene, 0 );
    if( instance.getEncounter( "Galvanth" ) )
      return;

    EncounterDefinition encounter{};
    encounter.key = "Galvanth";
    encounter.timeline = "dungeons/tamtara/Galvanth";
    encounter.shape = EncounterShape::POLYGON;
    encounter.shapeFileName = "dungeons/tamtara/Galvanth";
    setEncounterEntrance( encounter, instance, 2001741, 543 );
    encounter.participants = {
      { Galvanth, 6329, Common::BNpcType::Enemy, Entity::NoRoam | Entity::AutoAttackDisabled,
        EncounterBNpcCompletionRole::Required }
    };
    instance.registerEncounterDefinition( encounter, instance.getAsInstanceContent() )->init();
  }

public:
  TheTamTaraDeepcroft() : Sapphire::ScriptAPI::InstanceContentScript( 2 )
  { }

  void onInit( InstanceContent& instance ) override
  {
    Entity::EventObjectPtr pEObj;

    pEObj = instance.addEObj( "Cultistorb", 2000061, 3816929, 1958519, 0, { -8.115554f, 31.427530f, -15.950320f }, 1.000000f, 1.57031696f, 0 );
    // States -> vf_beam1_on (id: 10) vf_beam1_off (id: 11) all_off (id: 22) 

    pEObj = instance.addEObj( "Cultistorb_1", 2000062, 3817025, 1958523, 0, { -22.784361f, 25.526300f, 20.482220f }, 1.000000f, 0.62576829f, 0 );
    // States -> vf_beam2_off (id: 5) vf_beam2_on (id: 6) all_off (id: 16) 

    pEObj = instance.addEObj( "Exit", 2000139, 0, 1958526, 0, { -42.771080f, 14.067750f, -17.288521f }, 0.991789f, -0.00019175f, 0 );

    pEObj = instance.addEObj( "Sealedbarrier", 2000060, 2065984, 1958528, 0, { -103.196800f, 16.825590f, 15.152160f }, 0.950000f, 2.09743111f, 0 );
    pEObj->addCollisionBox( { -102.862619f, 16.217449f, 14.987254f }, 2.094395f, 18.400000f, 7.360000f, 1.077015f, true );
    pEObj->addCollisionBox( { -100.162314f, 16.217448f, 3.867309f }, -2.618339f, 18.400000f, 7.360000f, 1.840000f, true );
    pEObj->addCollisionBox( { -91.882309f, 16.217449f, 18.208687f }, -2.618339f, 18.400000f, 7.360000f, 1.840000f, true );
    pEObj->setCollisionEnabled( true );

    pEObj = instance.addEObj( "Cultistrosary", 2000057, 0, 1958529, 0, { -179.952606f, 14.711600f, -4.996500f }, 0.750000f, 1.57070045f, 0 );

    pEObj = instance.addEObj( "Cultistorb_2", 2000063, 3817039, 1958530, 0, { -89.733833f, 15.701490f, 13.923830f }, 0.991789f, -0.92508629f, 0 );
    // States -> vf_beam3_off (id: 6) vf_beam3_on (id: 7) all_off (id: 17) 

    pEObj = instance.addEObj( "Cultistorb_3", 2000067, 3817052, 1958536, 0, { -94.766899f, 15.709600f, 3.820237f }, 1.000000f, -1.22737638f, 0 );
    // States -> vf_beam3_off (id: 6) vf_beam3_on (id: 7) all_off (id: 17) 

    pEObj = instance.addEObj( "unknown_0", 2000054, 0, 1968438, 0, { -53.345570f, 49.647820f, -146.799103f }, 0.991760f, 0.000048f, 0 );

    pEObj = instance.addEObj( "sgvf_w_lvd_b0118_5", 2001741, 3860684, 3689242, 1, { -64.365189f, 15.049010f, -4.720837f }, 1.000000f, -0.00009587f, 0 );
    // States -> vf_bextwall_on (id: 3) vf_bextwall_of (id: 4) 
    pEObj->addCollisionBox( { -68.565607f, 18.053922f, -3.831173f }, -1.095258f, 9.054634f, 8.000000f, 1.500000f, true );

    pEObj = instance.addEObj( "unknown_1", 2001085, 0, 3861129, 1, { -68.771881f, 15.144450f, -3.840616f }, 0.539465f, -1.09535816f, 0 );

    pEObj = instance.addEObj( "Shortcut", 2000700, 0, 4108030, 0, { -57.755428f, 47.653931f, -120.622597f }, 0.991760f, -0.00009587f, 0 );

    pEObj = instance.addEObj( "sgvf_w_lvd_b0118", 2001697, 4108043, 4108044, 1, { 13.157160f, 29.280100f, -7.370761f }, 1.000000f, -0.00009587f, 0 );
    // States -> vf_bextwall_on (id: 3) vf_bextwall_of (id: 4) 
    pEObj->addCollisionBox( { 11.090250f, 31.718651f, -7.243425f }, -0.000000f, 15.000000f, 8.000000f, 1.500000f, true );

    pEObj = instance.addEObj( "sgvf_w_lvd_b0118_1", 2001698, 4108046, 4108049, 1, { -18.524361f, 23.342850f, 42.375740f }, 1.000000f, -0.00009587f, 0 );
    // States -> vf_bextwall_on (id: 3) vf_bextwall_of (id: 4) 
    pEObj->addCollisionBox( { -19.331751f, 25.795650f, 41.278980f }, -0.948673f, 15.000000f, 8.000000f, 1.500000f, true );

    pEObj = instance.addEObj( "sgvf_w_lvd_b0118_2", 2001699, 4108055, 4108060, 1, { 19.811380f, 30.119770f, -20.717630f }, 1.000000f, -0.00009587f, 0 );
    // States -> vf_bextwall_on (id: 3) vf_bextwall_of (id: 4) 
    pEObj->addCollisionBox( { 19.739401f, 32.619801f, -19.838499f }, -1.409320f, 15.000000f, 8.000000f, 1.500000f, true );

    pEObj = instance.addEObj( "unknown_2", 2001700, 0, 4108066, 1, { 19.739401f, 30.119801f, -19.838511f }, 1.000000f, -1.40944072f, 0 );

    pEObj = instance.addEObj( "sgvf_w_lvd_b0118_3", 2001701, 4108071, 4108072, 1, { -3.087158f, 23.754129f, 40.895020f }, 1.000000f, -0.00009587f, 0 );
    // States -> vf_bextwall_on (id: 3) vf_bextwall_of (id: 4) 
    pEObj->addCollisionBox( { -3.984200f, 26.254101f, 41.874901f }, 0.675015f, 15.000000f, 8.000000f, 1.500000f, true );

    pEObj = instance.addEObj( "unknown_3", 2001702, 0, 4108073, 1, { -3.984208f, 23.754129f, 41.874859f }, 1.000000f, 0.67495155f, 0 );

    pEObj = instance.addEObj( "sgvf_w_lvd_b0118_4", 2001703, 4108078, 4108097, 1, { -95.779678f, 14.524550f, 45.017761f }, 1.000000f, -0.00009587f, 0 );
    // States -> vf_bextwall_on (id: 3) vf_bextwall_of (id: 4) 
    pEObj->addCollisionBox( { -97.035698f, 17.024600f, 44.317101f }, -0.656244f, 15.000000f, 8.000000f, 1.500000f, true );

    pEObj = instance.addEObj( "unknown_4", 2001704, 0, 4108099, 1, { -97.035683f, 14.524550f, 44.317101f }, 1.000000f, -0.65635203f, 0 );

    pEObj = instance.addEObj( "Entrance", 2000182, 4108136, 4108138, 1, { -57.606178f, 49.507038f, -142.170105f }, 1.000000f, -0.00009587f, 0 );
    // States -> vf_lock_on (id: 11) vf_lock_of (id: 12) 
    pEObj->addCollisionBox( { -55.724998f, 51.576199f, -145.110199f }, -0.000000f, 2.600000f, 8.000000f, 0.500000f, true );
    pEObj->addCollisionBox( { -55.715196f, 51.576199f, -139.110199f }, -0.000000f, 2.600000f, 8.000000f, 0.500000f, true );
    pEObj->addCollisionBox( { -52.724998f, 51.576199f, -142.106308f }, 1.570451f, 2.600000f, 8.000000f, 0.500000f, true );
    pEObj->addCollisionBox( { -58.724998f, 51.576199f, -142.099223f }, 1.570451f, 2.600000f, 8.000000f, 0.500000f, true );
    pEObj->addCollisionBox( { -57.824998f, 51.576199f, -144.210199f }, 0.785398f, 2.800000f, 8.000000f, 0.500000f, true );
    pEObj->addCollisionBox( { -53.624999f, 51.576199f, -140.010199f }, 0.785398f, 2.800000f, 8.000000f, 0.500000f, true );
    pEObj->addCollisionBox( { -53.624999f, 51.576199f, -144.210199f }, -0.785398f, 2.800000f, 8.000000f, 0.500000f, true );
    pEObj->addCollisionBox( { -57.824998f, 51.576199f, -140.010199f }, -0.785398f, 2.800000f, 8.000000f, 0.500000f, true );

    pEObj->setCollisionEnabled( true );

    instance.setDirectorVar( SharedGroups, 62 );
    setGate( instance, 2001697, true );
    setGate( instance, 2001698, true );
    for( auto id : { 2001699, 2001701, 2001703, 2001741 } )
      setGate( instance, id, false );
    for( const auto& altar : Altars )
      registerAltar( instance, altar );
    instance.getEObjByBaseId( 2000139 )->setPermissionInvisibility( 7 );
    instance.getEObjByBaseId( 2000700 )->setPermissionInvisibility( 7 );
  }

  void onStateChange( InstanceContent& instance, InstanceContentState oldState, InstanceContentState newState ) override
  {
    if( newState != DutyInProgress )
      return;

    instance.getEObjByBaseId( 2000182 )->setCollisionEnabled( false );
    for( auto i : { 0, 1 } )
    {
      auto encounter = instance.getEncounter( Altars[ i ].key );
      if( encounter->getStatus() == EncounterStatus::UNINITIALIZED )
        encounter->init();
    }
  }

  void onReset( InstanceContent& instance ) override
  {
    if( instance.getState() == DutyReset )
      instance.getEObjByBaseId( 2000182 )->setCollisionEnabled( true );

    for( const auto& altar : Altars )
    {
      auto encounter = instance.getEncounter( altar.key );
      const auto status = encounter->getStatus();
      if( status == EncounterStatus::ACTIVE || ( status == EncounterStatus::IDLE && getEngagedGuard( *encounter, altar ) ) )
      {
        encounter->setStatus( EncounterStatus::FAIL );
        instance.setCurrentBGM( 0 );
      }
    }
    instance.setCustomVar( CurrentAltar, 0 );

    auto encounter = instance.getEncounter( "Galvanth" );
    if( encounter && encounter->getStatus() == EncounterStatus::ACTIVE )
    {
      instance.getEObjByBaseId( 2001085 )->setPermissionInvisibility( 0 );
      encounter->setStatus( EncounterStatus::FAIL );
    }
  }

  void onUpdate( InstanceContent& instance, uint64_t tickCount ) override
  {
    if( instance.getState() == DutyFinished )
    {
      if( !instance.getCustomVar( GalvanthScene ) )
        instance.setCustomVar( GalvanthScene, BossEnding );
      else
        updateGalvanthScene( instance );
      return;
    }
    if( instance.getState() != DutyInProgress )
      return;

    if( instance.getCustomVar( WipePending ) )
    {
      instance.setCustomVar( WipePending, 0 );
      onReset( instance );
      return;
    }

    updateGalvanthScene( instance );

    updateAltars( instance, tickCount );

    auto encounter = instance.getEncounter( "Galvanth" );
    if( !encounter )
      return;

    auto boss = encounter->getBNpc( Galvanth );
    if( !boss || !boss->isAlive() )
      return;

    auto target = boss->hateListGetHighest();
    if( encounter->getStatus() == EncounterStatus::IDLE && target )
    {
      boss->setStatValue( Common::BaseParam::PhysicalDamage, 62 );
      boss->setStatValue( Common::BaseParam::MagicDamage, 62 );
      encounter->onEnterRange( target );
      encounter->start();
    }
    if( encounter->getStatus() != EncounterStatus::ACTIVE )
      return;

    if( !target )
    {
      instance.getEObjByBaseId( 2001085 )->setPermissionInvisibility( 0 );
      encounter->setStatus( EncounterStatus::FAIL );
      return;
    }

    encounter->update( tickCount );
  }

  void onActorDeath( InstanceContent& instance, Entity::Chara& chara ) override
  {
    if( instance.getState() != DutyInProgress )
      return;

    if( chara.isPlayer() )
    {
      const auto& players = instance.getPlayers();
      if( std::none_of( players.begin(), players.end(), []( const auto& entry ) { return entry.second->isAlive(); } ) )
        instance.setCustomVar( WipePending, 1 );
      return;
    }

    auto npc = chara.getAsBNpc();
    if( !npc )
      return;

    auto id = npc->getLayoutId();
    for( size_t i = 0; i < Altars.size(); ++i )
    {
      const auto& altar = Altars[ i ];
      if( id == altar.soulcounter )
      {
        instance.setCustomVar( SoulcountersDefeated, instance.getCustomVar( SoulcountersDefeated ) | ( 1u << i ) );
        instance.getEncounter( altar.key )->setStatus( EncounterStatus::SUCCESS );
        instance.setCustomVar( CurrentAltar, 0 );
        instance.setCurrentBGM( 0 );
        instance.getEObjByBaseId( altar.marker )->setPermissionInvisibility( 7 );
        announce( instance, 2006 );
        if( i == 0 )
          instance.getEObjByBaseId( 2000700 )->setPermissionInvisibility( 0 );
        return;
      }

      if( std::find( altar.guards.begin(), altar.guards.end(), id ) != altar.guards.end() )
      {
        if( instance.getEncounter( altar.key )->areRequiredParticipantsDefeated() )
          announce( instance, 2009, { 455, 1 } );
        return;
      }
    }

    auto rosaryGuard = std::find( RosaryGuards.begin(), RosaryGuards.end(), id );
    if( rosaryGuard != RosaryGuards.end() )
    {
      auto defeated = instance.getCustomVar( RosaryGuardsDefeated ) | ( 1u << std::distance( RosaryGuards.begin(), rosaryGuard ) );
      instance.setCustomVar( RosaryGuardsDefeated, defeated );
      if( defeated == 15 )
        announce( instance, 2008 );
    }
    else if( id == Galvanth && instance.getEncounter( "Galvanth" )->areRequiredParticipantsDefeated() )
    {
      instance.getEncounter( "Galvanth" )->setStatus( EncounterStatus::SUCCESS );
      instance.getEObjByBaseId( 2001085 )->setPermissionInvisibility( 7 );
      instance.getEObjByBaseId( 2000139 )->setPermissionInvisibility( 0 );
      instance.finishDuty( false );
      instance.setCustomVar( GalvanthScene, 0 );
    }
  }

  bool canUseObjective( InstanceContent& instance, Entity::Player& player, Entity::EventObject& eobj )
  {
    if( instance.getState() != DutyInProgress || !player.isAlive() ||
        player.getTerritoryId() != instance.getGuId() || eobj.getPermissionInvisibility() == 7 ||
        Common::Util::distance( player.getPos(), eobj.getPos() ) > 5.f )
      return false;

    auto id = eobj.getBaseId();
    if( id == 2000057 )
      return instance.getCustomVar( RosaryGuardsDefeated ) == 15 && !instance.getDirectorVar( Rosary );
    if( id == 2000060 )
      return instance.getDirectorVar( Rosary ) && !instance.getDirectorVar( BrokenBarrier );

    auto orb = std::find( OrbIds.begin(), OrbIds.end(), id );
    if( orb == OrbIds.end() )
      return false;

    auto index = std::distance( OrbIds.begin(), orb );
    auto altar = std::min< size_t >( index, 2 );
    return ( instance.getDirectorVar( SharedGroups ) & ( 1u << ( index + 1 ) ) ) &&
           ( instance.getCustomVar( SoulcountersDefeated ) & ( 1u << altar ) );
  }

  void finishObjective( InstanceContent& instance, Entity::Player& player, Entity::EventObject& eobj )
  {
    if( !canUseObjective( instance, player, eobj ) )
      return;

    auto id = eobj.getBaseId();
    auto groups = instance.getDirectorVar( SharedGroups );
    auto orbs = instance.getDirectorVar( DestroyedOrbs );
    if( id == 2000057 )
    {
      instance.setDirectorVar( Rosary, 1 );
      instance.setDirectorVar( VisibleObjectives, 3 );
      announce( instance, 2031, { 2000244 } );
    }
    else if( id == 2000060 )
    {
      instance.setDirectorVar( BrokenBarrier, 1 );
      eobj.setCollisionEnabled( false );
      instance.getEncounter( Altars[ 2 ].key )->init();
      announce( instance, 1536, { 0 } );
    }
    else
    {
      auto orb = std::find( OrbIds.begin(), OrbIds.end(), id );
      auto index = std::distance( OrbIds.begin(), orb );
      auto bit = 1u << ( index + 1 );

      if( index == 0 )
        setGate( instance, 2001697, false );
      else if( index == 1 )
        setGate( instance, 2001698, false );

      ++orbs;
      instance.setDirectorVar( DestroyedOrbs, orbs );
      instance.setDirectorVar( SharedGroups, orbs == 4 ? 1 : groups & ~bit );
      announce( instance, 1537 );
      if( orbs == 2 )
      {
        instance.setDirectorVar( VisibleObjectives, 1 );
        for( auto guard : RosaryGuards )
        {
          if( auto npc = instance.createBNpcFromLayoutId( guard, 1155, Common::BNpcType::Enemy ) )
            instance.pushActor( npc );
        }
      }
      else if( orbs == 4 )
      {
        instance.setDirectorVar( VisibleObjectives, 7 );
        announce( instance, 1539 );
        instance.setCustomVar( GalvanthScene, BossIntro );
      }
    }
    eobj.setPermissionInvisibility( 7 );
    publishObjectives( instance );
  }

  void onTalk( InstanceContent& instance, Entity::Player& player, Entity::EventObject& eobj, uint32_t eventId ) override
  {
    std::weak_ptr< InstanceContent > duty = instance.getAsInstanceContent();
    if( eobj.getBaseId() == 2000700 && !eobj.getPermissionInvisibility() )
    {
      eventMgr().playScene( player, eventId, 1, HIDE_HOTBAR, { 1 },
        [ this, duty ]( Entity::Player& player, const Event::SceneResult& result )
        {
          auto instance = duty.lock();
          if( !instance || player.getTerritoryId() != instance->getGuId() || result.getResult( 0 ) != 0 )
            return;

          auto cleared = instance->getCustomVar( SoulcountersDefeated );
          size_t altar = cleared & 4 ? 2 : ( cleared & 2 ? 1 : 0 );
          auto destination = instance->getEObjByBaseId( OrbIds[ altar ] );
          warpMgr().requestWarp( player, Common::WarpType::WARP_TYPE_INSTANCE_CONTENT,
                                 destination->getPos(), destination->getRot() );
        } );
      return;
    }

    if( !canUseObjective( instance, player, eobj ) )
      return;

    eventMgr().eventActionStart( player, eventId, 24,
      [ this, duty ]( Entity::Player& player, uint32_t, uint64_t actorId )
      {
        auto instance = duty.lock();
        if( instance )
        {
          auto object = instance->getEObj( static_cast< uint32_t >( actorId ) );
          if( object )
            finishObjective( *instance, player, *object );
        }
      }, nullptr, eobj.getId() );
  }
};

EXPOSE_SCRIPT( TheTamTaraDeepcroft );
