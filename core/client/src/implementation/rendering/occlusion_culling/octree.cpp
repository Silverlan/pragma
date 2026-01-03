// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <cassert>

module pragma.client;

import :rendering.occlusion_culling.octree;

import :debug;

BaseOcclusionOctree::Node::Node(BaseOcclusionOctree *tree, Node *parent) : m_tree(tree), m_parent((parent != nullptr) ? parent->shared_from_this() : std::weak_ptr<Node> {}) {}
BaseOcclusionOctree::Node::~Node() { m_tree->FreeNodeIndex(*this); }
uint32_t BaseOcclusionOctree::Node::GetTotalObjectCount() const { return m_branchObjectCount + GetObjectCount(); }
uint32_t BaseOcclusionOctree::Node::GetChildObjectCount() const { return m_branchObjectCount; }
bool BaseOcclusionOctree::Node::IsEmpty() const { return (GetTotalObjectCount() == 0) ? true : false; }
bool BaseOcclusionOctree::Node::HasObjects() const { return (GetObjectCount() > 0) ? true : false; }
uint32_t BaseOcclusionOctree::Node::GetChildCount() const
{
	if(m_children == nullptr)
		return 0;
	return m_children->size();
}

void BaseOcclusionOctree::Node::SetWorldBounds(const Vector3 &min, const Vector3 &max)
{
	m_worldBounds.first = min;
	m_worldBounds.second = max;
	m_dimensions = max - min;
}

const std::pair<Vector3, Vector3> &BaseOcclusionOctree::Node::GetWorldBounds() const { return m_worldBounds; }
const Vector3 &BaseOcclusionOctree::Node::GetDimensions() const { return m_dimensions; }
Vector3 BaseOcclusionOctree::Node::GetChildDimensions() const { return GetDimensions() * 0.5f; }
const std::array<std::shared_ptr<BaseOcclusionOctree::Node>, 8> *BaseOcclusionOctree::Node::GetChildren() const { return m_children.get(); }

bool BaseOcclusionOctree::Node::IsContained(const Vector3 &min, const Vector3 &max) const
{
	if(m_tree->IsSingleReferenceMode() == true)
		return pragma::math::intersection::aabb_in_aabb(min, max, m_worldBounds.first, m_worldBounds.second);
	return (pragma::math::intersection::aabb_aabb(min, max, m_worldBounds.first, m_worldBounds.second) != pragma::math::intersection::Intersect::Outside) ? true : false;
}

bool BaseOcclusionOctree::Node::UpdateState(OcclusionOctreeUpdateMode updateMode)
{
	uint32_t branchObjectCount = 0;
	auto childrenValid = true;
	if(m_children != nullptr) {
		for(auto &c : *m_children)
			branchObjectCount += c->GetTotalObjectCount();
		if(branchObjectCount == 0) {
			childrenValid = false;
			m_children = nullptr;
		}
	}
	auto oldObjectCount = m_branchObjectCount;
	m_branchObjectCount = branchObjectCount;
	if((updateMode == OcclusionOctreeUpdateMode::ForceUpdateParents || m_branchObjectCount != oldObjectCount) && m_parent.expired() == false) {
		if(updateMode == OcclusionOctreeUpdateMode::DontUpdateParents)
			return childrenValid;
		if(m_parent.lock()->UpdateState() == false) // TODO: Should this always update parents recursively? (i.e. ForceUpdateParents)
			return false;                           // This node is no longer valid, we have to bail out!
	}
	std::weak_ptr<Node> wp = shared_from_this();
	if(m_parent.expired() == true)
		m_tree->ShrinkRoot();
	if(wp.expired() == false)
		UpdateDebugObject();
	return childrenValid;
}

