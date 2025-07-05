// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __C_ENV_MICROPHONE_H__
#define __C_ENV_MICROPHONE_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_microphone_base.h"

namespace pragma {
	class DLLCLIENT CMicrophoneComponent final : public BaseEnvMicrophoneComponent {
	  public:
		CMicrophoneComponent(BaseEntity &ent) : BaseEnvMicrophoneComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLCLIENT CEnvMicrophone : public CBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
