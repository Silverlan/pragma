// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_CAMERA_H__
#define __C_ENV_CAMERA_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_camera.h"

namespace pragma {
	class DLLCLIENT CCameraComponent final : public BaseEnvCameraComponent {
	  public:
		CCameraComponent(BaseEntity &ent) : BaseEnvCameraComponent(ent) {}
		virtual ~CCameraComponent() override;
		virtual void Initialize() override;
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnEntitySpawn() override;
	  protected:
		void UpdateState();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	};
};

class DLLCLIENT CEnvCamera : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
