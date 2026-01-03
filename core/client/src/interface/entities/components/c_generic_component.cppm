// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.generic;

export import pragma.shared;

export namespace pragma {
	class DLLCLIENT CGenericComponent final : public BaseGenericComponent {
	  public:
		CGenericComponent(ecs::BaseEntity &ent) : BaseGenericComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
