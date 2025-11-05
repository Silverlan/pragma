// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;
#include "pragma/clientdefinitions.h"

export module pragma.client:entities.debug;

export import :entities.base_entity;

export class DLLCLIENT CEntDebugText : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugPoint : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugLine : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugBox : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugSphere : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugCone : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugCylinder : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugPlane : public CBaseEntity {
  public:
	virtual void Initialize() override;
};
