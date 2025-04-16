/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/point/c_point_rendertarget.h"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/entities/components/c_scene_component.hpp"
#include <cmaterialmanager.h>
#include <mathutil/umat.h>
#include "pragma/entities/baseentity_luaobject.h"
#include "pragma/c_engine.h"
#include <texturemanager/texturemanager.h>
#include "textureinfo.h"
#include <pragma/networking/nwm_util.h>
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

#pragma message("FIXME: If point_rendertarget is out of view of the local player, but one of the texture targets isn't, they won't get updated! Find a solution!")

// LINK_ENTITY_TO_CLASS(point_rendertarget,CPointRenderTarget);

void CRenderTargetComponent::ReceiveData(NetPacket &packet)
{
	m_kvMaterial = packet->ReadString();
	m_kvFOV = packet->Read<float>();
	m_kvRefreshRate = packet->Read<float>();
	m_kvRenderWidth = packet->Read<float>();
	m_kvRenderHeight = packet->Read<float>();
	m_kvNearZ = packet->Read<float>();
	m_kvFarZ = packet->Read<float>();
	m_kvRenderDepth = packet->Read<int>();
}

void CRenderTargetComponent::SetRenderSize(Vector2 &size) { SetRenderSize(size.x, size.y); }
void CRenderTargetComponent::SetRenderSize(float w, float h)
{
	m_kvRenderWidth = w;
	m_kvRenderHeight = h;
}
Vector2 CRenderTargetComponent::GetRenderSize() { return Vector2(m_kvRenderWidth, m_kvRenderHeight); }
void CRenderTargetComponent::GetRenderSize(float *w, float *h)
{
	*w = m_kvRenderWidth;
	*h = m_kvRenderHeight;
}

void CRenderTargetComponent::SetRenderMaterial(Material *mat) { m_matRender = mat; }
Material *CRenderTargetComponent::GetRenderMaterial() { return m_matRender; }
void CRenderTargetComponent::SetRenderMaterial(std::string mat) { SetRenderMaterial(client->LoadMaterial(mat.c_str())); }

void CRenderTargetComponent::SetRefreshRate(float f) { m_kvRefreshRate = f; }
float CRenderTargetComponent::GetRefreshRate() { return m_kvRefreshRate; }

void CRenderTargetComponent::SetRenderFOV(float fov) { m_kvFOV = fov; }
float CRenderTargetComponent::GetRenderFOV() { return m_kvFOV; }

void CRenderTargetComponent::SetRenderDepth(unsigned int depth) { m_kvRenderDepth = depth; }
unsigned int CRenderTargetComponent::GetRenderDepth() { return m_kvRenderDepth; }

void CRenderTargetComponent::Spawn()
{
	// TODO
}

unsigned int CRenderTargetComponent::GetTextureBuffer() { return 0; } // Obsolete

void CRenderTargetComponent::Render(pragma::rendering::SceneRenderPass renderMode)
{
	// TODO
}
void CRenderTargetComponent::InitializeLuaObject(lua_State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointRenderTarget::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CRenderTargetComponent>();
}
