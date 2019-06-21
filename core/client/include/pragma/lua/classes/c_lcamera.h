#ifndef __C_LCAMERA_H__
#define __C_LCAMERA_H__

#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include <mathutil/glmutil.h>

lua_registercheck(Scene,::Scene);

class WorldEnvironment;
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
	};
	namespace Camera
	{
		DLLCLIENT void Create(lua_State *l,float fov,float fovView,float aspectRatio,float nearZ,float farZ);
		DLLCLIENT void Copy(lua_State *l,::Camera &cam);
		DLLCLIENT void GetProjectionMatrix(lua_State *l,::Camera &cam);
		DLLCLIENT void GetViewMatrix(lua_State *l,::Camera &cam);
		DLLCLIENT void GetRight(lua_State *l,::Camera &cam);
		DLLCLIENT void GetUp(lua_State *l,::Camera &cam);
		DLLCLIENT void SetUp(lua_State *l,::Camera &cam,Vector3 &up);
		DLLCLIENT void GetPos(lua_State *l,::Camera &cam);
		DLLCLIENT void SetPos(lua_State *l,::Camera &cam,Vector3 &pos);
		DLLCLIENT void LookAt(lua_State *l,::Camera &cam,const Vector3 &pos);
		DLLCLIENT void UpdateMatrices(lua_State *l,::Camera &cam);
		DLLCLIENT void UpdateViewMatrix(lua_State *l,::Camera &cam);
		DLLCLIENT void UpdateProjectionMatrix(lua_State *l,::Camera &cam);
		DLLCLIENT void UpdateViewProjectionMatrix(lua_State *l,::Camera &cam);
		DLLCLIENT void UpdateProjectionMatrices(lua_State *l,::Camera &cam);
		DLLCLIENT void SetFOV(lua_State *l,::Camera &cam,float fov);
		DLLCLIENT void SetViewFOV(lua_State *l,::Camera &cam,float fov);
		DLLCLIENT void SetAspectRatio(lua_State *l,::Camera &cam,float aspectRatio);
		DLLCLIENT void SetZNear(lua_State *l,::Camera &cam,float nearZ);
		DLLCLIENT void SetZFar(lua_State *l,::Camera &cam,float farZ);
		DLLCLIENT void SetForward(lua_State *l,::Camera &cam,Vector3 &forward);
		DLLCLIENT void GetViewProjectionMatrix(lua_State *l,::Camera &cam);
		DLLCLIENT void GetForward(lua_State *l,::Camera &cam);
		DLLCLIENT void GetFOV(lua_State *l,::Camera &cam);
		DLLCLIENT void GetViewFOV(lua_State *l,::Camera &cam);
		DLLCLIENT void GetFOVRad(lua_State *l,::Camera &cam);
		DLLCLIENT void GetViewFOVRad(lua_State *l,::Camera &cam);
		DLLCLIENT void GetAspectRatio(lua_State *l,::Camera &cam);
		DLLCLIENT void GetZNear(lua_State *l,::Camera &cam);
		DLLCLIENT void GetZFar(lua_State *l,::Camera &cam);
		DLLCLIENT void GetFrustumPlanes(lua_State *l,::Camera &cam);
		DLLCLIENT void GetFarPlaneCenter(lua_State *l,::Camera &cam);
		DLLCLIENT void GetNearPlaneCenter(lua_State *l,::Camera &cam);
		DLLCLIENT void GetFarPlaneBoundaries(lua_State *l,::Camera &cam);
		DLLCLIENT void GetNearPlaneBoundaries(lua_State *l,::Camera &cam);
		DLLCLIENT void GetPlaneBoundaries(lua_State *l,::Camera &cam);
		DLLCLIENT void SetProjectionMatrix(lua_State *l,::Camera &cam,Mat4 &mat);
		DLLCLIENT void SetViewMatrix(lua_State *l,::Camera &cam,Mat4 &mat);
		DLLCLIENT void SetViewProjectionMatrix(lua_State *l,::Camera &cam,Mat4 &mat);
		DLLCLIENT void GetNearPlaneBounds(lua_State *l,::Camera &cam);
		DLLCLIENT void GetFarPlaneBounds(lua_State *l,::Camera &cam);
		DLLCLIENT void GetFrustumPoints(lua_State *l,::Camera &cam);
		DLLCLIENT void GetNearPlanePoint(lua_State *l,::Camera &cam,const Vector2 &uv);
		DLLCLIENT void GetFarPlanePoint(lua_State *l,::Camera &cam,const Vector2 &uv);

		DLLCLIENT void GetFrustumNeighbors(lua_State *l,::Camera &cam,int planeID);
		DLLCLIENT void GetFrustumPlaneCornerPoints(lua_State *l,::Camera &cam,int planeA,int planeB);
		DLLCLIENT void CreateFrustumKDop(lua_State *l,::Camera &cam,luabind::object o1,luabind::object o2,Vector3 dir);
		DLLCLIENT void CreateFrustumKDop(lua_State *l,::Camera &cam,const Vector2 &uvStart,const Vector2 &uvEnd);
		DLLCLIENT void GetRotation(lua_State *l,::Camera &cam);

		DLLCLIENT void CreateFrustumMesh(lua_State *l,::Camera &cam,const Vector2 &uvStart,const Vector2 &uvEnd);
	};
};

#endif
