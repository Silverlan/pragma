/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_OCCLUSION_OCTREE_HPP__
#define __C_OCCLUSION_OCTREE_HPP__

#include "pragma/clientdefinitions.h"
#include <array>
#include <vector>
#include <cinttypes>
#include <memory>
#include <queue>
#include <functional>

namespace DebugRenderer {class BaseObject;};

#define ENABLE_OCCLUSION_DEBUG_MODE 0

#pragma warning(push)
#pragma warning(disable : 4251)
class BaseOcclusionOctree
{
public:
	class DLLCLIENT Node
		: public std::enable_shared_from_this<Node>
	{
	public:
		virtual ~Node();
		// Object count of child-branches, as well as own objects
		uint32_t GetTotalObjectCount() const;
		// Object count of child-branches, not counting own objects
		uint32_t GetChildObjectCount() const;
		virtual uint32_t GetObjectCount() const=0;
		uint32_t GetChildCount() const;
		bool IsEmpty() const;
		const std::pair<Vector3,Vector3> &GetWorldBounds() const;
		const Vector3 &GetDimensions() const;
		Vector3 GetChildDimensions() const;
		bool IsContained(const Vector3 &min,const Vector3 &max) const;
		bool UpdateState(bool bForceUpdateParents=false);
		void InitializeChildren(bool bPopulateChildren=false);
		const std::array<std::shared_ptr<Node>,8> *GetChildren() const;
		bool HasObjects() const;
		Node *GetParent();
		const Node *GetParent() const;
		BaseOcclusionOctree *GetTree();
		const BaseOcclusionOctree *GetTree() const;
		uint32_t GetIndex() const;
		bool IsLeaf() const;

		void DebugPrint(const std::string &t="") const;
	protected:
		friend BaseOcclusionOctree;
		Node(BaseOcclusionOctree *tree,Node *parent=nullptr);
		void SetWorldBounds(const Vector3 &min,const Vector3 &max);
		void SetIndex(uint32_t idx);
		// Debug
		virtual void DebugPrintObjects(const std::string &t) const=0;
		void DebugDraw(bool b,bool applyToChildren=true,uint32_t depth=0) const;
		void UpdateDebugObject() const;

		std::shared_ptr<std::array<std::shared_ptr<Node>,8>> m_children;
		bool m_bIsFinal = false;
		uint32_t m_index = 0;
		std::weak_ptr<Node> m_parent = {};
		BaseOcclusionOctree * const m_tree = nullptr;
		// Total object count in children (Not counting own objects)
		uint32_t m_branchObjectCount = 0;
		std::pair<Vector3,Vector3> m_worldBounds;
		Vector3 m_dimensions;
	private:
		// Debug
		mutable std::shared_ptr<DebugRenderer::BaseObject> m_debugObject;
	};
	virtual ~BaseOcclusionOctree();
	// If set to true, only one reference of a object can exist in the tree at a time.
	// This is better for performance, but might not be accurate in all cases.
	void SetSingleReferenceMode(bool b);
	bool IsSingleReferenceMode() const;
	void Initialize();
	void ExtendRoot(const Vector3 &origin);
	void ShrinkRoot();
	float GetMinNodeSize() const;
	float GetMaxNodeSize() const;
	uint32_t GetMaxNodeCount() const;
	Node &GetRootNode();
	const Node &GetRootNode() const;
	CallbackHandle AddNodeCreatedCallback(const std::function<void(std::reference_wrapper<const Node>)> &callback);
	CallbackHandle AddNodeDestroyedCallback(const std::function<void(std::reference_wrapper<const Node>)> &callback);
	void IterateTree(const std::function<bool(const Node&)> fNodeCallback) const;

	// Debug
	void SetDebugModeEnabled(bool b) const;
	virtual void DebugPrint() const;
protected:
	BaseOcclusionOctree(float minNodeSize,float maxNodeSize,float initialBounds);
	virtual std::shared_ptr<Node> CreateNode(Node *parent=nullptr)=0;
	void InitializeNode(Node &node);
	void FreeNodeIndex(const Node &node);

	uint32_t m_nextNodeIndex = 0;
	bool m_bRefOnce = false;
	float m_minNodeSize = 0.f;
	float m_maxNodeSize = std::numeric_limits<float>::max();
	float m_initialBounds = 0.f;
	std::shared_ptr<Node> m_root = nullptr;
	std::vector<CallbackHandle> m_nodeCreatedCallbacks;
	std::vector<CallbackHandle> m_nodeDestroyedCallback;
private:
	std::queue<uint32_t> m_freeIndices;
	// Debug
	mutable bool m_bDebugModeEnabled = false;
};

template<class T>
	class DLLCLIENT OcclusionOctree
		: public BaseOcclusionOctree
{
public:
	class DLLCLIENT Node
		: public BaseOcclusionOctree::Node
	{
	public:
		virtual uint32_t GetObjectCount() const override;
		bool HasObject(const T &o) const;
		void RemoveObject(const T &o,bool bSkipCheck=false);
		bool InsertObject(const T &o,const Vector3 &min,const Vector3 &max,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted,bool bForceInsert=false);
		bool InsertObjectReverse(const T &o,const Vector3 &min,const Vector3 &max,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted);
		const std::vector<T> &GetObjects() const;
		OcclusionOctree<T> *GetTree();
		const OcclusionOctree<T> *GetTree() const;
	protected:
		static std::shared_ptr<Node> Create(OcclusionOctree<T> *tree,Node *parent=nullptr);
		friend OcclusionOctree;
		// Debug
		virtual void DebugPrintObjects(const std::string &t) const override;
	private:
		Node(BaseOcclusionOctree *tree,Node *parent=nullptr) : BaseOcclusionOctree::Node(tree,parent) {};
		std::vector<T> m_objects;
	};
	Node &GetRootNode();
	const Node &GetRootNode() const;
	// An object mustn't be inserted multiple times!
	void InsertObject(const T &o);
	void UpdateObject(const T &o);
	void RemoveObject(const T &o);
	bool ContainsObject(const T &o);
	void InsertObjectAndExtendRoot(const T &o,const Vector3 &min,const Vector3 &max,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted);
	void SetToStringCallback(const std::function<std::string(const T&)> &callback);
	void IterateObjects(const std::function<bool(const Node&)> fNodeCallback,const std::function<void(const T&)> &fObjCallback) const;
	OcclusionOctree(float minNodeSize,float maxNodeSize,float initialBounds,const std::function<void(const T&,Vector3&,Vector3&)> &factory);

	virtual void DebugPrint() const override;
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
	enum class DLLCLIENT ValidationError : uint32_t
	{
		Success = 0,
		InvalidObjectReferenceCount,
		InvalidObjectReferenceToNode,
		MissingObject,
		DuplicateObject
	};
	ValidationError Validate(const T **r=nullptr) const;
#endif
protected:
	void GetObjectBounds(const T &o,Vector3 &min,Vector3 &max) const;
	void RemoveNodeReference(const Node &node,const T &o);
private:
	virtual std::shared_ptr<BaseOcclusionOctree::Node> CreateNode(BaseOcclusionOctree::Node *parent=nullptr) override;

	std::function<void(const T&,Vector3&,Vector3&)> m_objectBoundsCallback = nullptr;
	std::function<std::string(const T&)> m_objectToStringCallback = nullptr;
	std::unordered_map<T,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>>> m_objectNodes;
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
	std::vector<T> m_dbgObjects;
#endif
};
#pragma warning(pop)

#endif
