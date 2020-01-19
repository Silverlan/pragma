#ifndef __S_SKYBOX_H__
#define __S_SKYBOX_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/baseskybox.h>

namespace pragma
{
	class DLLSERVER SSkyboxComponent final
		: public BaseSkyboxComponent,
		public SBaseNetComponent
	{
	public:
		SSkyboxComponent(BaseEntity &ent) : BaseSkyboxComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
		virtual void SetSkyAngles(const EulerAngles &ang) override;

		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

class DLLSERVER Skybox
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif