/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __RENDERSYSTEM_H__
#define __RENDERSYSTEM_H__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/rendering/c_entitymeshinfo.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"
#include <vector>
#include <deque>

class DLLCLIENT RenderSystem;
//class DLLCLIENT RenderWrapper;
class Material;
class CBaseEntity;

namespace prosper {class Shader; class IPrimaryCommandBuffer;};
namespace pragma
{
	class CLightComponent; class CCameraComponent;
	namespace rendering {struct CulledMeshData;};
};
namespace util {struct DrawSceneInfo;};
class CModelMesh;
#pragma warning(push)
#pragma warning(disable : 4251)
enum class RenderFlags : uint8_t
{
	None = 0u,
	Reflection = 1u,
	RenderAs3DSky = Reflection<<1u
};
class DLLCLIENT RenderSystem
{
public:
	struct DLLCLIENT MaterialMeshContainer
	{
		MaterialMeshContainer(Material *mat);
		MaterialMeshContainer(MaterialMeshContainer&)=delete;
		MaterialMeshContainer &operator=(const MaterialMeshContainer &other)=delete;
		Material *material;
		std::unordered_map<CBaseEntity*,EntityMeshInfo> containers;
	};
	struct DLLCLIENT TranslucentMesh
	{
		TranslucentMesh(CBaseEntity *ent,CModelSubMesh *mesh,Material *mat,::util::WeakHandle<prosper::Shader> shader,float distance);
		CBaseEntity *ent;
		CModelSubMesh *mesh;
		Material *material;
		::util::WeakHandle<prosper::Shader> shader = {};
		float distance;
	};
public:
	static void Render(const util::DrawSceneInfo &drawSceneInfo,pragma::CCameraComponent &cam,RenderMode renderMode,RenderFlags flags,std::vector<std::unique_ptr<RenderSystem::TranslucentMesh>> &translucentMeshes,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f});
	
	static uint32_t Render(const util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode=RenderMode::World,RenderFlags flags=RenderFlags::None,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f});
	static uint32_t Render(const util::DrawSceneInfo &drawSceneInfo,const pragma::rendering::CulledMeshData &renderMeshes,RenderMode renderMode=RenderMode::World,RenderFlags flags=RenderFlags::None,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f});

	static void RenderPrepass(const util::DrawSceneInfo &drawSceneInfo,RenderMode renderMode=RenderMode::World);
	static void RenderShadows(const util::DrawSceneInfo &drawSceneInfo,pragma::CRasterizationRendererComponent &renderer,std::vector<pragma::CLightComponent*> &lights);
};
REGISTER_BASIC_BITWISE_OPERATORS(RenderFlags)
#pragma warning(pop)

#endif
