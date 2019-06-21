#include "stdafx_client.h"
#include "pragma/lua/classes/c_lcamera.h"
#include "pragma/lua/classes/c_ldef_camera.h"
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include <pragma/math/plane.h>
#include "luasystem.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
#include "pragma/lua/classes/c_ldef_light.hpp"
#include "pragma/lua/classes/c_lshader.h"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <pragma/lua/lua_entity_component.hpp>
#include <pragma/lua/classes/ldef_entity.h>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCENGINE CEngine *c_engine;

void Lua::Scene::GetCamera(lua_State *l,::Scene &scene)
{
	auto &cam = scene.camera;
	Lua::Push<std::shared_ptr<::Camera>>(l,cam);
}

void Lua::Scene::GetSize(lua_State *l,::Scene &scene)
{
	Lua::Push<Vector2i>(l,Vector2i{scene.GetWidth(),scene.GetHeight()});
}
void Lua::Scene::GetWidth(lua_State *l,::Scene &scene)
{
	Lua::PushInt(l,scene.GetWidth());
}
void Lua::Scene::GetHeight(lua_State *l,::Scene &scene)
{
	Lua::PushInt(l,scene.GetHeight());
}
void Lua::Scene::Resize(lua_State*,::Scene &scene,uint32_t width,uint32_t height)
{
	scene.Resize(width,height);
}
void Lua::Scene::BeginDraw(lua_State *l,::Scene &scene)
{
	/*auto *renderer = scene.GetRenderer();
	if(renderer == nullptr)
		return;
	renderer->BeginRendering();*/
}
void Lua::Scene::UpdateBuffers(lua_State *l,::Scene &scene,prosper::CommandBuffer &hCommandBuffer)
{
	if(hCommandBuffer.IsPrimary() == false)
		return;
	auto pCmdBuffer = std::static_pointer_cast<prosper::PrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	scene.UpdateBuffers(pCmdBuffer);
}
void Lua::Scene::GetWorldEnvironment(lua_State *l,::Scene &scene)
{
	auto *worldEnv = scene.GetWorldEnvironment();
	if(worldEnv == nullptr)
		return;
	Lua::Push<std::shared_ptr<WorldEnvironment>>(l,worldEnv->shared_from_this());
}
void Lua::Scene::ClearWorldEnvironment(lua_State *l,::Scene &scene)
{
	scene.ClearWorldEnvironment();
}
void Lua::Scene::SetWorldEnvironment(lua_State *l,::Scene &scene,WorldEnvironment &worldEnv)
{
	scene.SetWorldEnvironment(worldEnv);
}
void Lua::Scene::InitializeRenderTarget(lua_State *l,::Scene &scene)
{
	scene.InitializeRenderTarget();
}
void Lua::Scene::AddLightSource(lua_State *l,::Scene &scene,CLightHandle &hLight)
{
	pragma::Lua::check_component(l,hLight);
	scene.AddLight(hLight.get());
}
void Lua::Scene::RemoveLightSource(lua_State *l,::Scene &scene,CLightHandle &hLight)
{
	pragma::Lua::check_component(l,hLight);
	scene.RemoveLight(hLight.get());
}
void Lua::Scene::SetLightSources(lua_State *l,::Scene &scene,luabind::object o)
{
	int32_t t = 2;
	Lua::CheckTable(l,t);
	auto numLights = Lua::GetObjectLength(l,t);
	std::vector<pragma::CLightComponent*> lights;
	lights.reserve(numLights);

	auto bHasDirectional = false;
	for(auto i=decltype(numLights){0};i<numLights;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,t);
		auto &pLight = Lua::Check<CLightHandle>(l,-1);
		pragma::Lua::check_component(l,pLight);
		Lua::Pop(l,1);
		auto pToggleComponent = pLight->GetEntity().GetComponent<pragma::CToggleComponent>();
		if(pToggleComponent.expired() || pToggleComponent->IsTurnedOn() == false)
			continue;
		auto type = LightType::Invalid;
		auto *pBaseLight = pLight->GetLight(type);
		if(pBaseLight == nullptr)
			continue;
		if(type == LightType::Directional)
		{
			if(bHasDirectional == true)
				continue;
			lights.insert(lights.begin(),pLight.get()); // Directional light source has to be at front
			bHasDirectional = true;
			continue;
		}
		lights.push_back(pLight.get());
	}
	scene.SetLights(lights);
}
void Lua::Scene::GetLightSources(lua_State *l,::Scene &scene)
{
	auto &lightSources = scene.GetLightSources();
	auto t = Lua::CreateTable(l);
	int32_t idx = 1;
	for(auto &hLight : lightSources)
	{
		if(hLight.expired())
			continue;
		Lua::PushInt(l,idx++);
		hLight->PushLuaObject(l);
		Lua::SetTableValue(l,t);
	}
}
void Lua::Scene::LinkLightSources(lua_State *l,::Scene &scene,::Scene &sceneOther) {scene.LinkLightSources(sceneOther);}
void Lua::Scene::AddEntity(lua_State *l,::Scene &scene,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	scene.AddEntity(static_cast<CBaseEntity&>(*hEnt.get()));
}
void Lua::Scene::RemoveEntity(lua_State *l,::Scene &scene,EntityHandle &hEnt)
{
	LUA_CHECK_ENTITY(l,hEnt);
	scene.RemoveEntity(static_cast<CBaseEntity&>(*hEnt.get()));
}
void Lua::Scene::SetEntities(lua_State *l,::Scene &scene,luabind::object o)
{
	int32_t t = 2;
	Lua::CheckTable(l,t);
	auto numEnts = Lua::GetObjectLength(l,t);
	std::vector<CBaseEntity*> ents;
	ents.reserve(numEnts);

	auto bHasDirectional = false;
	for(auto i=decltype(numEnts){0};i<numEnts;++i)
	{
		Lua::PushInt(l,i +1);
		Lua::GetTableValue(l,t);
		auto *ent = static_cast<CBaseEntity*>(Lua::CheckEntity(l,-1));
		Lua::Pop(l,1);
		if(ent->IsSpawned() == false)
			continue;
		ents.push_back(ent);
	}
	scene.SetEntities(ents);
}
void Lua::Scene::GetEntities(lua_State *l,::Scene &scene)
{
	auto &entityList = scene.GetEntities();
	auto t = Lua::CreateTable(l);
	int32_t idx = 1;
	for(auto &hEnt : entityList)
	{
		if(hEnt.IsValid() == false)
			continue;
		Lua::PushInt(l,idx++);
		Lua::Push<BaseEntity*>(l,hEnt.get());
		Lua::SetTableValue(l,t);
	}
}
void Lua::Scene::LinkEntities(lua_State *l,::Scene &scene,::Scene &sceneOther) {scene.LinkEntities(sceneOther);}
void Lua::Scene::GetCameraDescriptorSet(lua_State *l,::Scene &scene,uint32_t bindPoint)
{
	auto &descSet = scene.GetCameraDescriptorSetGroup(static_cast<vk::PipelineBindPoint>(bindPoint));
	if(descSet == nullptr)
		return;
	Lua::Push(l,descSet);
}
void Lua::Scene::GetCameraDescriptorSet(lua_State *l,::Scene &scene) {GetCameraDescriptorSet(l,scene,umath::to_integral(vk::PipelineBindPoint::eGraphics));}
void Lua::Scene::GetViewCameraDescriptorSet(lua_State *l,::Scene &scene)
{
	auto &descSet = scene.GetViewCameraDescriptorSetGroup();
	if(descSet == nullptr)
		return;
	Lua::Push(l,descSet);
}

