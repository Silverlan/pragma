// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.soft_body;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CSoftBodyComponent final : public BaseSoftBodyComponent {
	  public:
		CSoftBodyComponent(ecs::BaseEntity &ent) : BaseSoftBodyComponent(ent) {}

		virtual void Initialize() override;
		void UpdateSoftBodyGeometry();
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual bool InitializeSoftBodyData() override;
		virtual void ReleaseSoftBodyData() override;
	};
};
