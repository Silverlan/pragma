// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :entities.components.point_render_target;
import :client_state;
import :engine;

using namespace pragma;

#pragma message("FIXME: If point_rendertarget is out of view of the local player, but one of the texture targets isn't, they won't get updated! Find a solution!")

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

void CRenderTargetComponent::SetRenderMaterial(material::Material *mat) { m_matRender = mat; }
material::Material *CRenderTargetComponent::GetRenderMaterial() { return m_matRender; }
void CRenderTargetComponent::SetRenderMaterial(std::string mat) { SetRenderMaterial(get_client_state()->LoadMaterial(mat.c_str())); }

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

void CRenderTargetComponent::Render(rendering::SceneRenderPass renderMode)
{
	// TODO
}
void CRenderTargetComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }

void CPointRenderTarget::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CRenderTargetComponent>();
}
