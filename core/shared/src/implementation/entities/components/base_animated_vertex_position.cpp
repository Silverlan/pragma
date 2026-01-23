// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :entities.components.base_animated;

using namespace pragma;

bool BaseAnimatedComponent::GetVertexPosition(uint32_t meshGroupId, uint32_t meshId, uint32_t subMeshId, uint32_t vertexId, Vector3 &pos) const
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
	return GetVertexPosition(*subMesh, vertexId, pos);
}
bool BaseAnimatedComponent::GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, math::ScaledTransform &outPose) const
{
	auto &verts = const_cast<geometry::ModelSubMesh &>(subMesh).GetVertices();
	if(vertexId >= verts.size())
		return false;
	auto *bindPose = GetBindPose();
	if(bindPose == nullptr)
		return false;
	auto &vertWeights = const_cast<geometry::ModelSubMesh &>(subMesh).GetVertexWeights();
	math::ScaledTransform transformMatrix {};
	auto valid = false;
	std::vector<Quat> rots;
	std::vector<float> rotWeights;
	if(vertexId < vertWeights.size()) {
		auto &processedBones = GetProcessedBones();
		auto &vw = vertWeights.at(vertexId);
		for(auto i = 0u; i < 4u; ++i) {
			auto boneId = vw.boneIds[i];
			if(boneId == -1)
				continue;
			if(boneId >= processedBones.size())
				continue;
			auto weight = vw.weights[i];
			auto &t = processedBones.at(boneId);

			//
			auto &pos = t.GetOrigin();

			auto &orientation = t.GetRotation();
			auto &scale = t.GetScale();

			auto *posBind = bindPose->GetBonePosition(boneId);
			auto *rotBind = bindPose->GetBoneOrientation(boneId);
			if(posBind != nullptr && rotBind != nullptr) {
				math::Transform tBindPose {*posBind, *rotBind};
				tBindPose = tBindPose.GetInverse();

				auto tTmp = t;
				tTmp.SetScale(Vector3 {1.f, 1.f, 1.f});
				auto l0 = uquat::length(tTmp.GetRotation());
				auto l1 = uquat::length(tBindPose.GetRotation());
				auto mat = tTmp * tBindPose;
				//auto mat = (t *tBindPose).ToMatrix();
				//transformMatrix += weight *mat;
				transformMatrix.SetScale(transformMatrix.GetScale() + mat.GetScale() * weight);
				transformMatrix.SetOrigin(transformMatrix.GetOrigin() + mat.GetOrigin() * weight);
				rots.push_back(mat.GetRotation());
				rotWeights.push_back(weight);
				valid = true;
			}
			//
		}
	}
	if(valid == false) {
		outPose = {};
		return true;
	}
	transformMatrix.SetRotation(uquat::calc_average(rots, rotWeights));
	outPose = transformMatrix;
	return true;
}
std::optional<Mat4> BaseAnimatedComponent::GetVertexTransformMatrix(const geometry::ModelSubMesh &subMesh, uint32_t vertexId) const
{
	auto &verts = const_cast<geometry::ModelSubMesh &>(subMesh).GetVertices();
	if(vertexId >= verts.size())
		return {};
	auto *bindPose = GetBindPose();
	if(bindPose == nullptr)
		return {};
	auto &vertWeights = const_cast<geometry::ModelSubMesh &>(subMesh).GetVertexWeights();
	auto transformMatrix = glm::mat4 {0.f};
	auto valid = false;
	if(vertexId < vertWeights.size()) {
		auto &processedBones = GetProcessedBones();
		auto &vw = vertWeights.at(vertexId);
		for(auto i = 0u; i < 4u; ++i) {
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
			if(posBind != nullptr && rotBind != nullptr) {
				math::Transform tBindPose {*posBind, *rotBind};
				tBindPose = tBindPose.GetInverse();

				auto mat = t.ToMatrix() * tBindPose.ToMatrix();
				//auto mat = (t *tBindPose).ToMatrix();
				transformMatrix += weight * mat;
				valid = true;
			}
			//
		}
	}
	if(valid == false)
		return umat::identity();
	return transformMatrix;
}
bool BaseAnimatedComponent::GetLocalVertexPosition(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, Vector3 &pos, const std::optional<Vector3> &vertexOffset) const
{
	auto transformMatrix = GetVertexTransformMatrix(subMesh, vertexId);
	if(transformMatrix.has_value() == false)
		return false;
	auto vpos = *transformMatrix * Vector4 {pos.x, pos.y, pos.z, 1.f};
	if(vpos.w < 0.001f)
		return true;
	pos = Vector3 {vpos.x, vpos.y, vpos.z} / vpos.w;
	return true;
}
bool BaseAnimatedComponent::GetVertexPosition(const geometry::ModelSubMesh &subMesh, uint32_t vertexId, Vector3 &pos) const
{
	auto &ent = GetEntity();
	auto &hMdl = ent.GetModel();
	if(hMdl == nullptr || GetLocalVertexPosition(subMesh, vertexId, pos) == false)
		return false;
	auto &pose = ent.GetPose();
	pos = pose * pos;
	return true;
}
