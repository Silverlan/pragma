// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.server:entities.components.vehicle;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SVehicleComponent final : public BaseVehicleComponent, public SBaseSnapshotComponent {
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		static unsigned int GetVehicleCount();
		static const std::vector<SVehicleComponent *> &GetAll();
		SVehicleComponent(pragma::ecs::BaseEntity &ent);
		virtual ~SVehicleComponent() override;
		virtual void ClearDriver() override;
		virtual void SetDriver(pragma::ecs::BaseEntity *ent) override;
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void Initialize() override;
		virtual void OnTick(double tDelta) override;

		virtual void SetupSteeringWheel(const std::string &mdl, umath::Degree maxSteeringAngle) override;

		virtual bool ShouldTransmitNetData() const override { return true; };
		virtual void SendSnapshotData(NetPacket &packet, pragma::BasePlayerComponent &pl) override;
		virtual bool ShouldTransmitSnapshotData() const override;
	  protected:
		void WriteWheelInfo(NetPacket &p, WheelData &data);
		CallbackHandle m_playerAction;
		void OnActionInput(pragma::Action action, bool b);
		virtual void OnRemove() override;
		void OnPostSpawn();
		void OnUse(pragma::ecs::BaseEntity *pl);
		virtual BaseWheelComponent *CreateWheelEntity(uint8_t wheelIndex) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	  private:
		static std::vector<SVehicleComponent *> s_vehicles;
	};
};
