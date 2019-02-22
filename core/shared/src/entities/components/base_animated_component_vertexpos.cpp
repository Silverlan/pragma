#include "stdafx_shared.h"
#include "pragma/entities/components/base_animated_component.hpp"
#include "pragma/entities/components/base_model_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/model/model.h"
#include "pragma/model/modelmesh.h"

using namespace pragma;

bool BaseAnimatedComponent::GetVertexPosition(uint32_t meshGroupId,uint32_t meshId,uint32_t subMeshId,uint32_t vertexId,Vector3 &pos) const
{
	auto mdlComponent = GetEntity().GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
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
bool BaseAnimatedComponent::GetLocalVertexPosition(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 &pos) const
{
	auto &verts = const_cast<ModelSubMesh&>(subMesh).GetVertices();
	if(vertexId >= verts.size())
		return false;
	auto &v = verts.at(vertexId);
	pos = v.position;
	return true;
}
bool BaseAnimatedComponent::GetVertexPosition(const ModelSubMesh &subMesh,uint32_t vertexId,Vector3 &pos) const
{
	auto &ent = GetEntity();
	auto mdlComponent = ent.GetModelComponent();
	auto hMdl = mdlComponent.valid() ? mdlComponent->GetModel() : nullptr;
	if(hMdl == nullptr || GetLocalVertexPosition(subMesh,vertexId,pos) == false)
		return false;
	auto &vertWeights = const_cast<ModelSubMesh&>(subMesh).GetVertexWeights();
	auto pTrComponent = ent.GetTransformComponent();
	auto scale = pTrComponent.valid() ? pTrComponent->GetScale() : Vector3{1.f,1.f,1.f};
	auto transformMatrix = umat::identity();
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
			auto &refFrame = hMdl->GetReference();
			auto &pos = t.GetPosition();

			auto &orientation = t.GetOrientation();
			auto &scale = t.GetScale();

			auto *posBind = refFrame.GetBonePosition(boneId);
			auto *rotBind = refFrame.GetBoneOrientation(boneId);
			if(posBind != nullptr && rotBind != nullptr)
			{
				auto mat = umat::identity(); // Inverse of parent bones??
				auto rot = orientation *glm::inverse(*rotBind);
				mat = glm::translate(mat,*posBind);

				// TODO: Is this the correct order? (It looks correct?)
				if(scale.x != 1.f || scale.y != 1.f || scale.z != 1.f)
					mat = glm::scale(mat,scale);

				mat = mat *glm::toMat4(rot);
				mat = glm::translate(mat,-(*posBind));
				mat = glm::translate(pos -(*posBind)) *mat;

				transformMatrix += weight *mat;
			}
			//
		}
	}

	auto vpos = transformMatrix *Vector4{pos.x,pos.y,pos.z,1.f};
	auto mdlMatrix = umat::identity();
	if(pTrComponent.valid())
		mdlMatrix = glm::translate(umat::identity(),pTrComponent->GetOrigin()) *umat::create(pTrComponent->GetOrientation()) *glm::scale(umat::identity(),scale);
	vpos = mdlMatrix *vpos;
	pos = Vector3{vpos.x,vpos.y,vpos.z} /vpos.w;
	return true;
}
