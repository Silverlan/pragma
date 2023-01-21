/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_OCCLUSION_OCTREE_IMPL_HPP__
#define __C_OCCLUSION_OCTREE_IMPL_HPP__

#include "pragma/rendering/occlusion_culling/c_occlusion_octree.hpp"
#include "pragma/game/c_game.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/debug/c_debugoverlay.h"
#include <pragma/math/intersection.h>
#include <sstream>

template<class T>
	std::shared_ptr<typename OcclusionOctree<T>::Node> OcclusionOctree<T>::Node::Create(OcclusionOctree<T> *tree,Node *parent)
{
	return std::shared_ptr<Node>(new Node(tree,parent));
}

template<class T>
	bool OcclusionOctree<T>::Node::HasObject(const T &o) const {return (std::find(m_objects.begin(),m_objects.end(),o) != m_objects.end()) ? true : false;}

template<class T>
	uint32_t OcclusionOctree<T>::Node::GetObjectCount() const {return static_cast<uint32_t>(m_objects.size());}

template<class T>
	const std::vector<T> &OcclusionOctree<T>::Node::GetObjects() const {return m_objects;}

template<class T>
	void OcclusionOctree<T>::Node::RemoveObject(const T &o,bool bSkipCheck)
{
	auto it = std::find(m_objects.begin(),m_objects.end(),o);
	if(it == m_objects.end())
		return;
	GetTree()->RemoveNodeReference(*this,o);
	m_objects.erase(it);
	if(bSkipCheck != false)
		UpdateState();
}