////////////////////////////////

void Lua::RasterizationRenderer::GetPrepassDepthTexture(lua_State *l,pragma::rendering::RasterizationRenderer &renderer)
{
	auto &depthTex = renderer.GetPrepass().textureDepth;
	if(depthTex == nullptr)
		return;
	Lua::Push(l,depthTex);
}
void Lua::RasterizationRenderer::GetPrepassNormalTexture(lua_State *l,pragma::rendering::RasterizationRenderer &renderer)
{
	auto &normalTex = renderer.GetPrepass().textureNormals;
	if(normalTex == nullptr)
		return;
	Lua::Push(l,normalTex);
}
void Lua::RasterizationRenderer::GetRenderTarget(lua_State *l,pragma::rendering::RasterizationRenderer &renderer)
{
	auto &rt = renderer.GetHDRInfo().hdrRenderTarget;
	if(rt == nullptr)
		return;
	Lua::Push(l,rt);
}
void Lua::RasterizationRenderer::BeginRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,prosper::CommandBuffer &hCommandBuffer)
{
	if(hCommandBuffer->get_command_buffer_type() != Anvil::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto primCmdBuffer = std::static_pointer_cast<prosper::PrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	Lua::PushBool(l,renderer.BeginRenderPass(primCmdBuffer));
}
void Lua::RasterizationRenderer::BeginRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,prosper::CommandBuffer &hCommandBuffer,prosper::RenderPass &rp)
{
	if(hCommandBuffer->get_command_buffer_type() != Anvil::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto primCmdBuffer = std::static_pointer_cast<prosper::PrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	Lua::PushBool(l,renderer.BeginRenderPass(primCmdBuffer,&rp));
}
void Lua::RasterizationRenderer::EndRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,prosper::CommandBuffer &hCommandBuffer)
{
	if(hCommandBuffer->get_command_buffer_type() != Anvil::CommandBufferType::COMMAND_BUFFER_TYPE_PRIMARY)
	{
		Lua::PushBool(l,false);
		return;
	}
	auto primCmdBuffer = std::static_pointer_cast<prosper::PrimaryCommandBuffer>(hCommandBuffer.shared_from_this());
	Lua::PushBool(l,renderer.EndRenderPass(primCmdBuffer));
}
void Lua::RasterizationRenderer::GetPrepassShader(lua_State *l,pragma::rendering::RasterizationRenderer &renderer)
{
	auto &shader = renderer.GetPrepassShader();
	Lua::shader::push_shader(l,shader);
}
void Lua::RasterizationRenderer::SetShaderOverride(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const std::string &srcName,const std::string &dstName) {renderer.SetShaderOverride(srcName,dstName);}
void Lua::RasterizationRenderer::ClearShaderOverride(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const std::string &srcName) {renderer.ClearShaderOverride(srcName);}
void Lua::RasterizationRenderer::SetPrepassMode(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t mode) {renderer.SetPrepassMode(static_cast<pragma::rendering::RasterizationRenderer::PrepassMode>(mode));}
void Lua::RasterizationRenderer::GetPrepassMode(lua_State *l,pragma::rendering::RasterizationRenderer &renderer) {Lua::PushInt(l,umath::to_integral(renderer.GetPrepassMode()));}

