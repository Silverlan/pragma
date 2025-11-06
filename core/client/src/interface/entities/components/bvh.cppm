// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/lua/core.hpp"

export module pragma.client:entities.components.bvh;

export import pragma.shared;

export namespace pragma {
	namespace rendering {
		struct RenderBufferData;
	};

	class DLLCLIENT CBvhComponent final : public BaseBvhComponent {
	  public:
		static bool ShouldConsiderMesh(const pragma::ModelSubMesh &mesh, const rendering::RenderBufferData &bufferData);
		CBvhComponent(pragma::ecs::BaseEntity &ent) : BaseBvhComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		virtual bool IntersectionTest(const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, HitInfo &outHitInfo) const override;
		using BaseBvhComponent::IntersectionTest;
	  private:
		void UpdateBvhStatus();
		virtual void DoRebuildBvh() override;
	};
};
