#ifndef __S_ATTACHABLE_COMPONENT_HPP__
#define __S_ATTACHABLE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_attachable_component.hpp>

namespace pragma
{
	class DLLSERVER SAttachableComponent final
		: public BaseAttachableComponent,
		public SBaseNetComponent
	{
	public:
		SAttachableComponent(BaseEntity &ent) : BaseAttachableComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual void SetAttachmentFlags(FAttachmentMode flags) override;
		virtual bool ShouldTransmitNetData() const override {return true;}

		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		virtual AttachmentData *SetupAttachment(BaseEntity *ent,const AttachmentInfo &attInfo) override;
	};
};

#endif
