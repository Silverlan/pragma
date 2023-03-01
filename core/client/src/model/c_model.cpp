/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include <pragma/networking/enums.hpp>

extern DLLCLIENT ClientState *client;

CModel::CModel(NetworkState *nw, unsigned int numBones, const std::string &name) : Model(nw, numBones, name) {}

std::shared_ptr<ModelMesh> CModel::CreateMesh() const { return std::make_shared<CModelMesh>(); }
std::shared_ptr<ModelSubMesh> CModel::CreateSubMesh() const { return std::make_shared<CModelSubMesh>(); }

void CModel::PrecacheTextureGroup(uint32_t i) { Model::PrecacheTextureGroup(i); }

void CModel::OnMaterialMissing(const std::string &matName)
{
	NetPacket p {};
	p->WriteString(GetName());
	p->WriteString(matName);
	client->SendPacket("query_model_texture", p, pragma::networking::Protocol::FastUnreliable);
}

void CModel::PrecacheTexture(uint32_t texId, bool bReload) { Model::PrecacheTexture(texId, bReload); }

void CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<ModelMesh> &mesh) {}

void CModel::AddMesh(const std::string &meshGroup, const std::shared_ptr<CModelMesh> &mesh) { Model::AddMesh(meshGroup, mesh); }

void CModel::Update(ModelUpdateFlags flags)
{
	Model::Update(flags);
	if((flags & ModelUpdateFlags::UpdateVertexAnimationBuffer) != ModelUpdateFlags::None)
		UpdateVertexAnimationBuffer();
}
