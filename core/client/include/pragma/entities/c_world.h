/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_WORLD_H__
#define __C_WORLD_H__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include <pragma/entities/baseworld.h>
#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include <pragma/util/util_bsp_tree.hpp>
#include <unordered_map>

#pragma warning(push)
#pragma warning(disable : 4251)
namespace prosper {
	class Buffer;
};
namespace util {
	class BSPTree;
};
class CHC;
namespace pragma {
	using RenderMeshIndex = uint32_t;
	using RenderMeshGroup = std::pair<RenderMeshIndex, RenderMeshIndex>; // Start index +count
	namespace rendering {
		class RenderQueue;
	};
	class DLLCLIENT CWorldComponent final : public BaseWorldComponent {
	  public:
		CWorldComponent(BaseEntity &ent) : BaseWorldComponent(ent) {}
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		void ReloadMeshCache();
		std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> GetMeshTree() const;
		std::shared_ptr<CHC> GetCHCController() const;
		virtual void InitializeLuaObject(lua_State *l) override;

		const rendering::RenderQueue *GetClusterRenderQueue(util::BSPTree::ClusterIndex clusterIndex, bool translucent = false) const;
		void SetBSPTree(const std::shared_ptr<util::BSPTree> &bspTree, const std::vector<std::vector<RenderMeshIndex>> &meshesPerCluster);
		const std::shared_ptr<util::BSPTree> &GetBSPTree() const;
		void RebuildRenderQueues();
	  protected:
		void BuildOfflineRenderQueues(bool rebuild = false);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		void UpdateRenderMeshes();
		void ReloadCHCController();
		std::vector<std::shared_ptr<rendering::RenderQueue>> m_clusterRenderQueues;
		std::vector<std::shared_ptr<rendering::RenderQueue>> m_clusterRenderTranslucentQueues;
		std::unordered_map<uint32_t, bool> m_lodBaseMeshIds;
		std::shared_ptr<OcclusionOctree<std::shared_ptr<ModelMesh>>> m_meshTree;
		std::shared_ptr<CHC> m_chcController;

		std::shared_ptr<util::BSPTree> m_bspTree = nullptr;
		std::vector<std::vector<RenderMeshIndex>> m_meshesPerCluster;
	};
};
#pragma warning(pop)

class CPolyMesh;
class ModelMesh;
class CHC;
template<class T>
class OcclusionOctree;
class DLLCLIENT CWorld : public CBaseEntity {
  public:
	virtual void Initialize() override;
	Con::c_cout &print(Con::c_cout &);
	std::ostream &print(std::ostream &);
};

#endif
