#ifndef __C_BOT_H__
#define __C_BOT_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_ai_component.hpp"
#include "pragma/entities/basebot.h"
#include <pragma/entities/components/base_character_component.hpp>

namespace pragma
{
	class DLLCLIENT CBotComponent final
		: public BaseBotComponent
	{
	public:
		CBotComponent(BaseEntity &ent) : BaseBotComponent(ent) {}
		virtual void Initialize() override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId,ComponentEvent &evData) override;
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		void OnFootStep(BaseCharacterComponent::FootType foot);
	};
};

class DLLCLIENT CBot
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif