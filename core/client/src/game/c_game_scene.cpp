#include "stdafx_client.h"
#include "pragma/game/c_game.h"

extern DLLCLIENT CGame *c_game;

std::shared_ptr<Scene> &CGame::GetScene() {return m_scene;}
Camera &CGame::GetSceneCamera() const {return *m_scene->camera.get();}

REGISTER_CONVAR_CALLBACK_CL(cl_fov,[](NetworkState*,ConVar*,int,int val) {
	if(c_game == nullptr)
		return;
	auto &cam = c_game->GetSceneCamera();
	cam.SetFOV(CFloat(val));
	cam.UpdateMatrices();
});

REGISTER_CONVAR_CALLBACK_CL(cl_fov_viewmodel,[](NetworkState*,ConVar*,int,int val) {
	if(c_game == nullptr)
		return;
	auto &cam = c_game->GetSceneCamera();
	cam.SetViewFOV(CFloat(val));
	cam.UpdateMatrices();
});

REGISTER_CONVAR_CALLBACK_CL(cl_render_fov,[](NetworkState*,ConVar*,float,float fov) {
	if(c_game == nullptr)
		return;
	auto &cam = c_game->GetSceneCamera();
	cam.SetFOV(fov);
	cam.UpdateMatrices();
})
