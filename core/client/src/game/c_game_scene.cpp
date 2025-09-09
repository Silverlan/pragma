// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/environment/c_env_camera.h"

extern CGame *c_game;

pragma::CSceneComponent *CGame::GetScene() { return m_scene.get(); }
const pragma::CSceneComponent *CGame::GetScene() const { return const_cast<CGame *>(this)->GetScene(); }

static void cl_fov_callback(NetworkState *, const ConVar &, float, float val)
{
	if(c_game == nullptr)
		return;
	auto *cam = c_game->GetPrimaryCamera();
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
