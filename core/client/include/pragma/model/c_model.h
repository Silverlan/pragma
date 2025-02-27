/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_MODEL_H__
#define __C_MODEL_H__
#include "pragma/clientdefinitions.h"
#include <pragma/model/model.h>
#include "material.h"

class CModelMesh;
namespace prosper {
	class IBuffer;
};
class DLLCLIENT CModel : public Model {
  public:
	CModel(NetworkState *nw, uint32_t numBones, const std::string &name = "");
	void AddMesh(const std::string &meshGroup, const std::shared_ptr<CModelMesh> &mesh);
	virtual void PrecacheTextureGroup(uint32_t i) override;
	virtual void PrecacheTexture(uint32_t texId, bool bReload = false) override;
	virtual void Update(ModelUpdateFlags flags = ModelUpdateFlags::AllData) override;

	virtual std::shared_ptr<ModelMesh> CreateMesh() const override;
	virtual std::shared_ptr<ModelSubMesh> CreateSubMesh() const override;

	const std::shared_ptr<prosper::IBuffer> &GetVertexAnimationBuffer() const;
	bool GetVertexAnimationBufferFrameOffset(uint32_t vaIdx, CModelSubMesh &subMesh, uint32_t frameId, uint64_t &offset) const;
  protected:
	virtual void AddMesh(const std::string &meshGroup, const std::shared_ptr<ModelMesh> &mesh) override;
	virtual void OnMaterialMissing(const std::string &matName) override;

	std::shared_ptr<prosper::IBuffer> m_vertexAnimationBuffer = nullptr;
	std::vector<std::vector<std::vector<uint32_t>>> m_frameIndices = {};
	void UpdateVertexAnimationBuffer();
};
#endif
