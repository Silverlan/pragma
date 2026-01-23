// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.transform;

import :entities.components.entity;

export namespace pragma {
	namespace sTransformComponent {
		using namespace baseTransformComponent;
	}
	class DLLSERVER STransformComponent final : public BaseTransformComponent, public SBaseNetComponent {
	  public:
		STransformComponent(ecs::BaseEntity &ent) : BaseTransformComponent(ent) {}
		virtual void SetEyeOffset(const Vector3 &offset) override;
		virtual void SetScale(const Vector3 &scale) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