void BaseOcclusionOctree::Node::InitializeChildren(bool bPopulateChildren)
{
	if(m_children != nullptr || m_bIsFinal == true)
		return;
	// THIS CAUSES ERROR
	m_children = pragma::util::make_shared<std::array<std::shared_ptr<Node>, 8>>();
	for(auto &c : *m_children)
		c = m_tree->CreateNode(this);
	//

	auto dim = GetChildDimensions();
	auto minNodeSize = m_tree->GetMinNodeSize();

	uint32_t childIdx = 0;
	auto start = m_worldBounds.first;
	m_children->at(childIdx++)->SetWorldBounds(start, start + dim);

	start = m_worldBounds.first + Vector3(dim.x, 0, 0);
	m_children->at(childIdx++)->SetWorldBounds(start, start + dim);

	start = m_worldBounds.first + Vector3(0, dim.y, 0);
	m_children->at(childIdx++)->SetWorldBounds(start, start + dim);

	start = m_worldBounds.first + Vector3(0, 0, dim.z);
	m_children->at(childIdx++)->SetWorldBounds(start, start + dim);

	start = m_worldBounds.first + dim;
	m_children->at(childIdx++)->SetWorldBounds(start, start + dim);

	start = m_worldBounds.first + dim - Vector3(dim.x, 0, 0);
	m_children->at(childIdx++)->SetWorldBounds(start, start + dim);

	start = m_worldBounds.first + dim - Vector3(0, dim.y, 0);
	m_children->at(childIdx++)->SetWorldBounds(start, start + dim);

	start = m_worldBounds.first + dim - Vector3(0, 0, dim.z);
	m_children->at(childIdx++)->SetWorldBounds(start, start + dim);

	auto childBounds = m_children->front()->GetChildDimensions();
	if(childBounds.x < minNodeSize && childBounds.y < minNodeSize && childBounds.z < minNodeSize) {
		for(auto &c : *m_children)
			c->m_bIsFinal = true;
	}
	else if(bPopulateChildren == true) {
		for(auto &c : *m_children)
			c->InitializeChildren(bPopulateChildren);
	}
	if(m_tree->m_bDebugModeEnabled == true)
		m_tree->SetDebugModeEnabled(true);
}

void BaseOcclusionOctree::Node::UpdateDebugObject() const
{
	/*if(m_debugObject == nullptr)
		return;
	m_debugObject->SetVisible(GetTotalObjectCount() > 0);*/
}

BaseOcclusionOctree::Node *BaseOcclusionOctree::Node::GetParent() { return m_parent.lock().get(); }
const BaseOcclusionOctree::Node *BaseOcclusionOctree::Node::GetParent() const { return m_parent.lock().get(); }
BaseOcclusionOctree *BaseOcclusionOctree::Node::GetTree() { return m_tree; }
const BaseOcclusionOctree *BaseOcclusionOctree::Node::GetTree() const { return m_tree; }

void BaseOcclusionOctree::Node::SetIndex(uint32_t idx) { m_index = idx; }
uint32_t BaseOcclusionOctree::Node::GetIndex() const { return m_index; }
bool BaseOcclusionOctree::Node::IsLeaf() const { return (GetChildObjectCount() == 0) ? true : false; }

void BaseOcclusionOctree::Node::DebugPrint(const std::string &t) const
{
	auto &dim = GetDimensions();
	std::stringstream ss;
	ss << t << "(" << GetTotalObjectCount() << ") (Dim: " << dim << ") (Min: " << m_worldBounds.first << ") (Max: " << m_worldBounds.second << ")";
	if(IsEmpty() == true)
		Con::COUT << ss.str() << Con::endl;
	else
		Con::CCL << ss.str() << Con::endl;
	DebugPrintObjects(t);
	if(m_children == nullptr)
		return;
	for(auto &c : *m_children)
		c->DebugPrint(t + "\t");
}

