/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"

using namespace pragma;

bool BaseAnimatedComponent::GetVertexPosition(uint32_t meshGroupId,uint32_t meshId,uint32_t subMeshId,uint32_t vertexId,Vector3 &pos) const
{
	auto &hMdl = GetEntity().GetModel();
	if(hMdl == nullptr)
		return false;
	auto meshGroup = hMdl->GetMeshGroup(meshGroupId);
	if(meshGroup == nullptr)
		return false;
	auto &meshes = meshGroup->GetMeshes();
	if(meshId >= meshes.size())
		return false;
	auto &mesh = meshes.at(meshId);
	auto &subMeshes = mesh->GetSubMeshes();
	if(subMeshId >= subMeshes.size())
		return false;
	auto &subMesh = subMeshes.at(subMeshId);
	return GetVertexPosition(*subMesh,vertexId,pos);
}
std::optional<Mat4> BaseAnimatedComponent::GetVertexTransformMatrix(const ModelSubMesh &subMesh,uint32_t vertexId) const
{
	auto &verts = const_cast<ModelSubMesh&>(subMesh).GetVertices();
	if(vertexId >= verts.size())
		return {};
	auto *bindPose = GetBindPose();
	if(bindPose == nullptr)
		return {};
	auto &vertWeights = const_cast<ModelSubMesh&>(subMesh).GetVertexWeights();
	auto transformMatrix = glm::mat4{0.f};
	auto valid = false;
	if(vertexId < vertWeights.size())
	{
		auto &processedBones = GetProcessedBones();
		auto &vw = vertWeights.at(vertexId);
		for(auto i=0u;i<4u;++i)
		{
			auto boneId = vw.boneIds[i];
			if(boneId == -1 || boneId >= processedBones.size())
				continue;
			auto weight = vw.weights[i];
			auto &t = processedBones.at(boneId);

			//
			auto &pos = t.GetOrigin();

			auto &orientation = t.GetRotation();
			auto &scale = t.GetScale();

			auto *posBind = bindPose->GetBonePosition(boneId);
			auto *rotBind = bindPose->GetBoneOrientation(boneId);
			if(posBind != nullptr && rotBind != nullptr)
			{
				umath::Transform tBindPose {*posBind,*rotBind};
				tBindPose = tBindPose.GetInverse();

				auto mat = t.ToMatrix() *tBindPose.ToMatrix();
				//auto mat = (t *tBindPose).ToMatrix();
				transformMatrix += weight *mat;
				valid = true;
			}
			//
		}
	}
	if(valid == false)
		return umat::identity();
	return transformMatrix;
}
bool BaseAnimatedComponent::GetLocalVertexPosition(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 &pos,const std::optional<Vector3> &vertexOffset) const
{
	auto transformMatrix = GetVertexTransformMatrix(subMesh,vertexId);
	if(transformMatrix.has_value() == false)
		return false;
	auto vpos = *transformMatrix *Vector4{pos.x,pos.y,pos.z,1.f};
	if(vpos.w < 0.001f)
		return true;
	pos = Vector3{vpos.x,vpos.y,vpos.z} /vpos.w;
	return true;
}
bool BaseAnimatedComponent::GetVertexPosition(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 &pos) const
{
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr || GetLocalVertexPosition(subMesh,vertexId,pos) == false)
		return false;
	auto &pose = ent.GetPose();
	pos *= pose;
	return true;
}
