// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :game;

template<typename TCPPM>
TCPPM *pragma::CGame::GetScene()
{
	return static_cast<CSceneComponent *>(m_scene.get());
}
template DLLCLIENT pragma::CSceneComponent *pragma::CGame::GetScene<pragma::CSceneComponent>();
template DLLCLIENT const pragma::CSceneComponent *pragma::CGame::GetScene<const pragma::CSceneComponent>();

template<typename TCPPM>
const TCPPM *pragma::CGame::GetScene() const
{
	return const_cast<CGame *>(this)->GetScene<TCPPM>();
}
template DLLCLIENT const pragma::CSceneComponent *pragma::CGame::GetScene<pragma::CSceneComponent>() const;
template DLLCLIENT const pragma::CSceneComponent *pragma::CGame::GetScene<const pragma::CSceneComponent>() const;

static void cl_fov_callback(pragma::NetworkState *, const pragma::console::ConVar &, float, float val)
{
	if(pragma::get_cgame() == nullptr)
		return;
	auto *cam = pragma::get_cgame()->GetPrimaryCamera<pragma::CCameraComponent>();
	if(cam == nullptr)
		return;
	cam->SetFOV(CFloat(val));
	cam->UpdateMatrices();
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>("cl_fov", &cl_fov_callback);
}
namespace {
	auto UVN = pragma::console::client::register_variable_listener<float>("cl_render_fov", &cl_fov_callback);
}

static void cl_fov_viewmodel_callback(pragma::NetworkState *, const pragma::console::ConVar &, int, int val)
{
	if(pragma::get_cgame() == nullptr)
		return;
	pragma::get_cgame()->SetViewModelFOV(CFloat(val));
}

namespace {
	auto UVN = pragma::console::client::register_variable_listener<int>("cl_fov_viewmodel", &cl_fov_viewmodel_callback);
}
