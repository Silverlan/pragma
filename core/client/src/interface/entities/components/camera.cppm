// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.camera;

export import :entities.base_entity;

export namespace pragma {
	namespace cCameraComponent {
		using namespace baseEnvCameraComponent;
	}
	class DLLCLIENT CCameraComponent final : public BaseEnvCameraComponent {
	  public:
		CCameraComponent(ecs::BaseEntity &ent) : BaseEnvCameraComponent(ent) {}
		virtual ~CCameraComponent() override;
		virtual void Initialize() override;
		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
	  protected:
		void UpdateState();
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
	};
};

export class DLLCLIENT CEnvCamera : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
