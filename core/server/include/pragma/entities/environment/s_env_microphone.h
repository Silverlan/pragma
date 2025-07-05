// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_ENV_MICROPHONE_H__
#define __S_ENV_MICROPHONE_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/env_microphone_base.h"

namespace pragma {
	class DLLSERVER SMicrophoneComponent final : public BaseEnvMicrophoneComponent {
	  public:
		SMicrophoneComponent(BaseEntity &ent) : BaseEnvMicrophoneComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvMicrophone : public SBaseEntity {
  public:
	virtual void Initialize() override;
};

#endif
