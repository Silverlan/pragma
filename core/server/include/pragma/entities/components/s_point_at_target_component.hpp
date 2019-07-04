#ifndef __S_POINT_AT_TARGET_COMPONENT_HPP__
#define __S_POINT_AT_TARGET_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_point_at_target_component.hpp>
#include <pragma/lua/luaapi.h>

namespace pragma
{
	class DLLSERVER SPointAtTargetComponent final
		: public BasePointAtTargetComponent,
		public SBaseNetComponent
	{
	public:
		SPointAtTargetComponent(BaseEntity &ent) : BasePointAtTargetComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;

		using BasePointAtTargetComponent::SetPointAtTarget;
	protected:
		virtual void SetPointAtTarget(BaseEntity *ent) override;
	};
};

#endif
