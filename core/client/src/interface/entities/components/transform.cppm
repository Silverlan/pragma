// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.transform;

export import :entities.components.entity;

export namespace pragma {
	namespace cTransformComponent {
		using namespace baseTransformComponent;
	}
	class DLLCLIENT CTransformComponent final : public BaseTransformComponent, public CBaseNetComponent {
	  public:
		CTransformComponent(ecs::BaseEntity &ent) : BaseTransformComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
