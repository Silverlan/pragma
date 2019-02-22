#ifndef __C_MODEL_H__
#define __C_MODEL_H__
#include "pragma/clientdefinitions.h"
#include <pragma/model/model.h>
#include "material.h"

class CModelMesh;
namespace prosper {class Buffer;};
class DLLCLIENT CModel
	: public Model
{
public:
	CModel(NetworkState *nw,uint32_t numBones,const std::string &name="");
	void AddMesh(const std::string &meshGroup,const std::shared_ptr<CModelMesh> &mesh);
	virtual void PrecacheTextureGroup(uint32_t i) override;
	virtual void PrecacheTexture(uint32_t texId,bool bReload=false) override;
	virtual void Update(ModelUpdateFlags flags=ModelUpdateFlags::AllData) override;

	const std::shared_ptr<prosper::Buffer> &GetVertexAnimationBuffer() const;
	bool GetVertexAnimationBufferFrameOffset(uint32_t vaIdx,CModelSubMesh &subMesh,uint32_t frameId,uint64_t &offset) const;
protected:
	virtual void AddMesh(const std::string &meshGroup,const std::shared_ptr<ModelMesh> &mesh) override;
	virtual void OnMaterialMissing(const std::string &matName) override;

	std::shared_ptr<prosper::Buffer> m_vertexAnimationBuffer = nullptr;
	std::vector<std::vector<std::vector<uint32_t>>> m_frameIndices = {};
	void UpdateVertexAnimationBuffer();
};
#endif