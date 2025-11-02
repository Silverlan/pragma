// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


module pragma.client;


import :model;
import :model;

import :client_state;


CModel::CModel(NetworkState *nw, unsigned int numBones, const std::string &name) : pragma::Model(nw, numBones, name) {}

std::shared_ptr<ModelMesh> CModel::CreateMesh() const { return std::make_shared<CModelMesh>(); }
std::shared_ptr<pragma::ModelSubMesh> CModel::CreateSubMesh() const { return std::make_shared<CModelSubMesh>(); }

void CModel::PrecacheTextureGroup(uint32_t i) { pragma::Model::PrecacheTextureGroup(i); }

void CModel::OnMaterialMissing(const std::string &matName)
{
	NetPacket p {};
	p->WriteString(GetName());
	p->WriteString(matName);
	pragma::get_client_state()->SendPacket("query_model_texture", p, pragma::networking::Protocol::FastUnreliable);
}

void CModel::PrecacheTexture(uint32_t texId, bool bReload) { pragma::Model::PrecacheTexture(texId, bReload); }

void CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<ModelMesh> &mesh) {}

void CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<CModelMesh> &mesh) { pragma::Model::AddMesh(meshGroup, mesh); }

void CModel::Update(pragma::model::ModelUpdateFlags flags)
{
	pragma::Model::Update(flags);
	if((flags & pragma::model::ModelUpdateFlags::UpdateVertexAnimationBuffer) != pragma::model::ModelUpdateFlags::None)
		UpdateVertexAnimationBuffer();
}
