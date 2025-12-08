// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :model;
import :model;

import :client_state;

CModel::CModel(pragma::NetworkState *nw, unsigned int numBones, const std::string &name) : pragma::asset::Model(nw, numBones, name) {}

std::shared_ptr<pragma::geometry::ModelMesh> CModel::CreateMesh() const { return ::util::make_shared<CModelMesh>(); }
std::shared_ptr<pragma::geometry::ModelSubMesh> CModel::CreateSubMesh() const { return ::util::make_shared<CModelSubMesh>(); }

void CModel::PrecacheTextureGroup(uint32_t i) { pragma::asset::Model::PrecacheTextureGroup(i); }

void CModel::OnMaterialMissing(const std::string &matName)
{
	NetPacket p {};
	p->WriteString(GetName());
	p->WriteString(matName);
	pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::QUERY_MODEL_TEXTURE, p, pragma::networking::Protocol::FastUnreliable);
}

void CModel::PrecacheTexture(uint32_t texId, bool bReload) { pragma::asset::Model::PrecacheTexture(texId, bReload); }

void CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<pragma::geometry::ModelMesh> &mesh) {}

void CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<CModelMesh> &mesh) { pragma::asset::Model::AddMesh(meshGroup, mesh); }

void CModel::Update(pragma::asset::ModelUpdateFlags flags)
{
	pragma::asset::Model::Update(flags);
	if((flags & pragma::asset::ModelUpdateFlags::UpdateVertexAnimationBuffer) != pragma::asset::ModelUpdateFlags::None)
		UpdateVertexAnimationBuffer();
}