void BaseOcclusionOctree::Node::DebugDraw(bool b, bool applyToChildren, uint32_t depth) const
{
	if(b == false) {
		//m_debugObject = nullptr;
		if(m_children != nullptr) {
			for(auto &c : *m_children)
				c->DebugDraw(false);
		}
		return;
	}
	/*if(m_debugObject == nullptr) {
		const std::array<std::pair<const Color *, const Color *>, 7> colors = {std::pair<const Color *, const Color *> {&colors::Red, &colors::Lime}, std::pair<const Color *, const Color *> {&colors::Lime, &colors::Aqua}, std::pair<const Color *, const Color *> {&colors::Aqua, &colors::Magenta},
		  std::pair<const Color *, const Color *> {&colors::Magenta, &colors::Yellow}, std::pair<const Color *, const Color *> {&colors::Yellow, &colors::Teal}, std::pair<const Color *, const Color *> {&colors::Teal, &colors::Maroon},
		  std::pair<const Color *, const Color *> {&colors::Maroon, &colors::Red}};
		depth = depth % colors.size();
		if(HasObjects() == true) {
			auto &col = colors.at(depth);
			auto colMesh = *col.first;
			colMesh.a = 64;
			auto center = (m_worldBounds.first + m_worldBounds.second) * 0.5f;
			auto min = (m_worldBounds.first - center) - Vector3(1.f, 1.f, 1.f);
			auto max = (m_worldBounds.second - center) - Vector3(1.f, 1.f, 1.f);
			DebugRenderInfo renderInfo {colMesh, *col.second};
			renderInfo.SetOrigin(center);
			m_debugObject = DebugRenderer::DrawBox(min, max, renderInfo);
		}
		UpdateDebugObject();
	}*/
	if(applyToChildren == true && m_children != nullptr) {
		for(auto &c : *m_children)
			c->DebugDraw(b, applyToChildren, depth + 1);
	}
}

///////////////////////////////////////

BaseOcclusionOctree::BaseOcclusionOctree(float minNodeSize, float maxNodeSize, float initialBounds) : m_minNodeSize(minNodeSize), m_maxNodeSize(maxNodeSize), m_initialBounds(initialBounds) {}

BaseOcclusionOctree::~BaseOcclusionOctree()
{
	m_root = nullptr; // Make sure tree is destroyed before everything else
}

void BaseOcclusionOctree::Initialize()
{
	m_root = CreateNode();
	m_root->SetWorldBounds(Vector3(-m_initialBounds, -m_initialBounds, -m_initialBounds), Vector3(m_initialBounds, m_initialBounds, m_initialBounds));
	m_root->InitializeChildren();
}

void BaseOcclusionOctree::InitializeNode(Node &node)
{
	auto idx = decltype(m_nextNodeIndex) {0};
	if(m_freeIndices.empty() == false) {
		idx = m_freeIndices.front();
		m_freeIndices.pop();
	}
	else
		idx = m_nextNodeIndex++;
	node.SetIndex(idx);
}

void BaseOcclusionOctree::IterateTree(const std::function<bool(const Node &)> fNodeCallback) const
{
	auto &root = GetRootNode();
	std::function<void(const Node &)> fIterateNode = nullptr;
	fIterateNode = [&fIterateNode, &fNodeCallback](const Node &node) {
		if(fNodeCallback(node) == false)
			return;
		auto *children = node.GetChildren();
		if(children == nullptr)
			return;
		for(auto &c : *children)
			fIterateNode(*c);
	};
	fIterateNode(root);
}

void BaseOcclusionOctree::FreeNodeIndex(const Node &node) { m_freeIndices.push(node.GetIndex()); }

CallbackHandle BaseOcclusionOctree::AddNodeCreatedCallback(const std::function<void(std::reference_wrapper<const Node>)> &callback)
{
	auto hCb = FunctionCallback<void, std::reference_wrapper<const Node>>::Create(callback);
	m_nodeCreatedCallbacks.push_back(hCb);
	return hCb;
}
CallbackHandle BaseOcclusionOctree::AddNodeDestroyedCallback(const std::function<void(std::reference_wrapper<const Node>)> &callback)
{
	auto hCb = FunctionCallback<void, std::reference_wrapper<const Node>>::Create(callback);
	m_nodeDestroyedCallback.push_back(hCb);
	return hCb;
}

