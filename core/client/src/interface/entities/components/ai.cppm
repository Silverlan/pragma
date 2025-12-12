// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "definitions.hpp"

export module pragma.client:entities.components.ai;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CAIComponent final : public BaseAIComponent, public CBaseSnapshotComponent {
	  private:
		static std::vector<CAIComponent *> s_npcs;
	  protected:
		Vector3 OnCalcMovementDirection() const;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		static unsigned int GetNPCCount();
		static const std::vector<CAIComponent *> &GetAll();
		CAIComponent(pragma::ecs::BaseEntity &ent);
		virtual ~CAIComponent() override;
		virtual void Initialize() override;
		virtual void UpdateMovementProperties(MovementComponent &movementC) override;
		virtual pragma::util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual bool ShouldTransmitNetData() const override { return false; }
		virtual void ReceiveSnapshotData(NetPacket &packet) override;
	};
};
