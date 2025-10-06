// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/serverdefinitions.h"
#include "pragma/lua/luaapi.h"


export module pragma.server.entities.components.vehicle;

import pragma.server.entities.components.entity;

export namespace pragma {
	class DLLSERVER SVehicleComponent final : public BaseVehicleComponent, public SBaseSnapshotComponent {
	  public:
		static unsigned int GetVehicleCount();
		static const std::vector<SVehicleComponent *> &GetAll();
		SVehicleComponent(BaseEntity &ent);
		virtual ~SVehicleComponent() override;
		virtual void ClearDriver() override;
		virtual void SetDriver(BaseEntity *ent) override;
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
		void OnActionInput(Action action, bool b);
		virtual void OnRemove() override;
		void OnPostSpawn();
		void OnUse(BaseEntity *pl);
		virtual BaseWheelComponent *CreateWheelEntity(uint8_t wheelIndex) override;
		virtual void InitializeLuaObject(lua_State *l) override;
	  private:
		static std::vector<SVehicleComponent *> s_vehicles;
	};
};
