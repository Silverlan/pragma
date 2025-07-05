// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __S_TRIGGER_HURT_H__
#define __S_TRIGGER_HURT_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/trigger/base_trigger_hurt.hpp"

namespace pragma {
	class DLLSERVER STriggerHurtComponent final : public BaseTriggerHurtComponent {
	  public:
		STriggerHurtComponent(BaseEntity &ent) : BaseTriggerHurtComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER TriggerHurt : public SBaseEntity {
  protected:
  public:
	virtual void Initialize() override;
};

#endif
