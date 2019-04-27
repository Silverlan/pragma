#ifndef __C_ENTITY_COMPONENT_HPP__
#define __C_ENTITY_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/entities/entity_component_manager.hpp>

class NetPacket;
namespace nwm {class RecipientFilter;};
namespace pragma
{
	class DLLCLIENT CBaseNetComponent
		: public BaseNetComponent
	{
	public:
		virtual void ReceiveData(NetPacket &packet)=0;
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) {return false;}

		pragma::NetEventId FindNetEvent(const std::string &evName) const;
	};

	class DLLCLIENT CBaseSnapshotComponent
		: public CBaseNetComponent
	{
	public:
		virtual void ReceiveSnapshotData(NetPacket &packet)=0;
		virtual bool ShouldTransmitSnapshotData() const=0;
	};

	/////////////////////////////

	class DLLCLIENT CEntityComponentManager
		: public EntityComponentManager
	{
	public:
		static const ComponentId INVALID_COMPONENT;

		const std::vector<ComponentId> &GetServerComponentIdToClientComponentIdTable() const;
		std::vector<ComponentId> &GetServerComponentIdToClientComponentIdTable();
	private:
		std::vector<ComponentId> m_svComponentToClComponentTable;
	};
};

#endif
