// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.bsp;

export import pragma.shared;
import source_engine.bsp;

export {
	namespace pragma {
		class DLLCLIENT CBSPComponent final : public BaseEntityComponent {
		  public:
			CBSPComponent(ecs::BaseEntity &ent) : BaseEntityComponent(ent) {}
			virtual void Initialize() override;
			virtual void InitializeLuaObject(lua::State *l) override;
		  protected:
			void InitializeBSPTree(source_engine::bsp::File &bsp);
		};
	};
};
