// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

module pragma.client;

import :model;
import :model;

import :client_state;

pragma::asset::CModel::CModel(pragma::NetworkState *nw, unsigned int numBones, const std::string &name) : pragma::asset::Model(nw, numBones, name) {}

std::shared_ptr<pragma::geometry::ModelMesh> pragma::asset::CModel::CreateMesh() const { return ::util::make_shared<pragma::geometry::CModelMesh>(); }
std::shared_ptr<pragma::geometry::ModelSubMesh> pragma::asset::CModel::CreateSubMesh() const { return ::util::make_shared<pragma::geometry::CModelSubMesh>(); }

void pragma::asset::CModel::PrecacheTextureGroup(uint32_t i) { pragma::asset::Model::PrecacheTextureGroup(i); }

void pragma::asset::CModel::OnMaterialMissing(const std::string &matName)
{
	NetPacket p {};
	p->WriteString(GetName());
	p->WriteString(matName);
	pragma::get_client_state()->SendPacket(pragma::networking::net_messages::server::QUERY_MODEL_TEXTURE, p, pragma::networking::Protocol::FastUnreliable);
}

void pragma::asset::CModel::PrecacheTexture(uint32_t texId, bool bReload) { pragma::asset::Model::PrecacheTexture(texId, bReload); }

void pragma::asset::CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<pragma::geometry::ModelMesh> &mesh) {}

void pragma::asset::CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<pragma::geometry::CModelMesh> &mesh) { pragma::asset::Model::AddMesh(meshGroup, mesh); }

void pragma::asset::CModel::Update(pragma::asset::ModelUpdateFlags flags)
{
	pragma::asset::Model::Update(flags);
	if((flags & pragma::asset::ModelUpdateFlags::UpdateVertexAnimationBuffer) != pragma::asset::ModelUpdateFlags::None)
		UpdateVertexAnimationBuffer();
}
