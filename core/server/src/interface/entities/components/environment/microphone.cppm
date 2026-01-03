// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.microphone;

import :entities;

export {
	namespace pragma {
		class DLLSERVER SMicrophoneComponent final : public BaseEnvMicrophoneComponent {
		  public:
			SMicrophoneComponent(ecs::BaseEntity &ent) : BaseEnvMicrophoneComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvMicrophone : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
