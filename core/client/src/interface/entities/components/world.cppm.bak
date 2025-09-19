// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/model/render_mesh_group.hpp"
#include "buffers/prosper_buffer.hpp"
#include "pragma/rendering/render_queue.hpp"
#include <pragma/entities/baseworld.h>
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include <pragma/util/util_bsp_tree.hpp>
#include <unordered_map>

export module pragma.client.entities.components.world;

import pragma.client.rendering.occlusion_culling;

#pragma warning(push)
#pragma warning(disable : 4251)
export namespace pragma {
	class DLLCLIENT CWorldComponent final : public BaseWorldComponent {
	  public:
		CWorldComponent(BaseEntity &ent) : BaseWorldComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		void ReloadMeshCache();
		std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> GetMeshTree() const;
		std::shared_ptr<::CHC> GetCHCController() const;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void OnTick(double tDelta) override;

		const rendering::RenderQueue *GetClusterRenderQueue(util::BSPTree::ClusterIndex clusterIndex, bool translucent = false) const;
		void SetBSPTree(const std::shared_ptr<util::BSPTree> &bspTree, const std::vector<std::vector<RenderMeshIndex>> &meshesPerCluster);
		const std::shared_ptr<util::BSPTree> &GetBSPTree() const;
		void RebuildRenderQueues();
	  protected:
		void SetRenderQueuesDirty();
		void BuildOfflineRenderQueues(bool rebuild = false);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void UpdateRenderMeshes();
		void ReloadCHCController();
		std::vector<std::shared_ptr<rendering::RenderQueue>> m_clusterRenderQueues;
		std::vector<std::shared_ptr<rendering::RenderQueue>> m_clusterRenderTranslucentQueues;
		std::unordered_map<uint32_t, bool> m_lodBaseMeshIds;
		std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> m_meshTree;
		std::shared_ptr<::CHC> m_chcController;

		std::shared_ptr<util::BSPTree> m_bspTree = nullptr;
		std::vector<std::vector<RenderMeshIndex>> m_meshesPerCluster;

		bool m_renderQueuesDirty = false;
	};
};
#pragma warning(pop)

export class DLLCLIENT CWorld : public CBaseEntity {
  public:
	virtual void Initialize() override;
	virtual Con::c_cout &print(Con::c_cout &) override;
	virtual std::ostream &print(std::ostream &) override;
};
