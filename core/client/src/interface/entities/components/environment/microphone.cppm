// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.env_microphone;

export import :entities.base_entity;

export {
	namespace pragma {
		class DLLCLIENT CMicrophoneComponent final : public BaseEnvMicrophoneComponent {
		  public:
			CMicrophoneComponent(ecs::BaseEntity &ent) : BaseEnvMicrophoneComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLCLIENT CEnvMicrophone : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
