#ifndef __S_POINT_RENDERTARGET_H__
#define __S_POINT_RENDERTARGET_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/point_rendertarget.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SRenderTargetComponent final
		: public BasePointRenderTargetComponent,
		public SBaseNetComponent
	{
	public:
		SRenderTargetComponent(BaseEntity &ent) : BasePointRenderTargetComponent(ent) {}
		virtual void SendData(NetPacket &packet,nwm::RecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER PointRenderTarget
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif