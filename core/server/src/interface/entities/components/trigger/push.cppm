// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.server.entities.components.triggers.push;

import pragma.server.entities;

export {
	namespace pragma {
		class DLLSERVER STriggerPushComponent final : public BaseTriggerPushComponent {
		public:
			STriggerPushComponent(BaseEntity &ent) : BaseTriggerPushComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua_State *l) override;
		};
	};

	class DLLSERVER TriggerPush : public SBaseEntity {
	public:
		virtual void Initialize() override;
	};
};