uint32_t BaseOcclusionOctree::GetMaxNodeCount() const { return m_nextNodeIndex; }

void BaseOcclusionOctree::SetSingleReferenceMode(bool b) { m_bRefOnce = b; }
bool BaseOcclusionOctree::IsSingleReferenceMode() const { return m_bRefOnce; }

void BaseOcclusionOctree::SetDebugModeEnabled(bool b) const
{
	m_root->DebugDraw(b);
	m_bDebugModeEnabled = b;
}

void BaseOcclusionOctree::ExtendRoot(const Vector3 &origin)
{
	auto &dim = m_root->GetDimensions();
	auto newDim = dim * 2.f;
	auto maxDim = GetMaxNodeSize();
	if(pragma::math::abs(newDim.x) > maxDim || pragma::math::abs(newDim.y) > maxDim || pragma::math::abs(newDim.z) > maxDim)
		return;
	auto newNode = CreateNode();
	newNode->SetWorldBounds(m_root->m_worldBounds.first, m_root->m_worldBounds.first + newDim);
	for(uint8_t i = 0; i < 3; ++i) {
		if(origin[i] < newNode->m_worldBounds.first[i]) {
			newNode->m_worldBounds.first[i] = newNode->m_worldBounds.first[i] - dim[i];
			newNode->m_worldBounds.second[i] = newNode->m_worldBounds.second[i] - dim[i];
		}
	}
	newNode->InitializeChildren();

	// Find new child-node which has the same position as our root-node
	auto dClosest = std::numeric_limits<float>::max();
	std::shared_ptr<Node> *closestNode = nullptr;
	for(auto &c : *newNode->m_children) {
		auto d = uvec::length_sqr(c->m_worldBounds.first - m_root->m_worldBounds.first);
		if(d < dClosest) {
			dClosest = d;
			closestNode = &c;
		}
	}
	assert(closestNode != nullptr);
	assert(dClosest < 0.01f); // The node should have the exact same position; If not, something went wrong
	if(closestNode == nullptr) {
		auto &bounds = newNode->GetWorldBounds();
		Con::CWAR << "Unable to extend occlusion tree node. Invalid dimensions? (" << origin.x << "," << origin.y << "," << origin.z << ") (" << bounds.first.x << "," << bounds.first.y << "," << bounds.first.x << ") (" << bounds.second.x << "," << bounds.second.y << "," << bounds.second.z
		          << ")" << Con::endl;
		return;
	}

	// Replace node with our root-node
	*closestNode = m_root;
	newNode->m_branchObjectCount = m_root->GetTotalObjectCount();
	m_root->m_parent = newNode;
	m_root = newNode;
}

void BaseOcclusionOctree::ShrinkRoot()
{
	if(m_root->m_children == nullptr || m_root->m_children->empty() == true || m_root->HasObjects() == true)
		return;
	std::shared_ptr<Node> *newRootNode = nullptr;
	for(auto &c : *m_root->m_children) {
		if(c->IsEmpty() == false) {
			if(newRootNode != nullptr)
				return;
			newRootNode = &c;
		}
	}
	if(newRootNode == nullptr)
		return;
	(*newRootNode)->m_parent = {};
	m_root = *newRootNode;
	ShrinkRoot();
}

typename BaseOcclusionOctree::Node &BaseOcclusionOctree::GetRootNode() { return *m_root; }
const typename BaseOcclusionOctree::Node &BaseOcclusionOctree::GetRootNode() const { return *m_root; }

float BaseOcclusionOctree::GetMinNodeSize() const { return m_minNodeSize; }
float BaseOcclusionOctree::GetMaxNodeSize() const { return m_maxNodeSize; }
void BaseOcclusionOctree::DebugPrint() const { m_root->DebugPrint(); }
