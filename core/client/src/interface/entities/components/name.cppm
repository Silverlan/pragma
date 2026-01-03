// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.name;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CNameComponent final : public BaseNameComponent, public CBaseNetComponent {
	  public:
		CNameComponent(ecs::BaseEntity &ent) : BaseNameComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};
};
