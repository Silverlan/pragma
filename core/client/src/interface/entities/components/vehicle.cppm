// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.vehicle;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CVehicleComponent final : public BaseVehicleComponent, public CBaseSnapshotComponent {
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		CVehicleComponent(ecs::BaseEntity &ent);
		virtual ~CVehicleComponent() override;
		static unsigned int GetVehicleCount();
		static const std::vector<CVehicleComponent *> &GetAll();
		virtual void ClearDriver() override;
		virtual void SetDriver(ecs::BaseEntity *ent) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void ReceiveSnapshotData(NetPacket &packet) override;
	  protected:
		void ReadWheelInfo(NetPacket &packet);
		CallbackHandle m_hCbSteeringWheel;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	  private:
		static std::vector<CVehicleComponent *> s_vehicles;
	};
};
