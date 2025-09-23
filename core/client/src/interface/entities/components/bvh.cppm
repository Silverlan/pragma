// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include <pragma/entities/components/base_bvh_component.hpp>
#include <unordered_set>

export module pragma.client:entities.components.bvh;
export namespace pragma {
	namespace rendering {
		struct RenderBufferData;
	};
	struct HitInfo;
	class DLLCLIENT CBvhComponent final : public BaseBvhComponent {
	  public:
		static bool ShouldConsiderMesh(const ModelSubMesh &mesh, const rendering::RenderBufferData &bufferData);
		CBvhComponent(BaseEntity &ent) : BaseBvhComponent(ent) {}
		virtual void Initialize() override;
		virtual void InitializeLuaObject(lua_State *l) override;
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
