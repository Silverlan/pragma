/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __NWM_UTIL_H__
#define __NWM_UTIL_H__

#include "pragma/networkdefinitions.h"
#include <mathutil/glmutil.h>
#include <sharedutils/netpacket.hpp>
#include <sharedutils/functioncallback.h>

class EulerAngles;
class BaseEntity;
class EntityHandle;
class BasePlayer;
namespace pragma {class BasePlayerComponent;};
namespace nwm
{
	DLLNETWORK void write_vector(NetPacket &packet,const Vector3 &v);
	DLLNETWORK void write_angles(NetPacket &packet,const EulerAngles &ang);
	DLLNETWORK void write_quat(NetPacket &packet,const Quat &rot);
	DLLNETWORK void write_entity(NetPacket &packet,const BaseEntity *ent);
	DLLNETWORK void write_entity(NetPacket &packet,const EntityHandle &hEnt);

	DLLNETWORK Vector3 read_vector(NetPacket &packet);
	DLLNETWORK EulerAngles read_angles(NetPacket &packet);
	DLLNETWORK Quat read_quat(NetPacket &packet);
	DLLNETWORK BaseEntity *read_entity(NetPacket &packet);
	DLLNETWORK CallbackHandle read_entity(NetPacket &packet,const std::function<void(BaseEntity*)> &onCreated);
	template<class T>
		T *read_entity(NetPacket &packet);
	template<class T>
		T *read_entity(NetPacket &packet,const std::function<void(BaseEntity*)> &onCreated);

	DLLNETWORK void write_player(NetPacket &packet,const BaseEntity *pl);
	DLLNETWORK void write_player(NetPacket &packet,const pragma::BasePlayerComponent *pl);
	DLLNETWORK pragma::BasePlayerComponent *read_player(NetPacket &packet);
};

template<class T>
	T *nwm::read_entity(NetPacket &packet,const std::function<void(BaseEntity*)> &onCreated)
{
	auto *ent = nwm::read_entity(packet,onCreated);
	return dynamic_cast<T*>(ent);
}

template<class T>
	T *nwm::read_entity(NetPacket &packet)
{
	BaseEntity *ent = nwm::read_entity(packet);
	return dynamic_cast<T*>(ent);
}

#endif