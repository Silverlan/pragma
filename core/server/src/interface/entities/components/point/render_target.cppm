// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/point/point_rendertarget.h"
#include "pragma/entities/components/s_entity_component.hpp"

export module pragma.server.entities.components.point.render_target;

export {
	namespace pragma {
		class DLLSERVER SRenderTargetComponent final : public BasePointRenderTargetComponent, public SBaseNetComponent {
		public:
			SRenderTargetComponent(BaseEntity &ent) : BasePointRenderTargetComponent(ent) {}
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER PointRenderTarget : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
