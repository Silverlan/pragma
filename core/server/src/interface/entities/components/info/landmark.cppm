// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.info.landmark;

import :entities;
import :entities.components.entity;

export {
	namespace pragma {
		class DLLSERVER SInfoLandmarkComponent final : public BaseInfoLandmarkComponent {
		  public:
			SInfoLandmarkComponent(ecs::BaseEntity &ent) : BaseInfoLandmarkComponent(ent) {}
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER InfoLandmark : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
