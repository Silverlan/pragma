// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.debug;

export import :entities.base_entity;

export class DLLCLIENT CEntDebugText : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugPoint : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugLine : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugBox : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugSphere : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugCone : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugCylinder : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};

////////////////

export class DLLCLIENT CEntDebugPlane : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
