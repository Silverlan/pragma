/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __BASEVEHICLE_RAYCASTER_H__
#define __BASEVEHICLE_RAYCASTER_H__

#include "pragma/networkdefinitions.h"
#include <BulletDynamics/Vehicle/btRaycastVehicle.h>

#define USE_CONVEX_WHEELS 0

class Game;
class BaseEntity;
namespace pragma
{
	class BaseVehicleComponent;
};
struct PhysVehicleRaycaster
	: public btVehicleRaycaster
{
private:
	Game *m_game;
	pragma::BaseVehicleComponent &m_vehicle;
	BaseEntity &m_entity;
#if USE_CONVEX_WHEELS == 1
	std::shared_ptr<PhysConvexShape> m_shape;
#endif
public:
	PhysVehicleRaycaster(Game *game,pragma::BaseVehicleComponent &vhc);
	virtual ~PhysVehicleRaycaster() override;
	virtual void *castRay(const btVector3 &from,const btVector3 &to, btVehicleRaycasterResult &result);
};

#endif
