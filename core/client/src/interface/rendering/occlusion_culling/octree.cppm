// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

export module pragma.client:rendering.occlusion_culling.octree;

export import pragma.shared;

#pragma warning(push)
#pragma warning(disable : 4251)
export {
	enum class OcclusionOctreeUpdateMode : uint8_t { Default = 0, ForceUpdateParents, DontUpdateParents };
	class BaseOcclusionOctree {
	  public:
		class DLLCLIENT Node : public std::enable_shared_from_this<Node> {
		  public:
			virtual ~Node();
			// Object count of child-branches, as well as own objects
			uint32_t GetTotalObjectCount() const;
			// Object count of child-branches, not counting own objects
			uint32_t GetChildObjectCount() const;
			virtual uint32_t GetObjectCount() const = 0;
			uint32_t GetChildCount() const;
			bool IsEmpty() const;
			const std::pair<Vector3, Vector3> &GetWorldBounds() const;
			const Vector3 &GetDimensions() const;
			Vector3 GetChildDimensions() const;
			bool IsContained(const Vector3 &min, const Vector3 &max) const;
			bool UpdateState(OcclusionOctreeUpdateMode updateMode = OcclusionOctreeUpdateMode::Default);
			void InitializeChildren(bool bPopulateChildren = false);
			const std::array<std::shared_ptr<Node>, 8> *GetChildren() const;
			bool HasObjects() const;
			Node *GetParent();
			const Node *GetParent() const;
			BaseOcclusionOctree *GetTree();
			const BaseOcclusionOctree *GetTree() const;
			uint32_t GetIndex() const;
			bool IsLeaf() const;

			void DebugPrint(const std::string &t = "") const;
		  protected:
			friend BaseOcclusionOctree;
			Node(BaseOcclusionOctree *tree, Node *parent = nullptr);
			void SetWorldBounds(const Vector3 &min, const Vector3 &max);
			void SetIndex(uint32_t idx);
			// Debug
			virtual void DebugPrintObjects(const std::string &t) const = 0;
			void DebugDraw(bool b, bool applyToChildren = true, uint32_t depth = 0) const;
			void UpdateDebugObject() const;

			std::shared_ptr<std::array<std::shared_ptr<Node>, 8>> m_children;
			bool m_bIsFinal = false;
			uint32_t m_index = 0;
			std::weak_ptr<Node> m_parent = {};
			BaseOcclusionOctree *const m_tree = nullptr;
			// Total object count in children (Not counting own objects)
			uint32_t m_branchObjectCount = 0;
			std::pair<Vector3, Vector3> m_worldBounds;
			Vector3 m_dimensions;
		  private:
			// Debug
			// mutable std::shared_ptr<DebugRenderer::BaseObject> m_debugObject;
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
		void IterateTree(const std::function<bool(const Node &)> fNodeCallback) const;

		// Debug
		void SetDebugModeEnabled(bool b) const;
		virtual void DebugPrint() const;
	  protected:
		BaseOcclusionOctree(float minNodeSize, float maxNodeSize, float initialBounds);
		virtual std::shared_ptr<Node> CreateNode(Node *parent = nullptr) = 0;
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

	enum class OcclusionOctreeInsertResult : uint8_t { ObjectOutOfBounds = 0, ObjectAlreadyIncluded, ObjectInsertedInChildNode, ObjectInserted };
	template<class T>
	class DLLCLIENT OcclusionOctree : public BaseOcclusionOctree {
	  public:
		class DLLCLIENT Node : public BaseOcclusionOctree::Node {
		  public:
			virtual uint32_t GetObjectCount() const override { return static_cast<uint32_t>(m_objects.size()); }
			bool HasObject(const T &o) const { return (std::find(m_objects.begin(), m_objects.end(), o) != m_objects.end()) ? true : false; }
			void RemoveObject(const T &o, bool bSkipCheck = false)
			{
				auto it = std::find(m_objects.begin(), m_objects.end(), o);
				if(it == m_objects.end())
					return;
				GetTree()->RemoveNodeReference(*this, o);
				m_objects.erase(it);
				if(bSkipCheck != false)
					UpdateState();
			}
			OcclusionOctreeInsertResult InsertObject(const T &o, const Vector3 &min, const Vector3 &max, std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted, bool bForceInsert = false)
			{
				if(bForceInsert == false && IsContained(min, max) == false)
					return OcclusionOctreeInsertResult::ObjectOutOfBounds;
				if(HasObject(o) == true)
					return OcclusionOctreeInsertResult::ObjectAlreadyIncluded;
				InitializeChildren();
				auto bInChildren = false;
				if(m_children != nullptr) {
					for(auto &c : *m_children) {
						if(static_cast<Node *>(c.get())->InsertObject(o, min, max, nodesInserted) != OcclusionOctreeInsertResult::ObjectOutOfBounds) {
							bInChildren = true;
							if(m_tree->IsSingleReferenceMode() == true) {
								UpdateState(OcclusionOctreeUpdateMode::DontUpdateParents);
								return OcclusionOctreeInsertResult::ObjectInsertedInChildNode;
							}
						}
					}
				}
				if(bInChildren == true) {
					UpdateState(OcclusionOctreeUpdateMode::DontUpdateParents);
					return OcclusionOctreeInsertResult::ObjectInsertedInChildNode;
				}
				m_objects.push_back(o);
				nodesInserted.push_back(shared_from_this());
				//if(IsEmpty() == true)
				//	UpdateState();
				UpdateState(OcclusionOctreeUpdateMode::DontUpdateParents);
				return OcclusionOctreeInsertResult::ObjectInserted;
			}
			bool InsertObjectReverse(const T &o, const Vector3 &min, const Vector3 &max, std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted)
			{
				auto *parent = GetParent();
				while(parent != nullptr) {
					if(static_cast<Node *>(parent)->InsertObject(o, min, max, nodesInserted) != OcclusionOctreeInsertResult::ObjectOutOfBounds)
						return true;
					parent = parent->GetParent();
				}
				if(parent == nullptr)
					GetTree()->InsertObjectAndExtendRoot(o, min, max, nodesInserted);
				return false;
			}
			const std::vector<T> &GetObjects() const { return m_objects; }
			OcclusionOctree<T> *GetTree() { return static_cast<OcclusionOctree<T> *>(m_tree); }
			const OcclusionOctree<T> *GetTree() const { return static_cast<OcclusionOctree<T> *>(m_tree); }
		  protected:
			static std::shared_ptr<Node> Create(OcclusionOctree<T> *tree, Node *parent = nullptr) { return std::shared_ptr<Node>(new Node(tree, parent)); }
			friend OcclusionOctree;
			// Debug
			virtual void DebugPrintObjects(const std::string &t) const override
			{
				using ::operator<<; // Workaround for msvc compiler being unable to resolve Con::CCL<< operator (compiler bug?)
				for(auto &o : m_objects) {
					Con::CCL << t;
					auto *tree = GetTree();
					if(tree->m_objectToStringCallback != nullptr)
						Con::CCL << tree->m_objectToStringCallback(o);
					else
						Con::CCL << &o;
					Con::CCL << Con::endl;
				}
			}
		  private:
			Node(BaseOcclusionOctree *tree, Node *parent = nullptr) : BaseOcclusionOctree::Node(tree, parent) {};
			std::vector<T> m_objects;
		};
		Node &GetRootNode() { return static_cast<Node &>(*m_root.get()); }
		const Node &GetRootNode() const { return static_cast<Node &>(*m_root.get()); }
		// An object mustn't be inserted multiple times!
		void InsertObject(const T &o) { InsertObject(o, nullptr); }
		void UpdateObject(const T &o)
		{
			auto it = m_objectNodes.find(o);
			if(it == m_objectNodes.end())
				return;
			if(IsSingleReferenceMode() == false) {
				Vector3 min, max;
				GetObjectBounds(o, min, max);
				auto nodesCopy = it->second;
				for(auto &node : nodesCopy) {
					if(node.expired() == true)
						continue;
					static_cast<Node *>(node.lock().get())->RemoveObject(o, false);
				}
				InsertObject(o);
				for(auto &node : nodesCopy) {
					if(node.expired() == true)
						continue;
					node.lock()->UpdateState();
				}
			}
			else {
				assert(it->second.size() == 1);
				if(it->second.empty() == true)
					return;
				auto &node = it->second.front();
				if(node.expired() == true)
					return;
				auto *n = static_cast<Node *>(node.lock().get());
				Vector3 min, max;
				GetObjectBounds(o, min, max);
				n->RemoveObject(o, false);
				it = m_objectNodes.find(o);
				if(it == m_objectNodes.end())
					it = m_objectNodes.insert(typename decltype(m_objectNodes)::value_type(o, std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> {})).first;
				// Attempt to re-insert object into node again
				std::weak_ptr<BaseOcclusionOctree::Node> wp = n->shared_from_this();
				if(n->InsertObject(o, min, max, it->second) == OcclusionOctreeInsertResult::ObjectOutOfBounds) {
					if(!wp.expired())
						n->InsertObjectReverse(o, min, max, it->second);
					if(!wp.expired())
						n->UpdateState(OcclusionOctreeUpdateMode::ForceUpdateParents);
				}
			}
		}
		void RemoveObject(const T &o)
		{
			auto it = m_objectNodes.find(o);
			if(it == m_objectNodes.end())
				return;
			auto nodesCopy = it->second;
			for(auto &node : nodesCopy) {
				if(node.expired() == true)
					continue;
				static_cast<Node *>(node.lock().get())->RemoveObject(o);
			}

			// We need to check for the object again, because the map may have been changed by the above calls
			it = m_objectNodes.find(o);
			if(it != m_objectNodes.end()) {
				m_objectNodes.erase(it);
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
				auto it = std::find_if(m_dbgObjects.begin(), m_dbgObjects.end(), [&o](const T &oRef) { return (o == oRef) ? true : false; });
				if(it != m_dbgObjects.end())
					m_dbgObjects.erase(it);
#endif
			}
		}
		bool ContainsObject(const T &o)
		{
			auto it = m_objectNodes.find(o);
			return (it != m_objectNodes.end()) ? true : false;
		}
		void InsertObjectAndExtendRoot(const T &o, const Vector3 &min, const Vector3 &max, std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted)
		{
			static auto recursiveCount = std::numeric_limits<uint32_t>::max();
			const uint32_t maxRecursiveCount = 4;
			if(recursiveCount == std::numeric_limits<uint32_t>::max())
				recursiveCount = maxRecursiveCount;
			auto v = recursiveCount;
			auto &root = GetRootNode();
			if(recursiveCount == 0) {
				recursiveCount = std::numeric_limits<uint32_t>::max();
				InsertObject(o, &root); // Force object into root node
				// Con::CWAR<<"Object "<<o<<" outside of occlusion tree bounds! Forcing in root node..."<<Con::endl;
				return;
			}
			--recursiveCount;
			std::vector<Vector3> entAabbPoints = {min, Vector3(min.x, min.y, max.z), Vector3(min.x, max.y, min.z), Vector3(max.x, min.y, min.z), Vector3(min.x, max.y, max.z), Vector3(max.x, min.y, min.z), Vector3(max.x, min.y, max.z), max};
			auto dFurthest = -1.f;
			Vector3 *pFurthest = nullptr;
			for(auto &p : entAabbPoints) {
				Vector3 r;
				auto &rootBounds = root.GetWorldBounds();
				pragma::math::geometry::closest_point_on_aabb_to_point(rootBounds.first, rootBounds.second, p, &r);
				auto d = uvec::length_sqr(p - r);
				if(d > dFurthest) {
					dFurthest = d;
					pFurthest = &p;
				}
			}
			assert(pFurthest != nullptr);
			if(pFurthest == nullptr) // Can happen if object bounds are NaN or similar
			{
				Con::CWAR << "Object " << o << " has invalid bounds (" << min.x << "," << min.y << "," << min.z << ") (" << max.x << "," << max.y << "," << max.z << ")! Object will not be rendered!" << Con::endl;
				return;
			}
			ExtendRoot(*pFurthest);
			InsertObject(o);

			recursiveCount = std::numeric_limits<uint32_t>::max();
			if(v == maxRecursiveCount - 1)
				ShrinkRoot(); // Shrink root once all recursions are complete
		}
		void SetToStringCallback(const std::function<std::string(const T &)> &callback) { m_objectToStringCallback = callback; }
		void IterateObjects(const std::function<bool(const Node &)> fNodeCallback, const std::function<void(const T &)> &fObjCallback) const
		{
			auto &root = GetRootNode();
			std::function<void(const Node &)> fIterateNode = nullptr;
			fIterateNode = [&fIterateNode, &fNodeCallback, &fObjCallback](const Node &node) {
				if(fNodeCallback(node) == false)
					return;
				auto &objs = node.GetObjects();
				for(auto &o : objs)
					fObjCallback(o);
				if(node.GetChildObjectCount() == 0)
					return;
				auto *children = node.GetChildren();
				if(children == nullptr)
					return;
				for(auto &c : *children)
					fIterateNode(*static_cast<Node *>(c.get()));
			};
			fIterateNode(root);
		}
		OcclusionOctree(float minNodeSize, float maxNodeSize, float initialBounds, const std::function<void(const T &, Vector3 &, Vector3 &)> &factory) : BaseOcclusionOctree(minNodeSize, maxNodeSize, initialBounds), m_objectBoundsCallback(factory) {}

		virtual void DebugPrint() const override
		{
			BaseOcclusionOctree::DebugPrint();
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
			const T *o = nullptr;
			auto validErr = Validate(&o);
			Con::COUT << "Validation Result: ";
			switch(validErr) {
			case ValidationError::Success:
				Con::COUT << "Success";
				break;
			case ValidationError::InvalidObjectReferenceCount:
				Con::COUT << "InvalidObjectReferenceCount";
				break;
			case ValidationError::InvalidObjectReferenceToNode:
				Con::COUT << "InvalidObjectReferenceToNode";
				break;
			case ValidationError::MissingObject:
				Con::COUT << "MissingObject";
				break;
			case ValidationError::DuplicateObject:
				Con::COUT << "DuplicateObject";
				break;
			default:
				Con::COUT << "Unknown";
				break;
			}
			if(o != nullptr) {
				Con::COUT << ": ";
				if(m_objectToStringCallback == nullptr)
					Con::COUT << o;
				else
					Con::COUT << m_objectToStringCallback(*o);
			}
			Con::COUT << Con::endl;
#endif
		}
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
		enum class DLLCLIENT ValidationError : uint32_t { Success = 0, InvalidObjectReferenceCount, InvalidObjectReferenceToNode, MissingObject, DuplicateObject };
		ValidationError Validate(const T **r = nullptr) const
		{
			if(m_objectNodes.size() != m_dbgObjects.size())
				return ValidationError::InvalidObjectReferenceCount;
			for(auto &pair : m_objectNodes) {
				for(auto &n : pair.second) {
					if(n.expired() == true)
						continue;
					auto node = n.lock();
					if(static_cast<Node *>(node.get())->HasObject(pair.first) == false) {
						if(r != nullptr)
							*r = &pair.first;
						return ValidationError::InvalidObjectReferenceToNode;
					}
				}
			}
			std::function<bool(const Node &, const T &, uint32_t &)> fIterateTree = nullptr;
			fIterateTree = [&fIterateTree](const Node &node, const T &o, uint32_t &count) -> bool {
				if(node.HasObject(o) == true) {
					++count;
					return true;
				}
				auto *children = node.GetChildren();
				if(children == nullptr)
					return false;
				for(auto &child : *children) {
					if(fIterateTree(*static_cast<Node *>(child.get()), o, count) == true)
						return true;
				}
				return false;
			};
			auto &node = GetRootNode();
			for(auto &o : m_dbgObjects) {
				uint32_t count = 0;
				if(fIterateTree(node, o, count) == false) {
					if(r != nullptr)
						*r = &o;
					return ValidationError::MissingObject;
				}
				else if(count > 1 && IsSingleReferenceMode() == true) {
					if(r != nullptr)
						*r = &o;
					return ValidationError::DuplicateObject;
				}
			}
			return ValidationError::Success;
		}
#endif
	  protected:
		void InsertObject(const T &o, Node *optNode)
		{
			auto it = m_objectNodes.find(o);
			if(it == m_objectNodes.end())
				it = m_objectNodes.insert(typename decltype(m_objectNodes)::value_type(o, std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> {})).first;
			else if(it->second.size() > 0)
				return; // Object already exists in tree

#if ENABLE_OCCLUSION_DEBUG_MODE == 1
			m_dbgObjects.push_back(o);
#endif
			auto &nodes = it->second;
			Vector3 min, max;
			GetObjectBounds(o, min, max);
			if(optNode == nullptr
			  && (min.x == std::numeric_limits<float>::lowest() || min.y == std::numeric_limits<float>::lowest() || min.z == std::numeric_limits<float>::lowest() || max.x == std::numeric_limits<float>::max() || max.y == std::numeric_limits<float>::max()
			    || max.z == std::numeric_limits<float>::max())) {
				// Note: This case usually indicates some kind of error, objects inserted into the tree should never be this large.
				// We'll just force-insert it into the root node and don't grow the tree.
				m_objectNodes.erase(it);
				InsertObject(o, static_cast<Node *>(m_root.get()));
				return;
			}
			auto &root = optNode ? *optNode : GetRootNode();
			std::weak_ptr<BaseOcclusionOctree::Node> wp = root.shared_from_this();
			if(root.InsertObject(o, min, max, nodes, optNode ? true : false) == OcclusionOctreeInsertResult::ObjectOutOfBounds) {
				m_objectNodes.erase(it);
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
				m_dbgObjects.erase(m_dbgObjects.end() - 1);
#endif
				if(optNode == nullptr && wp.expired() == false)
					root.InsertObjectReverse(o, min, max, nodes);
			}
		}
		void GetObjectBounds(const T &o, Vector3 &min, Vector3 &max) const { m_objectBoundsCallback(o, min, max); }
		void RemoveNodeReference(const Node &node, const T &o)
		{
			auto it = m_objectNodes.find(o);
			if(it == m_objectNodes.end())
				return;
			if(IsSingleReferenceMode() == true) {
				assert(it->second.size() <= 1);
				it->second.clear();
				return;
			}
			auto itNode = std::find_if(it->second.begin(), it->second.end(), [&node](const std::weak_ptr<BaseOcclusionOctree::Node> &nodeOther) { return (nodeOther.lock().get() == &node) ? true : false; });
			if(itNode == it->second.end())
				return;
			it->second.erase(itNode);
			if(it->second.empty())
				m_objectNodes.erase(it);
		}
	  private:
		virtual std::shared_ptr<BaseOcclusionOctree::Node> CreateNode(BaseOcclusionOctree::Node *parent = nullptr) override
		{
			auto n = Node::Create(this, static_cast<Node *>(parent));
			InitializeNode(*n);
			return n;
		}

		std::function<void(const T &, Vector3 &, Vector3 &)> m_objectBoundsCallback = nullptr;
		std::function<std::string(const T &)> m_objectToStringCallback = nullptr;
		std::unordered_map<T, std::vector<std::weak_ptr<BaseOcclusionOctree::Node>>> m_objectNodes;
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
		std::vector<T> m_dbgObjects;
#endif
	};
};
#pragma warning(pop)
