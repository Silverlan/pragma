// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.func.brush;

import :entities;

import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SBrushComponent final : public BaseFuncBrushComponent, public SBaseNetComponent {
		  public:
			SBrushComponent(ecs::BaseEntity &ent) : BaseFuncBrushComponent(ent) {}
			virtual void Initialize() override;
			virtual void OnEntitySpawn() override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER FuncBrush : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
