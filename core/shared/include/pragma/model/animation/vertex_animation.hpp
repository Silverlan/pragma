#ifndef __VERTEX_ANIMATION_HPP__
#define __VERTEX_ANIMATION_HPP__

#include "pragma/networkdefinitions.h"
#include <mathutil/uvec.h>
#include <memory>

class DLLNETWORK MeshVertexFrame
	: public std::enable_shared_from_this<MeshVertexFrame>
{
public:
	MeshVertexFrame()=default;
	MeshVertexFrame(const MeshVertexFrame &other);

	const std::vector<std::array<uint16_t,3>> &GetVertices() const;
	std::vector<std::array<uint16_t,3>> &GetVertices();
	void SetVertexCount(uint32_t count);
	void SetVertexPosition(uint32_t vertId,const Vector3 &pos);
	void SetVertexPosition(uint32_t vertId,const std::array<uint16_t,3> &pos);
	bool GetVertexPosition(uint32_t vertId,Vector3 &pos) const;
private:
	// Each uint16_t is a half-float
	std::vector<std::array<uint16_t,3>> m_vertices = {};
};

class DLLNETWORK MeshVertexAnimation
	: public std::enable_shared_from_this<MeshVertexAnimation>
{
public:
	MeshVertexAnimation()=default;
	MeshVertexAnimation(const MeshVertexAnimation &other);

	const MeshVertexFrame *GetFrame(uint32_t frameId) const;
	MeshVertexFrame *GetFrame(uint32_t frameId);

	const std::vector<std::shared_ptr<MeshVertexFrame>> &GetFrames() const;
	std::vector<std::shared_ptr<MeshVertexFrame>> &GetFrames();

	std::shared_ptr<MeshVertexFrame> AddFrame();

	ModelMesh *GetMesh() const;
	ModelSubMesh *GetSubMesh() const;
	void SetMesh(ModelMesh &mesh,ModelSubMesh &subMesh);
private:
	mutable std::weak_ptr<ModelMesh> m_wpMesh = {};
	mutable std::weak_ptr<ModelSubMesh> m_wpSubMesh = {};
	std::vector<std::shared_ptr<MeshVertexFrame>> m_frames;
};

class DLLNETWORK VertexAnimation
	: public std::enable_shared_from_this<VertexAnimation>
{
public:
	VertexAnimation()=default;
	VertexAnimation(const VertexAnimation &other);
	VertexAnimation(const std::string &name);
	virtual std::unique_ptr<VertexAnimation> Copy() const;
	virtual ~VertexAnimation()=default;

	std::shared_ptr<MeshVertexFrame> AddMeshFrame(ModelMesh &mesh,ModelSubMesh &subMesh);
	const std::vector<std::shared_ptr<MeshVertexAnimation>> &GetMeshAnimations() const;
	std::vector<std::shared_ptr<MeshVertexAnimation>> &GetMeshAnimations();

	bool GetMeshAnimationId(ModelSubMesh &subMesh,uint32_t &id) const;
	const MeshVertexAnimation *GetMeshAnimation(ModelSubMesh &subMesh) const;
	MeshVertexAnimation *GetMeshAnimation(ModelSubMesh &subMesh);

	const MeshVertexFrame *GetMeshFrame(ModelSubMesh &subMesh,uint32_t frameId) const;
	MeshVertexFrame *GetMeshFrame(ModelSubMesh &subMesh,uint32_t frameId);

	void SetName(const std::string &name);
	const std::string &GetName() const;
protected:
	std::string m_name;
	std::vector<std::shared_ptr<MeshVertexAnimation>> m_meshAnims;
};

#endif
