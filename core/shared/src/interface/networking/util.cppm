// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:networking.util;

export import :entities.base_entity_handle;

export import pragma.util;

export {
	namespace pragma::ecs {
		class BaseEntity;
	}
	namespace pragma {
		class BasePlayerComponent;
	};
	namespace pragma::networking {
		DLLNETWORK void write_vector(NetPacket &packet, const Vector3 &v);
		DLLNETWORK void write_angles(NetPacket &packet, const EulerAngles &ang);
		DLLNETWORK void write_quat(NetPacket &packet, const Quat &rot);
		DLLNETWORK void write_entity(NetPacket &packet, const ecs::BaseEntity *ent);
		DLLNETWORK void write_entity(NetPacket &packet, const EntityHandle &hEnt);

		DLLNETWORK Vector3 read_vector(NetPacket &packet);
		DLLNETWORK EulerAngles read_angles(NetPacket &packet);
		DLLNETWORK Quat read_quat(NetPacket &packet);
		DLLNETWORK ecs::BaseEntity *read_entity(NetPacket &packet);
		DLLNETWORK CallbackHandle read_entity(NetPacket &packet, const std::function<void(ecs::BaseEntity *)> &onCreated);
		template<class T>
		T *read_entity(NetPacket &packet);
		//template<class T>
		//T *read_entity(NetPacket &packet,const std::function<void(BaseEntity*)> &onCreated);

		DLLNETWORK void write_player(NetPacket &packet, const ecs::BaseEntity *pl);
		DLLNETWORK void write_player(NetPacket &packet, const BasePlayerComponent *pl);
		DLLNETWORK BasePlayerComponent *read_player(NetPacket &packet);
	};

	/*template<class T>
		T *pragma::networking::read_entity(NetPacket &packet,const std::function<void(BaseEntity*)> &onCreated)
	{
		auto *ent = pragma::networking::read_entity(packet,onCreated);
		return dynamic_cast<T*>(ent);
	}*/

	template<class T>
	T *pragma::networking::read_entity(NetPacket &packet)
	{
		ecs::BaseEntity *ent = read_entity(packet);
		return dynamic_cast<T *>(ent);
	}
};
