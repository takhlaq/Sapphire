#include <Action/Action.h>
#include <Actor/Chara.h>
#include <ScriptObject.h>

using namespace Sapphire;
using namespace Sapphire::World::Action;

class ActionDrainTouch : public ScriptAPI::ActionScript
{
public:
  ActionDrainTouch() : ScriptAPI::ActionScript( 988 )
  {
  }

  void onExecute( Action& action ) override
  {
    auto source = action.getSourceChara();
    auto target = action.getHitChara();
    if( !target )
      return;

    const auto& entry = ActionLut::getEntry( action.getId() );
    auto damage = action.calcDamage( entry.potency );
    auto results = action.getActionResultBuilder();
    results->damage( source, target, damage.first, damage.second );
    results->heal( target, source, damage.first / 4, Common::CalcResultType::TypeRecoverHp,
                   Common::ActionResultFlag::EffectOnSource );
  }
};

EXPOSE_SCRIPT( ActionDrainTouch );
