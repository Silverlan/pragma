/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/game/c_game.h"
#include <image/prosper_texture.hpp>
#include <pragma/lua/libraries/limport.hpp>
#include <pragma/entities/baseentity_handle.h>
#include <pragma/lua/classes/ldef_entity.h>
#include <pragma/lua/libraries/lfile.h>
#include <pragma/entities/components/c_light_map_component.hpp>
#include <pragma/entities/entity_iterator.hpp>
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <mathutil/transform.hpp>
#include <pragma/util/util_game.hpp>
#include <pragma/game/scene_snapshot.hpp>
#include <sharedutils/util_file.h>
//#include <assimp/Importer.hpp>
//#include <assimp/Exporter.hpp>
//#include <assimp/scene.h>
//#include <assimp/postprocess.h>
//#include <assimp/IOSystem.hpp>
//#include <assimp/IOStream.hpp>
#include "pragma/lua/libraries/c_limport.hpp"
#include "pragma/entities/environment/lights/c_env_light.h"
#include "pragma/entities/environment/lights/c_env_light_spot.h"
#include "pragma/entities/environment/lights/c_env_light_point.h"
#include "pragma/entities/environment/lights/c_env_light_directional.h"
#include "pragma/entities/components/c_color_component.hpp"
#include "pragma/entities/components/c_radius_component.hpp"
#include "pragma/entities/environment/c_env_camera.h"

extern DLLCLIENT CGame *c_game;

#if 0
static aiVector3D to_assimp_position(const Vector3 &pos)
{
	return aiVector3D{pos.x,pos.y,pos.z} *static_cast<float>(pragma::units_to_metres(1.f));
}
static aiVector3D to_assimp_normal(const Vector3 &dir)
{
	return aiVector3D{-dir.x,-dir.y,dir.z};
}
static Mat4 to_pragma_matrix(const aiMatrix4x4 &m)
{
	Mat4 mOut {
		m[0][0],m[0][1],m[0][2],m[0][3],
		m[1][0],m[1][1],m[1][2],m[1][3],
		m[2][0],m[2][1],m[2][2],m[2][3],
		m[3][0],m[3][1],m[3][2],m[3][3]
	};
	mOut = glm::transpose(mOut); // Assimp matrices are row-major, but we work with column-major

	// Invert rows 0 and 2, as well as columns 0 and 1.
	// I don't really know why this is necessary, it has been determined
	// through trial and error.
	for(uint8_t i=0;i<4;++i)
	{
		mOut[0][i] *= -1.f;
		mOut[2][i] *= -1.f;

		mOut[i][0] *= -1.f;
		mOut[i][1] *= -1.f;
	}
	return umat::create_from_axis_angle(Vector3{1.f,0.f,0.f},umath::rad_to_deg(90.f)) *mOut;
}
static aiMatrix4x4 to_assimp_matrix(const Mat4 &m)
{
	auto mOut = umat::create_from_axis_angle(Vector3{1.f,0.f,0.f},umath::rad_to_deg(-90.f)) *m;

	// Invert rows 0 and 2, as well as columns 0 and 1
	for(uint8_t i=0;i<4;++i)
	{
		mOut[0][i] *= -1.f;
		mOut[2][i] *= -1.f;

		mOut[i][0] *= -1.f;
		mOut[i][1] *= -1.f;
	}
	mOut = glm::transpose(mOut);
	return aiMatrix4x4 {
		mOut[0][0],mOut[0][1],mOut[0][2],mOut[0][3],
		mOut[1][0],mOut[1][1],mOut[1][2],mOut[1][3],
		mOut[2][0],mOut[2][1],mOut[2][2],mOut[2][3],
		mOut[3][0],mOut[3][1],mOut[3][2],mOut[3][3]
	};
}