template<class T>
	OcclusionOctreeInsertResult OcclusionOctree<T>::Node::InsertObject(const T &o,const Vector3 &min,const Vector3 &max,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted,bool bForceInsert)
{
	if(bForceInsert == false && IsContained(min,max) == false)
		return OcclusionOctreeInsertResult::ObjectOutOfBounds;
	if(HasObject(o) == true)
		return OcclusionOctreeInsertResult::ObjectAlreadyIncluded;
	InitializeChildren();
	auto bInChildren = false;
	if(m_children != nullptr)
	{
		for(auto &c : *m_children)
		{
			if(static_cast<Node*>(c.get())->InsertObject(o,min,max,nodesInserted) != OcclusionOctreeInsertResult::ObjectOutOfBounds)
			{
				bInChildren = true;
				if(m_tree->IsSingleReferenceMode() == true)
				{
					UpdateState(OcclusionOctreeUpdateMode::DontUpdateParents);
					return OcclusionOctreeInsertResult::ObjectInsertedInChildNode;
				}
			}
		}
	}
	if(bInChildren == true)
	{
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

template<class T>
	bool OcclusionOctree<T>::Node::InsertObjectReverse(const T &o,const Vector3 &min,const Vector3 &max,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted)
{
	auto *parent = GetParent();
	while(parent != nullptr)
	{
		if(static_cast<Node*>(parent)->InsertObject(o,min,max,nodesInserted) != OcclusionOctreeInsertResult::ObjectOutOfBounds)
			return true;
		parent = parent->GetParent();
	}
	if(parent == nullptr)
		GetTree()->InsertObjectAndExtendRoot(o,min,max,nodesInserted);
	return false;
}

template<class T>
	void OcclusionOctree<T>::Node::DebugPrintObjects(const std::string &t) const
{
	for(auto &o : m_objects)
	{
		Con::ccl<<t;
		auto *tree = GetTree();
		if(tree->m_objectToStringCallback != nullptr)
			Con::ccl<<tree->m_objectToStringCallback(o);
		else
			Con::ccl<<&o;
		Con::ccl<<Con::endl;
	}
}
template<class T>
	OcclusionOctree<T> *OcclusionOctree<T>::Node::GetTree() {return static_cast<OcclusionOctree<T>*>(m_tree);}
template<class T>
	const OcclusionOctree<T> *OcclusionOctree<T>::Node::GetTree() const {return static_cast<OcclusionOctree<T>*>(m_tree);}

//////////////////////////////////////////////

template<class T>
	OcclusionOctree<T>::OcclusionOctree(float minNodeSize,float maxNodeSize,float initialBounds,const std::function<void(const T&,Vector3&,Vector3&)> &factory)
		: BaseOcclusionOctree(minNodeSize,maxNodeSize,initialBounds),m_objectBoundsCallback(factory)
{}

#pragma warning(push)
#pragma warning(disable : 4505) // Disable false positive warning in VS
template<class T>
	std::shared_ptr<BaseOcclusionOctree::Node> OcclusionOctree<T>::CreateNode(BaseOcclusionOctree::Node *parent)
{
	auto n = Node::Create(this,static_cast<Node*>(parent));
	InitializeNode(*n);
	return n;
}
#pragma warning(pop)

template<class T>
	void OcclusionOctree<T>::IterateObjects(const std::function<bool(const Node&)> fNodeCallback,const std::function<void(const T&)> &fObjCallback) const
{
	auto &root = GetRootNode();
	std::function<void(const Node&)> fIterateNode = nullptr;
	fIterateNode = [&fIterateNode,&fNodeCallback,&fObjCallback](const Node &node) {
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
			fIterateNode(*static_cast<Node*>(c.get()));
	};
	fIterateNode(root);
}

template<class T>
	void OcclusionOctree<T>::SetToStringCallback(const std::function<std::string(const T&)> &callback) {m_objectToStringCallback = callback;}

template<class T>
	typename OcclusionOctree<T>::Node &OcclusionOctree<T>::GetRootNode() {return static_cast<Node&>(*m_root.get());}

template<class T>
	const typename OcclusionOctree<T>::Node &OcclusionOctree<T>::GetRootNode() const {return static_cast<Node&>(*m_root.get());}

template<class T>
	bool OcclusionOctree<T>::ContainsObject(const T &o)
{
	auto it = m_objectNodes.find(o);
	return (it != m_objectNodes.end()) ? true : false;
}
	
template<class T>
	void OcclusionOctree<T>::InsertObject(const T &o,Node *optNode)
{
	auto it = m_objectNodes.find(o);
	if(it == m_objectNodes.end())
		it = m_objectNodes.insert(typename decltype(m_objectNodes)::value_type(o,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>>{})).first;
	else if(it->second.size() > 0)
		return; // Object already exists in tree
	
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
	m_dbgObjects.push_back(o);
#endif
	auto &nodes = it->second;
	Vector3 min,max;
	GetObjectBounds(o,min,max);
	if(
		optNode == nullptr &&
		(min.x == std::numeric_limits<float>::lowest() || min.y == std::numeric_limits<float>::lowest() || min.z == std::numeric_limits<float>::lowest() ||
		max.x == std::numeric_limits<float>::max() || max.y == std::numeric_limits<float>::max() || max.z == std::numeric_limits<float>::max())
	)
	{
		// Note: This case usually indicates some kind of error, objects inserted into the tree should never be this large.
		// We'll just force-insert it into the root node and don't grow the tree.
		m_objectNodes.erase(it);
		InsertObject(o,static_cast<Node*>(m_root.get()));
		return;
	}
	auto &root = optNode ? *optNode : GetRootNode();
	std::weak_ptr<BaseOcclusionOctree::Node> wp = root.shared_from_this();
	if(root.InsertObject(o,min,max,nodes,optNode ? true : false) == OcclusionOctreeInsertResult::ObjectOutOfBounds)
	{
		m_objectNodes.erase(it);
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
		m_dbgObjects.erase(m_dbgObjects.end() -1);
#endif
		if(optNode == nullptr && wp.expired() == false)
			root.InsertObjectReverse(o,min,max,nodes);
	}
}

template<class T>
	void OcclusionOctree<T>::InsertObject(const T &o)
{
	InsertObject(o,nullptr);
}

template<class T>
	void OcclusionOctree<T>::RemoveObject(const T &o)
{
	auto it = m_objectNodes.find(o);
	if(it == m_objectNodes.end())
		return;
	auto nodesCopy = it->second;
	for(auto &node : nodesCopy)
	{
		if(node.expired() == true)
			continue;
		static_cast<Node*>(node.lock().get())->RemoveObject(o);
	}

	// We need to check for the object again, because the map may have been changed by the above calls
	it = m_objectNodes.find(o);
	if(it != m_objectNodes.end())
	{
		m_objectNodes.erase(it);
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
		auto it = std::find_if(m_dbgObjects.begin(),m_dbgObjects.end(),[&o](const T &oRef) {
			return (o == oRef) ? true : false;
		});
		if(it != m_dbgObjects.end())
			m_dbgObjects.erase(it);
#endif
	}
}

template<class T>
	void OcclusionOctree<T>::UpdateObject(const T &o)
{
	auto it = m_objectNodes.find(o);
	if(it == m_objectNodes.end())
		return;
	if(IsSingleReferenceMode() == false)
	{
		Vector3 min,max;
		GetObjectBounds(o,min,max);
		auto nodesCopy = it->second;
		for(auto &node : nodesCopy)
		{
			if(node.expired() == true)
				continue;
			static_cast<Node*>(node.lock().get())->RemoveObject(o,false);
		}
		InsertObject(o);
		for(auto &node : nodesCopy)
		{
			if(node.expired() == true)
				continue;
			node.lock()->UpdateState();
		}
	}
	else
	{
		assert(it->second.size() == 1);
		if(it->second.empty() == true)
			return;
		auto &node = it->second.front();
		if(node.expired() == true)
			return;
		auto *n = static_cast<Node*>(node.lock().get());
		Vector3 min,max;
		GetObjectBounds(o,min,max);
		n->RemoveObject(o,false);
		it = m_objectNodes.find(o);
		if(it == m_objectNodes.end())
			it = m_objectNodes.insert(typename decltype(m_objectNodes)::value_type(o,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>>{})).first;
		// Attempt to re-insert object into node again
		std::weak_ptr<BaseOcclusionOctree::Node> wp = n->shared_from_this();
		if(n->InsertObject(o,min,max,it->second) == OcclusionOctreeInsertResult::ObjectOutOfBounds)
		{
			if(!wp.expired())
				n->InsertObjectReverse(o,min,max,it->second);
			if(!wp.expired())
				n->UpdateState(OcclusionOctreeUpdateMode::ForceUpdateParents);
		}
	}
}

template<class T>
	void OcclusionOctree<T>::InsertObjectAndExtendRoot(const T &o,const Vector3 &min,const Vector3 &max,std::vector<std::weak_ptr<BaseOcclusionOctree::Node>> &nodesInserted)
{
	static auto recursiveCount = std::numeric_limits<uint32_t>::max();
	const uint32_t maxRecursiveCount = 4;
	if(recursiveCount == std::numeric_limits<uint32_t>::max())
		recursiveCount = maxRecursiveCount;
	auto v = recursiveCount;
	auto &root = GetRootNode();
	if(recursiveCount == 0)
	{
		recursiveCount = std::numeric_limits<uint32_t>::max();
		InsertObject(o,&root); // Force object into root node
		// Con::cwar<<"Object "<<o<<" outside of occlusion tree bounds! Forcing in root node..."<<Con::endl;
		return;
	}
	--recursiveCount;
	std::vector<Vector3> entAabbPoints = {
		min,
		Vector3(min.x,min.y,max.z),
		Vector3(min.x,max.y,min.z),
		Vector3(max.x,min.y,min.z),
		Vector3(min.x,max.y,max.z),
		Vector3(max.x,min.y,min.z),
		Vector3(max.x,min.y,max.z),
		max
	};
	auto dFurthest = -1.f;
	Vector3 *pFurthest = nullptr;
	for(auto &p : entAabbPoints)
	{
		Vector3 r;
		auto &rootBounds = root.GetWorldBounds();
		umath::geometry::closest_point_on_aabb_to_point(rootBounds.first,rootBounds.second,p,&r);
		auto d = uvec::length_sqr(p -r);
		if(d > dFurthest)
		{
			dFurthest = d;
			pFurthest = &p;
		}
	}
	assert(pFurthest != nullptr);
	if(pFurthest == nullptr) // Can happen if object bounds are NaN or similar
	{
		Con::cwar<<"Object "<<o<<" has invalid bounds ("<<min.x<<","<<min.y<<","<<min.z<<") ("<<max.x<<","<<max.y<<","<<max.z<<")! Object will not be rendered!"<<Con::endl;
		return;
	}
	ExtendRoot(*pFurthest);
	InsertObject(o);

	recursiveCount = std::numeric_limits<uint32_t>::max();
	if(v == maxRecursiveCount -1)
		ShrinkRoot(); // Shrink root once all recursions are complete
}

template<class T>
	void OcclusionOctree<T>::GetObjectBounds(const T &o,Vector3 &min,Vector3 &max) const {m_objectBoundsCallback(o,min,max);}

template<class T>
	void OcclusionOctree<T>::RemoveNodeReference(const Node &node,const T &o)
{
	auto it = m_objectNodes.find(o);
	if(it == m_objectNodes.end())
		return;
	if(IsSingleReferenceMode() == true)
	{
		assert(it->second.size() <= 1);
		it->second.clear();
		return;
	}
	auto itNode = std::find_if(it->second.begin(),it->second.end(),[&node](const std::weak_ptr<BaseOcclusionOctree::Node> &nodeOther) {
		return (nodeOther.lock().get() == &node) ? true : false;
	});
	if(itNode == it->second.end())
		return;
	it->second.erase(itNode);
	if(it->second.empty())
		m_objectNodes.erase(it);
}

#pragma warning(push)
#pragma warning(disable : 4505) // Disable false positive warning in VS
template<class T>
	void OcclusionOctree<T>::DebugPrint() const
{
	BaseOcclusionOctree::DebugPrint();
#if ENABLE_OCCLUSION_DEBUG_MODE == 1
	const T *o = nullptr;
	auto validErr = Validate(&o);
	Con::cout<<"Validation Result: ";
	switch(validErr)
	{
		case ValidationError::Success:
			Con::cout<<"Success";
			break;
		case ValidationError::InvalidObjectReferenceCount:
			Con::cout<<"InvalidObjectReferenceCount";
			break;
		case ValidationError::InvalidObjectReferenceToNode:
			Con::cout<<"InvalidObjectReferenceToNode";
			break;
		case ValidationError::MissingObject:
			Con::cout<<"MissingObject";
			break;
		case ValidationError::DuplicateObject:
			Con::cout<<"DuplicateObject";
			break;
		default:
			Con::cout<<"Unknown";
			break;
	}
	if(o != nullptr)
	{
		Con::cout<<": ";
		if(m_objectToStringCallback == nullptr)
			Con::cout<<o;
		else
			Con::cout<<m_objectToStringCallback(*o);
	}
	Con::cout<<Con::endl;
#endif
}
#pragma warning(pop)

#if ENABLE_OCCLUSION_DEBUG_MODE == 1
template<class T>
	typename OcclusionOctree<T>::ValidationError OcclusionOctree<T>::Validate(const T **r) const
{
	if(m_objectNodes.size() != m_dbgObjects.size())
		return ValidationError::InvalidObjectReferenceCount;
	for(auto &pair : m_objectNodes)
	{
		for(auto &n : pair.second)
		{
			if(n.expired() == true)
				continue;
			auto node = n.lock();
			if(static_cast<Node*>(node.get())->HasObject(pair.first) == false)
			{
				if(r != nullptr)
					*r = &pair.first;
				return ValidationError::InvalidObjectReferenceToNode;
			}
		}
	}
	std::function<bool(const Node&,const T&,uint32_t&)> fIterateTree = nullptr;
	fIterateTree = [&fIterateTree](const Node &node,const T &o,uint32_t &count) -> bool {
		if(node.HasObject(o) == true)
		{
			++count;
			return true;
		}
		auto *children = node.GetChildren();
		if(children == nullptr)
			return false;
		for(auto &child : *children)
		{
			if(fIterateTree(*static_cast<Node*>(child.get()),o,count) == true)
				return true;
		}
		return false;
	};
	auto &node = GetRootNode();
	for(auto &o : m_dbgObjects)
	{
		uint32_t count = 0;
		if(fIterateTree(node,o,count) == false)
		{
			if(r != nullptr)
				*r = &o;
			return ValidationError::MissingObject;
		}
		else if(count > 1 && IsSingleReferenceMode() == true)
		{
			if(r != nullptr)
				*r = &o;
			return ValidationError::DuplicateObject;
		}
	}
	return ValidationError::Success;
}
#endif


#endif
