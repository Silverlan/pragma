#include "stdafx_client.h"
#include "pragma/lua/classes/c_lcamera.h"
#include <pragma/lua/classes/ldef_vector.h>
#include "pragma/lua/classes/ldef_plane.h"
#include <pragma/math/plane.h>
#include "luasystem.h"
#include "pragma/rendering/world_environment.hpp"
#include "pragma/lua/libraries/c_lua_vulkan.h"
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
	auto &cam = scene.GetActiveCamera();
	if(cam.expired())
		return;
	cam->PushLuaObject(l);
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
	auto &rt = renderer.GetHDRInfo().sceneRenderTarget;
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