static aiNode &add_node(aiNode &parentNode,uint32_t index,const std::string &name,BaseEntity &ent)
{
	auto &t = ent.GetPose();

	auto scale = static_cast<float>(pragma::units_to_metres(1.f));
	umath::ScaledTransform tScaled = t;
	tScaled.Scale(Vector3{scale,scale,scale});

	auto m = tScaled.ToMatrix();
	auto nodeName = ent.GetName();
	if(nodeName.empty())
		nodeName = name;
	auto *node = new aiNode {};
	node->mName = nodeName +'_' +std::to_string(ent.GetLocalIndex());
	node->mParent = &parentNode;
	node->mTransformation = to_assimp_matrix(m);
	parentNode.mChildren[index] = node;
	return *node;
}
#endif
int Lua::lib_export::export_scene(lua_State *l)
{
#if 0
	std::string outputPath = FileManager::GetCanonicalizedPath(Lua::CheckString(l,1));
	if(Lua::file::validate_write_operation(l,outputPath) == false)
		return 0;
	auto *world = c_game->GetWorld();
	auto mdl = world ? world->GetEntity().GetModel() : nullptr;
	if(mdl == nullptr)
		return 0;

	auto &ent = world->GetEntity();

	Assimp::Exporter exporter;
	auto sceneSnapshot = pragma::SceneSnapshot::Create();
	sceneSnapshot->AddModel(*mdl,ent.GetSkin());
	sceneSnapshot->MergeMeshesByMaterial();
	auto scene = Lua::import::snapshot_to_assimp_scene(*sceneSnapshot);

#if 0
	// Export light sources
	EntityIterator entIt {*c_game};
	entIt.AttachFilter<TEntityIteratorFilterComponent<pragma::CLightComponent>>();
	auto numLights = entIt.GetCount();
	scene->mLights = new aiLight*[numLights];
	scene->mNumLights = numLights;

	auto *meshNode = scene->mRootNode->mChildren[0];
	auto numNodesOld = scene->mRootNode->mNumChildren;
	auto numNodes = numNodesOld +2; // +2 for the camera and the mesh nodes
	auto *oldNodes = scene->mRootNode->mChildren;
	scene->mRootNode->mChildren = new aiNode*[numNodes];
	for(auto i=decltype(numNodesOld){0u};i<numNodesOld;++i)
		scene->mRootNode->mChildren[i] = oldNodes[i];
	delete[] oldNodes;

	scene->mRootNode->mNumChildren = numNodes;

	uint32_t nodeIdx = numNodesOld;
	uint32_t lightIdx = 0;
	for(auto *ent : entIt)
	{
		auto lightC = ent->GetComponent<pragma::CLightComponent>();
		
		std::string name = add_node(*scene->mRootNode,nodeIdx,"light",*ent).mName.C_Str();

		auto *lightOut = new aiLight {};
		lightOut->mName = name;

		auto radiusC = ent->GetComponent<pragma::CRadiusComponent>();
		auto spotLightC = ent->GetComponent<pragma::CLightSpotComponent>();
		auto pointLightC = ent->GetComponent<pragma::CLightPointComponent>();
		auto dirLightC = ent->GetComponent<pragma::CLightDirectionalComponent>();
		if(spotLightC.valid())
		{
			lightOut->mType = aiLightSourceType::aiLightSource_SPOT;
			lightOut->mAngleInnerCone = umath::deg_to_rad(spotLightC->GetInnerCutoffAngle());
			lightOut->mAngleOuterCone = umath::deg_to_rad(spotLightC->GetOuterCutoffAngle());
			lightOut->mAttenuationLinear = radiusC.valid() ? radiusC->GetRadius() : 1.f;
		}
		else if(pointLightC.valid())
		{
			lightOut->mType = aiLightSourceType::aiLightSource_POINT;
			lightOut->mAngleInnerCone = lightOut->mAngleOuterCone = umath::pi *2.0;
			lightOut->mAttenuationLinear = radiusC.valid() ? radiusC->GetRadius() : 1.f;
		}
		else if(dirLightC.valid())
			lightOut->mType = aiLightSourceType::aiLightSource_DIRECTIONAL;
		auto colC = ent->GetComponent<pragma::CColorComponent>();
		auto col = colC.valid() ? colC->GetColor() : Color::White;
		auto vcol = col.ToVector4();
		lightOut->mColorDiffuse = aiColor3D{vcol.r,vcol.g,vcol.b} *vcol.a;

		scene->mLights[lightIdx++] = lightOut;
		++nodeIdx;
	}

	// Export camera
	// Note: Assimp seems to be practically unusable for importing/exporting anything other than mesh geometry or animations.
	// We'll probably have to write our own exporter at some point instead.
	auto *cam = c_game->GetRenderCamera();
	auto camNodeIndex = nodeIdx;
	scene->mNumCameras = 1;
	scene->mCameras = new aiCamera*[1];
	auto *outCam = new aiCamera {};
	scene->mCameras[0] = outCam;
	if(cam)
	{
		auto &entCam = cam->GetEntity();
		std::string name = add_node(*scene->mRootNode,camNodeIndex,"cam",cam->GetEntity()).mName.C_Str();
		outCam->mAspect = cam->GetAspectRatio();
		outCam->mClipPlaneFar = ::pragma::units_to_metres(cam->GetFarZ());
		outCam->mClipPlaneNear = ::pragma::units_to_metres(cam->GetNearZ());
		outCam->mHorizontalFOV = cam->GetFOVRad();
		outCam->mName = name;
	}
#endif

	if(outputPath.empty() || (outputPath.front() != '/' && outputPath.front() != '\\'))
		outputPath = '/' +outputPath;
	outputPath = FileManager::GetProgramPath() +outputPath;
	ufile::remove_extension_from_filename(outputPath);
	// The assimp FBX exporter currently does not support lights or cameras!
	outputPath += ".fbx";
	auto result = exporter.Export(scene.get(),"fbx",outputPath);

	//outputPath += ".dae";
	//auto result = exporter.Export(&scene,"collada",outputPath);
	Lua::PushBool(l,result == aiReturn::aiReturn_SUCCESS);

	if(result != aiReturn::aiReturn_SUCCESS)
	{
		auto *error = exporter.GetErrorString();
		Lua::PushString(l,error);
		return 2;
	}
	return 1;
#endif
	return 0;
}
