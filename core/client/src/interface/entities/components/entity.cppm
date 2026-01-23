// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.entity;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CBaseNetComponent : public BaseNetComponent {
	  public:
		virtual void ReceiveData(NetPacket &packet) = 0;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) { return false; }

		NetEventId FindNetEvent(const std::string &evName) const;
	};

	class DLLCLIENT CBaseSnapshotComponent : public CBaseNetComponent {
	  public:
		virtual void ReceiveSnapshotData(NetPacket &packet) = 0;
	};

	/////////////////////////////

	class DLLCLIENT CEntityComponentManager : public EntityComponentManager {
	  public:
		static const ComponentId INVALID_COMPONENT;

		const std::vector<ComponentId> &GetServerComponentIdToClientComponentIdTable() const;
		std::vector<ComponentId> &GetServerComponentIdToClientComponentIdTable();
	  private:
		std::vector<ComponentId> m_svComponentToClComponentTable;
	};
};
