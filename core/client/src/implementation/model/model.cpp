// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :model;
import :model;

import :client_state;

pragma::asset::CModel::CModel(NetworkState *nw, unsigned int numBones, const std::string &name) : Model(nw, numBones, name) {}

std::shared_ptr<pragma::geometry::ModelMesh> pragma::asset::CModel::CreateMesh() const { return pragma::util::make_shared<geometry::CModelMesh>(); }
std::shared_ptr<pragma::geometry::ModelSubMesh> pragma::asset::CModel::CreateSubMesh() const { return pragma::util::make_shared<geometry::CModelSubMesh>(); }

void pragma::asset::CModel::PrecacheTextureGroup(uint32_t i) { Model::PrecacheTextureGroup(i); }

void pragma::asset::CModel::OnMaterialMissing(const std::string &matName)
{
	NetPacket p {};
	p->WriteString(GetName());
	p->WriteString(matName);
	get_client_state()->SendPacket(networking::net_messages::server::QUERY_MODEL_TEXTURE, p, networking::Protocol::FastUnreliable);
}

void pragma::asset::CModel::PrecacheTexture(uint32_t texId, bool bReload) { Model::PrecacheTexture(texId, bReload); }

void pragma::asset::CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<geometry::ModelMesh> &mesh) {}

void pragma::asset::CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<geometry::CModelMesh> &mesh) { Model::AddMesh(meshGroup, mesh); }

void pragma::asset::CModel::Update(ModelUpdateFlags flags)
{
	Model::Update(flags);
	if((flags & ModelUpdateFlags::UpdateVertexAnimationBuffer) != ModelUpdateFlags::None)
		UpdateVertexAnimationBuffer();
}
