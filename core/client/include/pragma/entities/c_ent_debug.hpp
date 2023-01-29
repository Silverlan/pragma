/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENT_DEBUG_HPP__
#define __C_ENT_DEBUG_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/components/c_entity_component.hpp"

class DLLCLIENT CEntDebugText : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

class DLLCLIENT CEntDebugPoint : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

class DLLCLIENT CEntDebugLine : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

class DLLCLIENT CEntDebugBox : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

class DLLCLIENT CEntDebugSphere : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

class DLLCLIENT CEntDebugCone : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

class DLLCLIENT CEntDebugCylinder : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

class DLLCLIENT CEntDebugPlane : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
