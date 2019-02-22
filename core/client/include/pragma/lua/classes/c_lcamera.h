#ifndef __C_LCAMERA_H__
#define __C_LCAMERA_H__

#include "pragma/clientdefinitions.h"
#include "pragma/c_enginedefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include <mathutil/glmutil.h>

lua_registercheck(Scene,std::shared_ptr<::Scene>);

class WorldEnvironment;
namespace Lua
{
	namespace Scene
	{
		DLLCLIENT void GetCamera(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void GetSize(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void GetWidth(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void GetHeight(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void Resize(lua_State *l,std::shared_ptr<::Scene> &scene,uint32_t width,uint32_t height);

		DLLCLIENT void BeginDraw(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void UpdateBuffers(lua_State *l,std::shared_ptr<::Scene> &scene,std::shared_ptr<prosper::CommandBuffer> &hCommandBuffer);
		DLLCLIENT void GetWorldEnvironment(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void ClearWorldEnvironment(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void SetWorldEnvironment(lua_State *l,std::shared_ptr<::Scene> &scene,std::shared_ptr<WorldEnvironment> &worldEnv);
		DLLCLIENT void GetPrepassDepthTexture(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void GetPrepassNormalTexture(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void GetRenderTarget(lua_State *l,std::shared_ptr<::Scene> &scene);

		DLLCLIENT void InitializeRenderTarget(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void BeginRenderPass(lua_State *l,std::shared_ptr<::Scene> &scene,std::shared_ptr<prosper::CommandBuffer> &hCommandBuffer);
		DLLCLIENT void BeginRenderPass(lua_State *l,std::shared_ptr<::Scene> &scene,std::shared_ptr<prosper::CommandBuffer> &hCommandBuffer,std::shared_ptr<prosper::RenderPass> &rp);
		DLLCLIENT void EndRenderPass(lua_State *l,std::shared_ptr<::Scene> &scene,std::shared_ptr<prosper::CommandBuffer> &hCommandBuffer);

		DLLCLIENT void AddLightSource(lua_State *l,std::shared_ptr<::Scene> &scene,CLightHandle &hLight);
		DLLCLIENT void RemoveLightSource(lua_State *l,std::shared_ptr<::Scene> &scene,CLightHandle &hLight);
		DLLCLIENT void SetLightSources(lua_State *l,std::shared_ptr<::Scene> &scene,luabind::object o);
		DLLCLIENT void GetLightSources(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void LinkLightSources(lua_State *l,std::shared_ptr<::Scene> &scene,std::shared_ptr<::Scene> &sceneOther);

		DLLCLIENT void AddEntity(lua_State *l,std::shared_ptr<::Scene> &scene,EntityHandle &hEnt);
		DLLCLIENT void RemoveEntity(lua_State *l,std::shared_ptr<::Scene> &scene,EntityHandle &hEnt);
		DLLCLIENT void SetEntities(lua_State *l,std::shared_ptr<::Scene> &scene,luabind::object o);
		DLLCLIENT void GetEntities(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void LinkEntities(lua_State *l,std::shared_ptr<::Scene> &scene,std::shared_ptr<::Scene> &sceneOther);

		DLLCLIENT void GetPrepassShader(lua_State *l,std::shared_ptr<::Scene> &scene);

		DLLCLIENT void GetCameraDescriptorSet(lua_State *l,std::shared_ptr<::Scene> &scene);
		DLLCLIENT void GetCameraDescriptorSet(lua_State *l,std::shared_ptr<::Scene> &scene,uint32_t bindPoint);
		DLLCLIENT void GetViewCameraDescriptorSet(lua_State *l,std::shared_ptr<::Scene> &scene);

		DLLCLIENT void SetShaderOverride(lua_State *l,std::shared_ptr<::Scene> &scene,const std::string &srcName,const std::string &dstName);
		DLLCLIENT void ClearShaderOverride(lua_State *l,std::shared_ptr<::Scene> &scene,const std::string &srcName);

		DLLCLIENT void SetPrepassMode(lua_State *l,std::shared_ptr<::Scene> &scene,uint32_t mode);
		DLLCLIENT void GetPrepassMode(lua_State *l,std::shared_ptr<::Scene> &scene);
	};
	namespace Camera
	{
		DLLCLIENT void Create(lua_State *l,float fov,float fovView,float aspectRatio,float nearZ,float farZ);
		DLLCLIENT void Copy(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetProjectionMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetViewMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetRight(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetUp(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void SetUp(lua_State *l,std::shared_ptr<::Camera> &hCam,Vector3 &up);
		DLLCLIENT void GetPos(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void SetPos(lua_State *l,std::shared_ptr<::Camera> &hCam,Vector3 &pos);
		DLLCLIENT void LookAt(lua_State *l,std::shared_ptr<::Camera> &hCam,const Vector3 &pos);
		DLLCLIENT void UpdateMatrices(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void UpdateViewMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void UpdateProjectionMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void UpdateViewProjectionMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void UpdateProjectionMatrices(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void SetFOV(lua_State *l,std::shared_ptr<::Camera> &hCam,float fov);
		DLLCLIENT void SetViewFOV(lua_State *l,std::shared_ptr<::Camera> &hCam,float fov);
		DLLCLIENT void SetAspectRatio(lua_State *l,std::shared_ptr<::Camera> &hCam,float aspectRatio);
		DLLCLIENT void SetZNear(lua_State *l,std::shared_ptr<::Camera> &hCam,float nearZ);
		DLLCLIENT void SetZFar(lua_State *l,std::shared_ptr<::Camera> &hCam,float farZ);
		DLLCLIENT void SetForward(lua_State *l,std::shared_ptr<::Camera> &hCam,Vector3 &forward);
		DLLCLIENT void GetViewProjectionMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetForward(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetFOV(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetViewFOV(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetFOVRad(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetViewFOVRad(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetAspectRatio(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetZNear(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetZFar(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetFrustumPlanes(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetFarPlaneCenter(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetNearPlaneCenter(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetFarPlaneBoundaries(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetNearPlaneBoundaries(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetPlaneBoundaries(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void SetProjectionMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam,Mat4 &mat);
		DLLCLIENT void SetViewMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam,Mat4 &mat);
		DLLCLIENT void SetViewProjectionMatrix(lua_State *l,std::shared_ptr<::Camera> &hCam,Mat4 &mat);
		DLLCLIENT void GetNearPlaneBounds(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetFarPlaneBounds(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetFrustumPoints(lua_State *l,std::shared_ptr<::Camera> &hCam);
		DLLCLIENT void GetNearPlanePoint(lua_State *l,std::shared_ptr<::Camera> &hCam,const Vector2 &uv);
		DLLCLIENT void GetFarPlanePoint(lua_State *l,std::shared_ptr<::Camera> &hCam,const Vector2 &uv);

		DLLCLIENT void GetFrustumNeighbors(lua_State *l,std::shared_ptr<::Camera> &hCam,int planeID);
		DLLCLIENT void GetFrustumPlaneCornerPoints(lua_State *l,std::shared_ptr<::Camera> &hCam,int planeA,int planeB);
		DLLCLIENT void CreateFrustumKDop(lua_State *l,std::shared_ptr<::Camera> &hCam,luabind::object o1,luabind::object o2,Vector3 dir);
		DLLCLIENT void CreateFrustumKDop(lua_State *l,std::shared_ptr<::Camera> &hCam,const Vector2 &uvStart,const Vector2 &uvEnd);
		DLLCLIENT void GetRotation(lua_State *l,std::shared_ptr<::Camera> &hCam);

		DLLCLIENT void CreateFrustumMesh(lua_State *l,std::shared_ptr<::Camera> &hCam,const Vector2 &uvStart,const Vector2 &uvEnd);
	};
};

#endif
