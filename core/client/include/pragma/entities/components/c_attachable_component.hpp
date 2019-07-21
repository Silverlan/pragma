#ifndef __C_ATTACHABLE_COMPONENT_HPP__
#define __C_ATTACHABLE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_attachable_component.hpp>

namespace pragma
{
	class DLLCLIENT CAttachableComponent final
		: public BaseAttachableComponent,
		public CBaseNetComponent
	{
	public:
		CAttachableComponent(BaseEntity &ent) : BaseAttachableComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}

		virtual void ReceiveData(NetPacket &packet) override;
		virtual void UpdateViewAttachmentOffset(BaseEntity *ent,pragma::BaseCharacterComponent &pl,Vector3 &pos,Quat &rot,Bool bYawOnly=false) override;
	protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};

#endif
