/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include <prosper_command_buffer.hpp>

void Lua::BSP::register_class(lua_State *l,luabind::module_ &entsMod,luabind::class_<CWorldHandle,BaseEntityComponentHandle> &defWorld)
{
	auto defBspTree = luabind::class_<::util::BSPTree>("BSPTree");
	defBspTree.def("IsValid",static_cast<void(*)(lua_State*,::util::BSPTree&)>([](lua_State *l,::util::BSPTree &tree) {
		Lua::PushBool(l,tree.IsValid());
		}));
	defBspTree.def("IsClusterVisible",static_cast<void(*)(lua_State*,::util::BSPTree&,uint32_t,uint32_t)>([](lua_State *l,::util::BSPTree &tree,uint32_t clusterSrc,uint32_t clusterDst) {
		Lua::PushBool(l,tree.IsClusterVisible(clusterSrc,clusterDst));
		}));
	defBspTree.def("GetRootNode",static_cast<void(*)(lua_State*,::util::BSPTree&)>([](lua_State *l,::util::BSPTree &tree) {
		auto &node = tree.GetRootNode();
		Lua::Push(l,node.shared_from_this());
		}));
	defBspTree.def("GetNodes",static_cast<void(*)(lua_State*,::util::BSPTree&)>([](lua_State *l,::util::BSPTree &tree) {
		auto &nodes = tree.GetNodes();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &node : nodes)
		{
			Lua::PushInt(l,idx++);
			Lua::Push(l,node);
			Lua::SetTableValue(l,t);
		}
		}));
	defBspTree.def("GetClusterVisibility",static_cast<void(*)(lua_State*,::util::BSPTree&)>([](lua_State *l,::util::BSPTree &tree) {
		auto &clusterVisibility = tree.GetClusterVisibility();
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto vis : clusterVisibility)
		{
			Lua::PushInt(l,idx++);
			Lua::PushInt(l,vis);
			Lua::SetTableValue(l,t);
		}
		}));
	defBspTree.def("GetClusterCount",static_cast<void(*)(lua_State*,::util::BSPTree&)>([](lua_State *l,::util::BSPTree &tree) {
		Lua::PushInt(l,tree.GetClusterCount());
		}));
	defBspTree.def("FindLeafNode",static_cast<void(*)(lua_State*,::util::BSPTree&,const Vector3&)>([](lua_State *l,::util::BSPTree &tree,const Vector3 &origin) {
		auto *node = tree.FindLeafNode(origin);
		if(node == nullptr)
			return;
		Lua::Push(l,node->shared_from_this());
		}));
	defBspTree.def("FindLeafNodesInAABB",static_cast<luabind::object(*)(lua_State*,::util::BSPTree&,const Vector3&,const Vector3&)>([](lua_State *l,::util::BSPTree &tree,const Vector3 &min,const Vector3 &max) -> luabind::object {
		auto nodes = tree.FindLeafNodesInAABB(min,max);
		auto t = luabind::newtable(l);
		int32_t idx = 1;
		for(auto &n : nodes)
			t[idx++] = n;
		return t;
	}));

	auto defBspNode = luabind::class_<::util::BSPTree::Node>("Node");
	defBspNode.def("IsLeaf",static_cast<void(*)(lua_State*,::util::BSPTree::Node&)>([](lua_State *l,::util::BSPTree::Node &node) {
		Lua::PushBool(l,node.leaf);
		}));
	defBspNode.def("GetBounds",static_cast<void(*)(lua_State*,::util::BSPTree::Node&)>([](lua_State *l,::util::BSPTree::Node &node) {
		Lua::Push<Vector3>(l,node.min);
		Lua::Push<Vector3>(l,node.max);
		}));
	defBspNode.def("GetChildren",static_cast<void(*)(lua_State*,::util::BSPTree::Node&)>([](lua_State *l,::util::BSPTree::Node &node) {
		auto t = Lua::CreateTable(l);
		auto idx = 1;
		for(auto &child : node.children)
		{
			Lua::PushInt(l,idx++);
			Lua::Push(l,child);
		}
		}));
	defBspNode.def("GetCluster",static_cast<void(*)(lua_State*,::util::BSPTree::Node&)>([](lua_State *l,::util::BSPTree::Node &node) {
		Lua::PushInt(l,node.cluster);
		}));
	defBspNode.def("GetVisibleLeafAreaBounds",static_cast<void(*)(lua_State*,::util::BSPTree::Node&)>([](lua_State *l,::util::BSPTree::Node &node) {
		Lua::Push<Vector3>(l,node.minVisible);
		Lua::Push<Vector3>(l,node.maxVisible);
		}));
	defBspNode.def("GetInternalNodePlane",static_cast<void(*)(lua_State*,::util::BSPTree::Node&)>([](lua_State *l,::util::BSPTree::Node &node) {
		Lua::Push<Plane>(l,node.plane);
		}));
	defBspNode.def("GetInternalNodeFirstFaceIndex",static_cast<void(*)(lua_State*,::util::BSPTree::Node&)>([](lua_State *l,::util::BSPTree::Node &node) {
		Lua::PushInt(l,node.firstFace);
		}));
	defBspNode.def("GetInternalNodeFaceCount",static_cast<void(*)(lua_State*,::util::BSPTree::Node&)>([](lua_State *l,::util::BSPTree::Node &node) {
		Lua::PushInt(l,node.numFaces);
		}));
	defBspTree.scope[defBspNode];
	defWorld.scope[defBspTree];
}
