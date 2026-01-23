// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.server;
import :entities.components.point.path_node;

using namespace pragma;

void SPathNodeComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void PointPathNode::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<SPathNodeComponent>();
}
