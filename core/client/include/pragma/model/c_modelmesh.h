#ifndef __C_MODELMESH_H__
#define __C_MODELMESH_H__
#include "pragma/clientdefinitions.h"
#include "pragma/model/c_normalmesh.h"
#include <pragma/model/modelmesh.h>
#include <memory>

namespace pragma {class VkMesh;};
class DLLCLIENT CModelSubMesh
	: public ModelSubMesh,public NormalMesh
{
public:
	CModelSubMesh();
	CModelSubMesh(const CModelSubMesh &other);
	const std::shared_ptr<pragma::VkMesh> &GetVKMesh() const;
	virtual void Update(ModelUpdateFlags flags=ModelUpdateFlags::AllData) override;
	virtual void Centralize(const Vector3 &origin) override;
	virtual std::shared_ptr<ModelSubMesh> Copy() const override;
	static void InitializeBuffers();
	static void ClearBuffers();
private:
	std::shared_ptr<pragma::VkMesh> m_vkMesh;
	//static Vulkan::Buffer s_vertexBuffer; // prosper TODO
	//static Vulkan::Buffer s_indexBuffer; // prosper TODO
	void UpdateVertexBuffer();
};

class DLLCLIENT CModelMesh
	: public ModelMesh
{
private:
	virtual void AddSubMesh(const std::shared_ptr<ModelSubMesh> &subMesh) override;
public:
	CModelMesh();
	void AddSubMesh(const std::shared_ptr<CModelSubMesh> &subMesh);
	virtual std::shared_ptr<ModelMesh> Copy() const override;
};

#endif