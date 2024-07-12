/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

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
