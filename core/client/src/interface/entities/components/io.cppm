// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.io;

export import pragma.shared;

export namespace pragma {
	namespace cIOComponent {
		using namespace baseIOComponent;
	}
	class DLLCLIENT CIOComponent final : public BaseIOComponent {
	  public:
		CIOComponent(ecs::BaseEntity &ent) : BaseIOComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
