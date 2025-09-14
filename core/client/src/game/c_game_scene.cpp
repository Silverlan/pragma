// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/environment/c_env_camera.h"
#include "pragma/entities/components/c_scene_component.hpp"

extern CGame *c_game;

template<typename TCPPM>
TCPPM *CGame::GetScene() { return static_cast<pragma::CSceneComponent*>(m_scene.get()); }
template pragma::CSceneComponent* CGame::GetScene<pragma::CSceneComponent>();

template<typename TCPPM>
const TCPPM *CGame::GetScene() const { return const_cast<CGame *>(this)->GetScene<TCPPM>(); }
template const pragma::CSceneComponent* CGame::GetScene<pragma::CSceneComponent>() const;

static void cl_fov_callback(NetworkState *, const ConVar &, float, float val)
{
	if(c_game == nullptr)
		return;
	auto *cam = c_game->GetPrimaryCamera<pragma::CCameraComponent>();
	if(cam == nullptr)
		return;
	cam->SetFOV(CFloat(val));
	cam->UpdateMatrices();
}
REGISTER_CONVAR_CALLBACK_CL(cl_fov, cl_fov_callback)
REGISTER_CONVAR_CALLBACK_CL(cl_render_fov, cl_fov_callback)

static void cl_fov_viewmodel_callback(NetworkState *, const ConVar &, int, int val)
{
	if(c_game == nullptr)
		return;
	c_game->SetViewModelFOV(CFloat(val));
}
REGISTER_CONVAR_CALLBACK_CL(cl_fov_viewmodel, cl_fov_viewmodel_callback)