////////////////////////////////

void Lua::Camera::Create(lua_State *l,float fov,float fovView,float aspectRatio,float nearZ,float farZ)
{
	Lua::Push<std::shared_ptr<::Camera>>(l,::Camera::Create(fov,fovView,aspectRatio,nearZ,farZ));
}
void Lua::Camera::Copy(lua_State *l,::Camera &cam)
{
	Lua::Push<std::shared_ptr<::Camera>>(l,::Camera::Create(cam));
}

void Lua::Camera::GetProjectionMatrix(lua_State *l,::Camera &cam)
{
	luabind::object(l,cam.GetProjectionMatrix()).push(l);
}

void Lua::Camera::GetViewMatrix(lua_State *l,::Camera &cam)
{
	luabind::object(l,cam.GetViewMatrix()).push(l);
}

void Lua::Camera::GetRight(lua_State *l,::Camera &cam)
{
	luabind::object(l,cam.GetRight()).push(l);
}

void Lua::Camera::GetUp(lua_State *l,::Camera &cam)
{
	luabind::object(l,cam.GetUp()).push(l);
}

void Lua::Camera::SetUp(lua_State*,::Camera &cam,Vector3 &up)
{
	cam.SetUp(up);
}

void Lua::Camera::GetPos(lua_State *l,::Camera &cam)
{
	luabind::object(l,cam.GetPos()).push(l);
}

