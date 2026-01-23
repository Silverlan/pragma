// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.model;

import :entities.components.entity;

export namespace pragma {
	namespace sModelComponent {
		using namespace baseModelComponent;
	}
	class DLLSERVER SModelComponent final : public BaseModelComponent, public SBaseNetComponent {
	  public:
		SModelComponent(ecs::BaseEntity &ent) : BaseModelComponent(ent) {}
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;

		using BaseModelComponent::SetModel;
		virtual bool SetBodyGroup(UInt32 groupId, UInt32 id) override;
		virtual void SetSkin(unsigned int skin) override;
		virtual void SetMaxDrawDistance(float maxDist) override;
	  protected:
		virtual void OnModelChanged(const std::shared_ptr<asset::Model> &model) override;
	};
};
