#ifndef __C_LCAMERA_H__
#define __C_LCAMERA_H__

#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include <mathutil/glmutil.h>

lua_registercheck(Scene,::Scene);

class WorldEnvironment;
class Material;
namespace Lua
{
	namespace Scene
	{
		DLLCLIENT void GetCamera(lua_State *l,::Scene &scene);
		DLLCLIENT void GetSize(lua_State *l,::Scene &scene);
		DLLCLIENT void GetWidth(lua_State *l,::Scene &scene);
		DLLCLIENT void GetHeight(lua_State *l,::Scene &scene);
		DLLCLIENT void Resize(lua_State *l,::Scene &scene,uint32_t width,uint32_t height);

		DLLCLIENT void BeginDraw(lua_State *l,::Scene &scene);
		DLLCLIENT void UpdateBuffers(lua_State *l,::Scene &scene,prosper::CommandBuffer &hCommandBuffer);
		DLLCLIENT void GetWorldEnvironment(lua_State *l,::Scene &scene);
		DLLCLIENT void ClearWorldEnvironment(lua_State *l,::Scene &scene);
		DLLCLIENT void SetWorldEnvironment(lua_State *l,::Scene &scene,WorldEnvironment &worldEnv);

		DLLCLIENT void InitializeRenderTarget(lua_State *l,::Scene &scene);

		DLLCLIENT void AddLightSource(lua_State *l,::Scene &scene,CLightHandle &hLight);
		DLLCLIENT void RemoveLightSource(lua_State *l,::Scene &scene,CLightHandle &hLight);
		DLLCLIENT void SetLightSources(lua_State *l,::Scene &scene,luabind::object o);
		DLLCLIENT void GetLightSources(lua_State *l,::Scene &scene);
		DLLCLIENT void LinkLightSources(lua_State *l,::Scene &scene,::Scene &sceneOther);

		DLLCLIENT void AddEntity(lua_State *l,::Scene &scene,EntityHandle &hEnt);
		DLLCLIENT void RemoveEntity(lua_State *l,::Scene &scene,EntityHandle &hEnt);
		DLLCLIENT void SetEntities(lua_State *l,::Scene &scene,luabind::object o);
		DLLCLIENT void GetEntities(lua_State *l,::Scene &scene);
		DLLCLIENT void LinkEntities(lua_State *l,::Scene &scene,::Scene &sceneOther);

		DLLCLIENT void GetCameraDescriptorSet(lua_State *l,::Scene &scene);
		DLLCLIENT void GetCameraDescriptorSet(lua_State *l,::Scene &scene,uint32_t bindPoint);
		DLLCLIENT void GetViewCameraDescriptorSet(lua_State *l,::Scene &scene);
	};
	namespace RasterizationRenderer
	{
		DLLCLIENT void GetPrepassDepthTexture(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);
		DLLCLIENT void GetPrepassNormalTexture(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);

		DLLCLIENT void GetRenderTarget(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);
		DLLCLIENT void BeginRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,prosper::CommandBuffer &hCommandBuffer);
		DLLCLIENT void BeginRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,prosper::CommandBuffer &hCommandBuffer,prosper::RenderPass &rp);
		DLLCLIENT void EndRenderPass(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,prosper::CommandBuffer &hCommandBuffer);
		DLLCLIENT void GetPrepassShader(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);

		DLLCLIENT void SetShaderOverride(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const std::string &srcName,const std::string &dstName);
		DLLCLIENT void ClearShaderOverride(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,const std::string &srcName);

		DLLCLIENT void SetPrepassMode(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t mode);
		DLLCLIENT void GetPrepassMode(lua_State *l,pragma::rendering::RasterizationRenderer &renderer);

		DLLCLIENT void ScheduleMeshForRendering(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t renderMode,pragma::ShaderTextured3DBase &shader,::Material &mat,EntityHandle &hEnt,ModelSubMesh &mesh);
		DLLCLIENT void ScheduleMeshForRendering(lua_State *l,pragma::rendering::RasterizationRenderer &renderer,uint32_t renderMode,const std::string &shaderName,::Material &mat,EntityHandle &hEnt,ModelSubMesh &mesh);
	};
};

#endif
