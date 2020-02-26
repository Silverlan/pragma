#ifndef __RENDERSYSTEM_H__
#define __RENDERSYSTEM_H__

#include "pragma/clientdefinitions.h"
#include "pragma/opengl/renderobject.h"
#include "pragma/rendering/c_rendermode.h"
#include "pragma/rendering/c_entitymeshinfo.h"
#include "pragma/rendering/occlusion_culling/occlusion_culling_handler.hpp"
#include <vector>
#include <deque>

class DLLCLIENT RenderSystem;
//class DLLCLIENT RenderWrapper;
class Material;
class CBaseEntity;
/*
class DLLCLIENT RenderList
{
public:
	friend RenderWrapper;
	friend RenderSystem;
private:
	std::vector<RenderObject*> m_renderObjects;
	std::deque<unsigned int> m_freeIndices;
	RenderWrapper *m_renderWrapper;
	unsigned int m_index;
	Material *m_material;
	unsigned int m_objectCount;
protected:
	RenderList(RenderWrapper *wrapper,Material *mat,unsigned int idx);
	~RenderList();
	unsigned int GetIndex();
	void SetIndex(unsigned int idx);
	RenderWrapper *GetRenderWrapper();
	void SetRenderWrapper(RenderWrapper *wrapper);
	RenderObject *AddRenderObject(GLMesh *mesh);
	RenderObject *AddRenderObject(RenderObject *o,unsigned int idx=-1);
	void RemoveRenderObject(unsigned int idx,bool dontFree=false);
	void Render(Camera *cam,Shader3DTexturedBase *shader);
};

class DLLCLIENT RenderInstance
{
public:
	friend RenderSystem;
protected:
	RenderInstance(RenderMode renderMode);
	~RenderInstance();
	std::vector<RenderObjectHandle> m_renderObjects;
	bool m_bEnabled;
	RenderMode m_renderMode;
	bool (*m_renderCallback)(RenderObject*,CBaseEntity*,Camera*,Shader3DTexturedBase*,Material*);
	static bool RenderCallback(RenderObject *o,void *userData,Camera *cam,Shader3DTexturedBase *shader,Material *mat);
public:
	RenderObjectHandle CreateRenderObject(Shader3DTexturedBase *shader,Material *mat,GLMesh *mesh);
	void Remove();
	void Enable();
	void Disable();
	void SetEnabled(bool b);
	void *userData;
	void SetRenderCallback(bool (*callback)(RenderObject*,CBaseEntity*,Camera*,Shader3DTexturedBase*,Material*));
	RenderMode GetRenderMode();
	void SetRenderMode(RenderMode renderMode);
};

class DLLCLIENT RenderWrapper
{
public:
	friend RenderSystem;
	friend RenderList;
private:
	std::vector<RenderList*> m_renderLists;
	unsigned int m_index;
	Shader3DTexturedBase *m_shader;
	unsigned int m_listCount;
	RenderMode m_renderMode;
protected:
	RenderWrapper(RenderMode renderMode,Shader3DTexturedBase *shader,unsigned int idx);
	~RenderWrapper();
	unsigned int GetIndex();
	void SetIndex(unsigned int idx);
	RenderObject *AddRenderObject(Material *mat,GLMesh *mesh,RenderObject *o=NULL);
	void RemoveRenderList(unsigned int idx);
	void Render(Camera *cam);
};


struct DLLCLIENT EntityMeshContainer
{
	EntityMeshContainer(CBaseEntity *ent)
		: entity(ent)
	{}
	CBaseEntity *entity;
	std::vector<CModelSubMesh*> meshes;
};
*/

namespace prosper {class Shader; class PrimaryCommandBuffer;};
namespace pragma
{
	class CLightComponent; class CCameraComponent;
	namespace rendering {struct CulledMeshData;};
};
class CModelMesh;
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLCLIENT RenderSystem
{
public:
	struct DLLCLIENT MaterialMeshContainer
	{
		MaterialMeshContainer(Material *mat);
		MaterialMeshContainer(MaterialMeshContainer&)=delete;
		MaterialMeshContainer &operator=(const MaterialMeshContainer &other)=delete;
		Material *material;
		std::unordered_map<CBaseEntity*,EntityMeshInfo> containers;
	};
	struct DLLCLIENT TranslucentMesh
	{
		TranslucentMesh(CBaseEntity *ent,CModelSubMesh *mesh,Material *mat,::util::WeakHandle<prosper::Shader> shader,float distance);
		CBaseEntity *ent;
		CModelSubMesh *mesh;
		Material *material;
		::util::WeakHandle<prosper::Shader> shader = {};
		float distance;
	};
	enum class RenderFlags : uint8_t
	{
		None = 0u,
		Reflection = 1u,
		RenderAs3DSky = Reflection<<1u
	};
public:
	static void Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::CCameraComponent &cam,RenderMode renderMode,RenderFlags flags,std::vector<std::unique_ptr<RenderSystem::TranslucentMesh>> &translucentMeshes,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f});
	
	static uint32_t Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,RenderMode renderMode=RenderMode::World,RenderFlags flags=RenderFlags::None,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f});
	static uint32_t Render(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::CulledMeshData &renderMeshes,RenderMode renderMode=RenderMode::World,RenderFlags flags=RenderFlags::None,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f});

	static void RenderPrepass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,RenderMode renderMode=RenderMode::World);
	static void RenderPrepass(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,const pragma::rendering::CulledMeshData &renderMeshes);
	static void RenderShadows(std::shared_ptr<prosper::PrimaryCommandBuffer> &drawCmd,pragma::rendering::RasterizationRenderer &renderer,std::vector<pragma::CLightComponent*> &lights);
};
REGISTER_BASIC_BITWISE_OPERATORS(RenderSystem::RenderFlags)
#pragma warning(pop)

#endif
