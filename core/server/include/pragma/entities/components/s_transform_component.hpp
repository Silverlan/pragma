// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_TRANSFORM_COMPONENT_HPP__
#define __S_TRANSFORM_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_transform_component.hpp>

namespace pragma {
	class DLLSERVER STransformComponent final : public BaseTransformComponent, public SBaseNetComponent {
	  public:
		STransformComponent(BaseEntity &ent) : BaseTransformComponent(ent) {}
		virtual void SetEyeOffset(const Vector3 &offset) override;
		virtual void SetScale(const Vector3 &scale) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};

#endif