void Lua::Camera::SetPos(lua_State*,::Camera &cam,Vector3 &pos)
{
	cam.SetPos(pos);
}

void Lua::Camera::LookAt(lua_State*,::Camera &cam,const Vector3 &pos)
{
	auto &camPos = cam.GetPos();
	auto dir = pos -camPos;
	uvec::normalize(&dir);
	cam.SetForward(dir);
}

void Lua::Camera::UpdateMatrices(lua_State*,::Camera &cam)
{
	cam.UpdateMatrices();
}
void Lua::Camera::UpdateViewMatrix(lua_State*,::Camera &cam)
{
	cam.UpdateViewMatrix();
}
void Lua::Camera::UpdateProjectionMatrix(lua_State*,::Camera &cam)
{
	cam.UpdateProjectionMatrix();
}
void Lua::Camera::UpdateViewProjectionMatrix(lua_State*,::Camera &cam)
{
	cam.UpdateViewProjectionMatrix();
}
void Lua::Camera::UpdateProjectionMatrices(lua_State*,::Camera &cam)
{
	cam.UpdateProjectionMatrices();
}
void Lua::Camera::SetFOV(lua_State*,::Camera &cam,float fov)
{
	cam.SetFOV(fov);
}
void Lua::Camera::SetViewFOV(lua_State*,::Camera &cam,float fov)
{
	cam.SetViewFOV(fov);
}
void Lua::Camera::SetAspectRatio(lua_State*,::Camera &cam,float aspectRatio)
{
	cam.SetAspectRatio(aspectRatio);
}
void Lua::Camera::SetZNear(lua_State*,::Camera &cam,float nearZ)
{
	cam.SetZNear(nearZ);
}
void Lua::Camera::SetZFar(lua_State*,::Camera &cam,float farZ)
{
	cam.SetZFar(farZ);
}
void Lua::Camera::SetForward(lua_State*,::Camera &cam,Vector3 &forward)
{
	cam.SetForward(forward);
}
void Lua::Camera::GetViewProjectionMatrix(lua_State *l,::Camera &cam)
{
	Lua::Push<Mat4>(l,cam.GetViewProjectionMatrix());
}
void Lua::Camera::GetForward(lua_State *l,::Camera &cam)
{
	Lua::Push<Vector3>(l,cam.GetForward());
}
void Lua::Camera::GetFOV(lua_State *l,::Camera &cam)
{
	Lua::PushNumber(l,cam.GetFOV());
}
void Lua::Camera::GetViewFOV(lua_State *l,::Camera &cam)
{
	Lua::PushNumber(l,cam.GetViewFOV());
}
void Lua::Camera::GetFOVRad(lua_State *l,::Camera &cam)
{
	Lua::PushNumber(l,cam.GetFOVRad());
}
void Lua::Camera::GetViewFOVRad(lua_State *l,::Camera &cam)
{
	Lua::PushNumber(l,cam.GetViewFOVRad());
}
void Lua::Camera::GetAspectRatio(lua_State *l,::Camera &cam)
{
	Lua::PushNumber(l,cam.GetAspectRatio());
}
void Lua::Camera::GetZNear(lua_State *l,::Camera &cam)
{
	Lua::PushNumber(l,cam.GetZNear());
}
void Lua::Camera::GetZFar(lua_State *l,::Camera &cam)
{
	Lua::PushNumber(l,cam.GetZFar());
}
void Lua::Camera::GetFrustumPlanes(lua_State *l,::Camera &cam)
{
	std::vector<Plane> planes;
	cam.GetFrustumPlanes(planes);

	lua_newtable(l);
	int top = lua_gettop(l);
	for(unsigned int i=0;i<planes.size();i++)
	{
		Lua::Push<Plane>(l,planes[i]);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::Camera::GetFarPlaneCenter(lua_State *l,::Camera &cam)
{
	Lua::Push<Vector3>(l,cam.GetFarPlaneCenter());
}
void Lua::Camera::GetNearPlaneCenter(lua_State *l,::Camera &cam)
{
	Lua::Push<Vector3>(l,cam.GetNearPlaneCenter());
}
void Lua::Camera::GetFarPlaneBoundaries(lua_State *l,::Camera &cam)
{
	std::vector<Vector3> farBounds;
	cam.GetFarPlaneBoundaries(&farBounds);

	lua_newtable(l);
	int top = lua_gettop(l);
	for(unsigned int i=0;i<farBounds.size();i++)
	{
		Lua::Push<Vector3>(l,farBounds[i]);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::Camera::GetNearPlaneBoundaries(lua_State *l,::Camera &cam)
{
	std::vector<Vector3> nearBounds;
	cam.GetNearPlaneBoundaries(&nearBounds);

	lua_newtable(l);
	int top = lua_gettop(l);
	for(unsigned int i=0;i<nearBounds.size();i++)
	{
		Lua::Push<Vector3>(l,nearBounds[i]);
		lua_rawseti(l,top,i +1);
	}
}
void Lua::Camera::GetPlaneBoundaries(lua_State *l,::Camera &cam)
{
	std::vector<Vector3> nearBounds;
	std::vector<Vector3> farBounds;
	cam.GetPlaneBoundaries(&nearBounds,&farBounds);

	lua_newtable(l);
	int top = lua_gettop(l);
	for(unsigned int i=0;i<nearBounds.size();i++)
	{
		Lua::Push<Vector3>(l,nearBounds[i]);
		lua_rawseti(l,top,i +1);
	}

	lua_newtable(l);
	top = lua_gettop(l);
	for(unsigned int i=0;i<farBounds.size();i++)
	{
		Lua::Push<Vector3>(l,farBounds[i]);
		lua_rawseti(l,top,i +1);
	}
}

void Lua::Camera::SetProjectionMatrix(lua_State*,::Camera &cam,Mat4 &mat)
{
	cam.SetProjectionMatrix(mat);
}

void Lua::Camera::SetViewMatrix(lua_State*,::Camera &cam,Mat4 &mat)
{
	cam.SetViewMatrix(mat);
}
void Lua::Camera::SetViewProjectionMatrix(lua_State*,::Camera &cam,Mat4 &mat)
{
	cam.SetViewProjectionMatrix(mat);
}

void Lua::Camera::GetNearPlaneBounds(lua_State *l,::Camera &cam)
{
	float wNear,hNear;
	cam.GetNearPlaneBounds(&wNear,&hNear);
	Lua::PushNumber(l,wNear);
	Lua::PushNumber(l,hNear);
}

void Lua::Camera::GetFarPlaneBounds(lua_State *l,::Camera &cam)
{
	float wFar,hFar;
	cam.GetFarPlaneBounds(&wFar,&hFar);
	Lua::PushNumber(l,wFar);
	Lua::PushNumber(l,hFar);
}

void Lua::Camera::GetFrustumPoints(lua_State *l,::Camera &cam)
{
	std::vector<Vector3> points;
	cam.GetFrustumPoints(points);

	int table = Lua::CreateTable(l);
	for(unsigned int i=0;i<points.size();i++)
	{
		Lua::PushInt(l,i +1);
		Lua::Push<Vector3>(l,points[i]);
		Lua::SetTableValue(l,table);
	}
}

void Lua::Camera::GetNearPlanePoint(lua_State *l,::Camera &cam,const Vector2 &uv)
{
	auto point = cam.GetNearPlanePoint(uv);
	Lua::Push<decltype(point)>(l,point);
}
void Lua::Camera::GetFarPlanePoint(lua_State *l,::Camera &cam,const Vector2 &uv)
{
	auto point = cam.GetFarPlanePoint(uv);
	Lua::Push<decltype(point)>(l,point);
}

void Lua::Camera::GetFrustumNeighbors(lua_State *l,::Camera &cam,int planeID)
{
	if(planeID < 0 || planeID > 5)
		return;
	FRUSTUM_PLANE neighborIDs[4];
	cam.GetFrustumNeighbors(FRUSTUM_PLANE(planeID),&neighborIDs[0]);
	int table = Lua::CreateTable(l);
	for(unsigned int i=0;i<4;i++)
	{
		Lua::PushInt(l,i +1);
		Lua::PushInt(l,static_cast<int>(neighborIDs[i]));
		Lua::SetTableValue(l,table);
	}
}

void Lua::Camera::GetFrustumPlaneCornerPoints(lua_State *l,::Camera &cam,int planeA,int planeB)
{
	Lua::CheckTable(l,2);

	if(planeA < 0 || planeB < 0 || planeA > 5 || planeB > 5)
		return;

	FRUSTUM_POINT cornerPoints[2];
	cam.GetFrustumPlaneCornerPoints(FRUSTUM_PLANE(planeA),FRUSTUM_PLANE(planeB),&cornerPoints[0]);

	Lua::PushInt(l,static_cast<int>(cornerPoints[0]));
	Lua::PushInt(l,static_cast<int>(cornerPoints[1]));
}

void Lua::Camera::CreateFrustumKDop(lua_State *l,::Camera &cam,const Vector2 &uvStart,const Vector2 &uvEnd)
{
	std::vector<Plane> kDop;
	cam.CreateFrustumKDop(uvStart,uvEnd,kDop);

	auto table = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto &plane : kDop)
	{
		Lua::PushInt(l,idx++);
		Lua::Push<Plane>(l,plane);
		Lua::SetTableValue(l,table);
	}
}

void Lua::Camera::CreateFrustumKDop(lua_State *l,::Camera &cam,luabind::object o1,luabind::object o2,Vector3 dir)
{
	Lua::CheckTable(l,2);
	Lua::CheckTable(l,3);

	std::vector<Plane> planes;
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,2) != 0)
	{
		Plane *plane = Lua::CheckPlane(l,-1);
		planes.push_back(*plane);
		Lua::Pop(l,1);
	}
	if(planes.empty())
		return;

	std::vector<Vector3> points;
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,3) != 0)
	{
		Vector3 *v = Lua::CheckVector(l,-1);
		points.push_back(*v);
		Lua::Pop(l,1);
	}
	if(points.empty())
		return;

	std::vector<Plane> kDop;
	cam.CreateFrustumKDop(planes,points,dir,&kDop);

	int table = Lua::CreateTable(l);
	for(unsigned int i=0;i<kDop.size();i++)
	{
		Lua::PushInt(l,i +1);
		Lua::Push<Plane>(l,kDop[i]);
		Lua::SetTableValue(l,table);
	}
}
void Lua::Camera::GetRotation(lua_State *l,::Camera &cam)
{
	Lua::Push<Quat>(l,cam.GetRotation());
}

void Lua::Camera::CreateFrustumMesh(lua_State *l,::Camera &cam,const Vector2 &uvStart,const Vector2 &uvEnd)
{
	std::vector<Vector3> verts;
	std::vector<uint16_t> indices;
	cam.CreateFrustumMesh(uvStart,uvEnd,verts,indices);
	auto t = Lua::CreateTable(l);
	auto vertIdx = 1u;
	for(auto &v : verts)
	{
		Lua::PushInt(l,vertIdx++);
		Lua::Push<Vector3>(l,v);
		Lua::SetTableValue(l,t);
	}

	t = Lua::CreateTable(l);
	auto idx = 1u;
	for(auto i : indices)
	{
		Lua::PushInt(l,idx++);
		Lua::PushInt(l,i);
		Lua::SetTableValue(l,t);
	}
}
