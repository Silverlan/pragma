#include "stdafx_shared.h"
#include <pragma/definitions.h>
#include "pragma/model/model.h"
#include <pragma/engine.h>
#include "materialmanager.h"
#include "pragma/model/animation/activities.h"
#include <mathutil/umath.h>
#include "pragma/physics/collisionmesh.h"
#include "pragma/model/modelmesh.h"
#include <sharedutils/util_string.h>
#include "pragma/physics/physsoftbodyinfo.hpp"
#include "pragma/model/animation/vertex_animation.hpp"
#include "pragma/file_formats/wmd.h"
#include <sharedutils/util_file.h>

DEFINE_BASE_HANDLE(DLLNETWORK,Model,Model);

extern DLLENGINE Engine *engine;

std::shared_ptr<ModelMeshGroup> ModelMeshGroup::Create(const std::string &name)
{
	return std::shared_ptr<ModelMeshGroup>(new ModelMeshGroup{name});
}
std::shared_ptr<ModelMeshGroup> ModelMeshGroup::Create(const ModelMeshGroup &other)
{
	auto r = std::shared_ptr<ModelMeshGroup>(new ModelMeshGroup{other.m_name});
	r->m_meshes.reserve(other.m_meshes.size());
	for(auto &mesh : other.m_meshes)
		r->m_meshes.push_back(mesh->Copy());
	return r;
}
ModelMeshGroup::ModelMeshGroup(const std::string &name)
	: m_name(name)
{}
bool ModelMeshGroup::operator==(const ModelMeshGroup &other) const {return this == &other;}
bool ModelMeshGroup::operator!=(const ModelMeshGroup &other) const {return !operator==(other);}
const std::string &ModelMeshGroup::GetName() const {return m_name;}
std::vector<std::shared_ptr<ModelMesh>> &ModelMeshGroup::GetMeshes() {return m_meshes;}
void ModelMeshGroup::AddMesh(const std::shared_ptr<ModelMesh> &mesh) {m_meshes.push_back(mesh);}
uint32_t ModelMeshGroup::GetMeshCount() const {return static_cast<uint32_t>(m_meshes.size());}

/////////////////////////////////////

std::unordered_map<std::string,std::shared_ptr<Model>> Model::m_models;

Con::c_cout &operator<<(Con::c_cout &os,const Model& mdl)
{
	os<<"Model["<<mdl.m_name<<"]";
	return os;
}

Model::MetaInfo::MetaInfo()
	: flags(0)
{}

Model::Model()
	: m_handle(new PtrModel(this)),
	m_reference(Frame::Create(0))
{
	Construct();
}

Model::Model(NetworkState *nw,uint32_t numBones,const std::string &name)
	: Model()
{
	m_networkState = nw;
	m_name = name;
	for(auto i=decltype(numBones){0};i<numBones;++i)
		m_bindPose.push_back(umat::identity());
}

Model::Model(const Model &other)
	: m_handle(new PtrModel(this)),m_networkState(other.m_networkState),m_metaInfo(other.m_metaInfo),m_bValid(other.m_bValid),m_mass(other.m_mass),
	m_blendControllers(other.m_blendControllers),m_bodyGroups(other.m_bodyGroups),m_hitboxes(other.m_hitboxes),
	m_name(other.m_name),m_animationIDs(other.m_animationIDs),m_bindPose(other.m_bindPose),m_collisionMin(other.m_collisionMin),
	m_collisionMax(other.m_collisionMax),m_renderMin(other.m_renderMin),m_renderMax(other.m_renderMax),m_joints(other.m_joints),
	m_baseMeshes(other.m_baseMeshes),m_lods(other.m_lods),m_attachments(other.m_attachments),
	m_materials(other.m_materials),m_textureGroups(other.m_textureGroups),m_skeleton(std::make_unique<Skeleton>(*other.m_skeleton)),
	m_reference(Frame::Create(*other.m_reference)),m_vertexCount(other.m_vertexCount),m_triangleCount(other.m_triangleCount),
	m_bAllMaterialsLoaded(true),m_flexControllers(other.m_flexControllers),m_flexes(other.m_flexes),m_phonemeMap(other.m_phonemeMap)
{
	m_meshGroups.reserve(other.m_meshGroups.size());
	for(auto &meshGroup : other.m_meshGroups)
		m_meshGroups.push_back(ModelMeshGroup::Create(*meshGroup));
	m_animations.reserve(other.m_animations.size());
	for(auto &anim : other.m_animations)
		m_animations.push_back(Animation::Create(*anim));

	m_vertexAnimations.reserve(other.m_vertexAnimations.size());
	for(auto &anim : other.m_vertexAnimations)
		m_vertexAnimations.push_back(anim->Copy());

	m_collisionMeshes.reserve(other.m_collisionMeshes.size());
	for(auto &mesh : other.m_collisionMeshes)
		m_collisionMeshes.push_back(CollisionMesh::Create(*mesh));
}

Model::~Model()
{
	m_handle.Invalidate();
	for(auto &hCb : m_matLoadCallbacks)
	{
		if(hCb.IsValid() == true)
			hCb.Remove();
	}
	for(auto &hCb : m_onAllMatsLoadedCallbacks)
	{
		if(hCb.IsValid() == true)
			hCb.Remove();
	}
	m_joints.clear();
}

const PhonemeMap &Model::GetPhonemeMap() const {return const_cast<Model*>(this)->GetPhonemeMap();}
PhonemeMap &Model::GetPhonemeMap() {return m_phonemeMap;}

void Model::Rotate(const Quat &rot)
{
	uvec::rotate(&m_collisionMin,rot);
	uvec::rotate(&m_collisionMax,rot);
	uvec::rotate(&m_renderMin,rot);
	uvec::rotate(&m_renderMax,rot);
	auto &skeleton = GetSkeleton();
	for(auto &colMesh : m_collisionMeshes)
		colMesh->Rotate(rot);
	for(auto &anim : m_animations)
	{
		if(anim->HasFlag(FAnim::Gesture) == true)
			continue; // Don't rotate delta animations
		anim->Rotate(skeleton,rot);
	}
	for(auto &meshGroup : m_meshGroups)
	{
		for(auto &mesh : meshGroup->GetMeshes())
			mesh->Rotate(rot);
	}
	m_reference->Rotate(rot);
	GenerateBindPoseMatrices();
}

void Model::Translate(const Vector3 &t)
{
	m_collisionMin += t;
	m_collisionMax += t;
	m_renderMin += t;
	m_renderMax += t;
	auto &skeleton = GetSkeleton();
	for(auto &colMesh : m_collisionMeshes)
		colMesh->Translate(t);
	for(auto &anim : m_animations)
	{
		if(anim->HasFlag(FAnim::Gesture) == true)
			continue; // Don't rotate delta animations
		anim->Translate(skeleton,t);
	}
	for(auto &meshGroup : m_meshGroups)
	{
		for(auto &mesh : meshGroup->GetMeshes())
			mesh->Translate(t);
	}
	m_reference->Translate(t);
	GenerateBindPoseMatrices();
}

void Model::GenerateBindPoseMatrices()
{
	auto &bones = GetSkeleton().GetBones();
	for(auto i=decltype(bones.size()){0};i<bones.size();++i)
	{
		auto &pos = *m_reference->GetBonePosition(i);
		auto &rot = *m_reference->GetBoneOrientation(i);

		auto m = glm::toMat4(rot);
		m = glm::translate(m,pos);
		SetBindPoseBoneMatrix(i,m);
	}
}

uint32_t Model::GetVertexCount() const {return m_vertexCount;}
uint32_t Model::GetTriangleCount() const {return m_triangleCount;}
Model::MetaInfo &Model::GetMetaInfo() const {return m_metaInfo;}
void Model::GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups,std::vector<std::shared_ptr<ModelMesh>> &outMeshes) const {return GetBodyGroupMeshes(bodyGroups,0,outMeshes);}
void Model::GetBodyGroupMeshes(const std::vector<uint32_t> bodyGroups,uint32_t lod,std::vector<std::shared_ptr<ModelMesh>> &outMeshes) const
{
	auto meshIds = const_cast<Model*>(this)->GetBaseMeshes();
	meshIds.reserve(meshIds.size() +m_bodyGroups.size());
	for(auto i=decltype(bodyGroups.size()){0};i<bodyGroups.size();++i)
	{
		auto bg = bodyGroups[i];
		auto meshGroupId = std::numeric_limits<uint32_t>::max();
		if(const_cast<Model*>(this)->GetMesh(static_cast<uint32_t>(i),bg,meshGroupId) == true && meshGroupId != std::numeric_limits<uint32_t>::max())
			meshIds.push_back(meshGroupId);
	}
	//const_cast<Model*>(this)->TranslateLODMeshes(lod,meshIds);
	const_cast<Model*>(this)->GetMeshes(meshIds,outMeshes);
}
BodyGroup &Model::AddBodyGroup(const std::string &name)
{
	auto id = GetBodyGroupId(name);
	if(id == -1)
	{
		m_bodyGroups.push_back(BodyGroup(name));
		id = static_cast<int32_t>(m_bodyGroups.size() -1);
	}
	return m_bodyGroups[id];
}
int32_t Model::GetBodyGroupId(const std::string &name)
{
	for(auto i=decltype(m_bodyGroups.size()){0};i<m_bodyGroups.size();++i)
	{
		auto &bg = m_bodyGroups[i];
		if(bg.name == name)
			return static_cast<int32_t>(i);
	}
	return -1;
}
BodyGroup *Model::GetBodyGroup(uint32_t id)
{
	if(id >= m_bodyGroups.size())
		return nullptr;
	return &m_bodyGroups[id];
}
uint32_t Model::GetBodyGroupCount() const {return static_cast<uint32_t>(m_bodyGroups.size());}
std::vector<BodyGroup> &Model::GetBodyGroups() {return m_bodyGroups;}
void Model::Remove() {delete this;}
bool Model::GetLocalBonePosition(uint32_t animId,uint32_t frameId,uint32_t boneId,Vector3 &rPos,Quat &rRot,Vector3 *scale)
{
	rPos = Vector3{0.f,0.f,0.f};
	rRot = uquat::identity();
	if(scale != nullptr)
		*scale = {1.f,1.f,1.f};
	auto &skeleton = GetSkeleton();
	auto bone = skeleton.GetBone(boneId).lock();
	if(bone == nullptr)
		return false;
	auto anim = GetAnimation(animId);
	if(anim == nullptr)
		return false;
	auto frame = anim->GetFrame(frameId);
	if(frame == nullptr)
		return false;
	auto *pos = frame->GetBonePosition(boneId);
	auto *rot = frame->GetBoneOrientation(boneId);
	if(scale != nullptr)
	{
		auto *pScale = frame->GetBoneScale(boneId);
		if(pScale != nullptr)
			*scale = *pScale;
	}
	if(pos == nullptr)
		return false;
	rPos = *pos;
	rRot = *rot;
	auto parent = bone->parent.lock();
	while(parent != nullptr)
	{
		auto *posParent = frame->GetBonePosition(parent->ID);
		auto *rotParent = frame->GetBoneOrientation(parent->ID);
		uvec::rotate(&rPos,*rotParent);
		rPos += *posParent;
		rRot = *rotParent *rRot;
		parent = parent->parent.lock();
	}
	return true;
}
ModelHandle Model::GetHandle() {return m_handle;}

Frame &Model::GetReference() {return *m_reference;}
const Frame &Model::GetReference() const {return *m_reference;}
void Model::SetReference(std::shared_ptr<Frame> frame) {m_reference = frame;}
const std::vector<JointInfo> &Model::GetJoints() const {return const_cast<Model*>(this)->GetJoints();}
std::vector<JointInfo> &Model::GetJoints() {return m_joints;}
JointInfo &Model::AddJoint(uint8_t type,uint32_t src,uint32_t tgt)
{
	m_joints.push_back(JointInfo(type,src,tgt));
	return m_joints.back();
}

void Model::Construct()
{
	m_bValid = true;
	m_name = "";
	m_skeleton = std::make_unique<Skeleton>();
	m_mass = 0.f;
	uvec::zero(&m_collisionMin);
	uvec::zero(&m_collisionMax);
	uvec::zero(&m_renderMin);
	uvec::zero(&m_renderMax);
}

void Model::OnMaterialLoaded()
{
	auto bAllLoaded = true;
	for(auto &hMat : m_materials)
	{
		if(hMat.IsValid() && hMat->IsLoaded() == false)
		{
			bAllLoaded = false;
			break;
		}
	}
	for(auto it=m_matLoadCallbacks.begin();it!=m_matLoadCallbacks.end();)
	{
		if(!it->IsValid())
			it = m_matLoadCallbacks.erase(it);
		else
			++it;
	}
	m_bAllMaterialsLoaded = bAllLoaded;
	if(m_bAllMaterialsLoaded == true)
	{
		for(auto &hCb : m_onAllMatsLoadedCallbacks)
		{
			if(hCb.IsValid() == true)
				hCb();
		}
		m_onAllMatsLoadedCallbacks.clear();
	}
}
CallbackHandle Model::CallOnMaterialsLoaded(const std::function<void(void)> &f)
{
	if(m_bAllMaterialsLoaded == true)
	{
		f();
		return {};
	}
	m_onAllMatsLoadedCallbacks.push_back(FunctionCallback<>::Create(f));
	return m_onAllMatsLoadedCallbacks.back();
}
uint32_t Model::AddTexture(const std::string &tex,Material *mat)
{
	auto &meta = GetMetaInfo();
	auto it = std::find(meta.textures.begin(),meta.textures.end(),tex);
	if(it != meta.textures.end())
		return it -meta.textures.begin();
	meta.textures.push_back(tex);
	if(mat == nullptr)
		m_materials.push_back(MaterialHandle{});
	else
	{
		m_bAllMaterialsLoaded = false;
		m_materials.push_back(mat->GetHandle());
		auto cb = mat->CallOnLoaded([this]() {
			OnMaterialLoaded();
		});
		if(cb.IsValid() == true)
			m_matLoadCallbacks.push_back(cb);
	}
	return static_cast<uint32_t>(meta.textures.size() -1);
}
uint32_t Model::AddMaterial(uint32_t skin,Material *mat)
{
	auto texName = mat->GetName();
	AddTexturePath(ufile::get_path_from_filename(texName));
	texName = ufile::get_file_from_filename(texName);
	ufile::remove_extension_from_filename(texName);
	auto r = AddTexture(texName,mat);
	if(skin < m_textureGroups.size())
	{
		auto &textures = m_textureGroups.at(skin).textures;
		auto itTexId = std::find(textures.begin(),textures.end(),r);
		if(itTexId == textures.end())
			textures.push_back(r);
	}
	return r;
}
void Model::RemoveTexture(uint32_t idx)
{
	auto &meta = GetMetaInfo();
	if(idx < meta.textures.size())
		meta.textures.erase(meta.textures.begin() +idx);
	if(idx < m_materials.size())
		m_materials.erase(m_materials.begin() +idx);
	for(auto &group : m_textureGroups)
	{
		for(auto it=group.textures.begin();it!=group.textures.end();)
		{
			auto &texId = *it;
			if(texId < idx)
				++it;
			else if(texId == idx)
				it = group.textures.erase(it);
			else
			{
				--texId;
				++it;
			}
		}
	}
}
void Model::ClearTextures()
{
	auto &meta = GetMetaInfo();
	meta.textures.clear();
	m_materials.clear();
	for(auto &group : m_textureGroups)
		group.textures.clear();
}
void Model::PrecacheTextureGroup(const std::function<Material*(const std::string&,bool)> &loadMaterial,uint32_t i)
{
	if(i >= m_textureGroups.size())
		return;
	LoadMaterials({i},loadMaterial);
}
void Model::OnMaterialMissing(const std::string&) {}

bool Model::FindMaterial(const std::string &texture,std::string &matPath) const {return FindMaterial(texture,matPath,nullptr);}
bool Model::FindMaterial(const std::string &texture,std::string &matPath,const std::function<Material*(const std::string&,bool)> &loadMaterial) const
{
	auto &meta = GetMetaInfo();
	auto &texturePaths = meta.texturePaths;
	for(auto &path : texturePaths)
	{
		auto texPath = path +texture;
		if(FileManager::Exists("materials\\" +texPath +".wmi") || FileManager::Exists("materials\\" +texPath +".vmt"))
		{
			matPath = texPath;
			return true;
		}
	}
	static auto bSkipPort = false;
	if(bSkipPort == true || loadMaterial == nullptr || engine->ShouldMountExternalGameResources() == false)
		return false;
	// Material not found; Attempt to port
	for(auto &path : texturePaths)
	{
		auto texPath = path +texture;
		if(m_networkState->PortMaterial(texPath +".wmi",loadMaterial) == true)
		{
			bSkipPort = true;
			auto r = FindMaterial(texture,matPath,loadMaterial);
			bSkipPort = false;
			return r;
		}
	}
	return false;
}

void Model::PrecacheTexture(uint32_t texId,const std::function<Material*(const std::string&,bool)> &loadMaterial,bool bReload)
{
	// TODO: lodMaterial Parameter
	// TODO: Virtual (Clientside override)
	// TODO: Protected +public
	//PrecacheTextureGroup(std::bind(&ServerState::LoadMaterial,server,std::placeholders::_1,std::placeholders::_2),i);
	if(texId >= m_metaInfo.textures.size())
		return;
	if(texId >= m_materials.size())
		m_materials.resize(texId +1);
	auto &texture = m_metaInfo.textures.at(texId);
	std::string matPath;
	if(FindMaterial(texture,matPath,loadMaterial) == false)
	{
		OnMaterialMissing(texture);
		return;
	}
	if(!matPath.empty())
	{
		auto *mat = loadMaterial(matPath,bReload);
		if(mat != nullptr)
			m_materials.at(texId) = mat->GetHandle();
	}
}

void Model::PrecacheTexture(uint32_t texId,bool bReload)
{
	PrecacheTexture(texId,[](const std::string &matName,bool bReload) -> Material* {
		auto *nw = engine->GetServerNetworkState();
		return nw->LoadMaterial(matName,bReload);
	},bReload);
}

void Model::Optimize()
{
	auto &meshGroups = GetMeshGroups();
	for(auto &group : meshGroups)
	{
		// Group all sub-meshes for this mesh group by material
		std::unordered_map<uint32_t,std::vector<std::shared_ptr<ModelSubMesh>>> groupedMeshes;
		for(auto &mesh : group->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto texId = subMesh->GetTexture();
				auto it = groupedMeshes.find(texId);
				if(it == groupedMeshes.end())
					it = groupedMeshes.insert(decltype(groupedMeshes)::value_type(texId,{})).first;
				it->second.push_back(subMesh);
			}
		}

		// Merge all sub-meshes with same material, and clear (triangles) sub-meshes which aren't needed anymore
		for(auto &pair : groupedMeshes)
		{
			if(pair.second.size() == 1)
				continue;
			auto &meshFirst = pair.second.front();
			for(auto it=pair.second.begin() +1;it!=pair.second.end();++it)
			{
				auto &meshOther = *it;
				meshFirst->Merge(*meshOther);
				meshOther->ClearTriangles();
			}
		}

		// Remove obsolete sub-meshes from meshes
		auto &meshes = group->GetMeshes();
		for(auto it=meshes.begin();it!=meshes.end();)
		{
			auto &mesh = *it;
			auto &subMeshes = mesh->GetSubMeshes();
			for(auto it=subMeshes.begin();it!=subMeshes.end();)
			{
				auto &subMesh = *it;
				if(subMesh->GetTriangleCount() > 0)
				{
					++it;
					continue;
				}
				it = subMeshes.erase(it);
			}
			if(subMeshes.empty() == true)
				it = meshes.erase(it);
			else
				++it;
		}
	}
}

void Model::LoadMaterials(const std::vector<uint32_t> &textureGroupIds,const std::function<Material*(const std::string&,bool)> &loadMaterial,bool bReload)
{
	auto &meta = GetMetaInfo();
	auto &textures = meta.textures;
	//m_materials.clear();
	m_materials.resize(textures.size());
	std::vector<std::string> materialPaths;
	materialPaths.resize(textures.size());

	auto &textureGroups = GetTextureGroups();
	for(auto i=decltype(textureGroupIds.size()){0};i<textureGroupIds.size();++i)
	{
		auto &group = textureGroups[textureGroupIds[i]];
		for(auto texId : group.textures)
		{
			auto &texture = textures[texId];
			if(FindMaterial(texture,materialPaths[texId],loadMaterial) == false)
				OnMaterialMissing(texture);
		}
	}
	for(auto i=decltype(materialPaths.size()){0};i<materialPaths.size();++i)
	{
		auto &matPath = materialPaths[i];
		if(!matPath.empty())
		{
			auto *mat = loadMaterial(matPath,bReload);
			if(mat != nullptr)
				m_materials[i] = mat->GetHandle();
		}
	}
}

void Model::LoadMaterials(const std::function<Material*(const std::string&,bool)> &loadMaterial,bool bReload)
{
	auto &meta = GetMetaInfo();
	auto bDontPrecacheTexGroups = ((meta.flags &FWMD_DONT_PRECACHE_TEXTURE_GROUPS) == FWMD_DONT_PRECACHE_TEXTURE_GROUPS) ? true : false;
	std::vector<uint32_t> groupIds;
	auto &texGroups = GetTextureGroups();
	groupIds.reserve(texGroups.size());
	for(auto i=decltype(texGroups.size()){0};i<((bDontPrecacheTexGroups == true) ? 1 : texGroups.size());++i)
		groupIds.push_back(static_cast<uint32_t>(i));
	LoadMaterials(groupIds,loadMaterial,bReload);
}
TextureGroup *Model::CreateTextureGroup()
{
	m_textureGroups.push_back(TextureGroup{});
	auto &skin = m_textureGroups.back();
	//auto &meta = GetMetaInfo();
	//for(auto i=decltype(meta.textures.size()){0};i<meta.textures.size();++i)
	//	skin.textures.push_back(static_cast<uint32_t>(i)); // Only base-textures; See FWMD::Load
	return &skin;
}
void Model::PrecacheTextureGroup(uint32_t i)
{
	PrecacheTextureGroup([](const std::string &matName,bool bReload) -> Material* {
		auto *nw = engine->GetServerNetworkState();
		return nw->LoadMaterial(matName,bReload);
	},i);
}
void Model::PrecacheTextureGroups()
{
	for(auto i=decltype(m_textureGroups.size()){0};i<m_textureGroups.size();++i)
		PrecacheTextureGroup(static_cast<uint32_t>(i));
}
std::vector<std::string> &Model::GetTexturePaths() {return m_metaInfo.texturePaths;}
void Model::AddTexturePath(const std::string &path)
{
	if(path.empty())
		return;
	auto npath = path;
	if(npath.back() != '/' && npath.back() != '\\')
		npath += '\\';
	npath = FileManager::GetCanonicalizedPath(npath);
	if(npath.empty())
		return;
	auto it = std::find_if(m_metaInfo.texturePaths.begin(),m_metaInfo.texturePaths.end(),[&npath](const std::string &pathOther) {
		return ustring::compare(npath,pathOther,false);
	});
	if(it != m_metaInfo.texturePaths.end())
		return;
	m_metaInfo.texturePaths.push_back(npath);
}
void Model::RemoveTexturePath(uint32_t idx)
{
	if(idx >= m_metaInfo.texturePaths.size())
		return;
	m_metaInfo.texturePaths.erase(m_metaInfo.texturePaths.begin() +idx);
}
void Model::SetTexturePaths(const std::vector<std::string> &paths)
{
	m_metaInfo.texturePaths = paths;
}
std::vector<MaterialHandle> &Model::GetMaterials() {return m_materials;}
const std::vector<MaterialHandle> &Model::GetMaterials() const {return m_materials;}
std::vector<std::string> &Model::GetTextures() {return m_metaInfo.textures;}
std::vector<TextureGroup> &Model::GetTextureGroups() {return m_textureGroups;}
void Model::SetMaterial(uint32_t texId,Material *mat)
{
	if(texId >= m_materials.size())
		return;
	m_materials.at(texId) = mat;
	if(texId < m_metaInfo.textures.size())
		m_metaInfo.textures.at(texId) = mat->GetName();
}
Material *Model::GetMaterial(uint32_t texID)
{
	if(texID >= m_materials.size())
		return nullptr;
	return m_materials[texID].get();
}
Material *Model::GetMaterial(uint32_t texGroup,uint32_t texID)
{
	if(m_textureGroups.empty())
		return nullptr;
	if(texGroup >= m_textureGroups.size())
		texGroup = 0;
	auto &skin = m_textureGroups[texGroup];
	if(texID >= skin.textures.size())
	{
		if(texGroup == 0)
			return nullptr;
		return GetMaterial(0,texID);
	}
	texID = skin.textures[texID];
	if(texID >= m_materials.size())
		return nullptr;
	return m_materials[texID].get();
}
TextureGroup *Model::GetTextureGroup(uint32_t i)
{
	if(i >= m_textureGroups.size())
		return nullptr;
	return &m_textureGroups[i];
}

uint32_t Model::GetLODCount() const {return static_cast<uint32_t>(m_lods.size());}
uint32_t Model::GetLOD(uint32_t id) const
{
	if(id >= m_lods.size())
		return 0;
	return m_lods[id].lod;
}

LODInfo *Model::AddLODInfo(uint32_t lod,std::unordered_map<uint32_t,uint32_t> &replaceIds)
{
	LODInfo *info = nullptr;
	auto itLod = std::find_if(m_lods.begin(),m_lods.end(),[lod](const LODInfo &lodInfo) {
		return lod == lodInfo.lod;
	});
	if(itLod != m_lods.end())
		info = &(*itLod);
	else
	{
		itLod = std::find_if(m_lods.begin(),m_lods.end(),[lod](const LODInfo &lodInfo) {
			return lod < lodInfo.lod;
		});
		itLod = m_lods.insert(itLod,LODInfo{});
		info = &(*itLod);
	}
	info->lod = lod;
	for(auto &pair : replaceIds)
		info->meshReplacements.insert(pair);
	if(m_lods.size() > 1)
	{
		auto posInserted = itLod -m_lods.begin();
		// Copy all replacement meshes from lower LODs to this one (Unless this one already replaces the same mesh)
		if(posInserted > 0)
		{
			auto &prev = m_lods.at(posInserted -1);
			for(auto it=prev.meshReplacements.begin();it!=prev.meshReplacements.end();++it)
			{
				auto itCur = info->meshReplacements.find(it->first);
				if(itCur == info->meshReplacements.end())
					info->meshReplacements[it->first] = it->second;
			}
		}
		//
		// If this LOD has been placed somewhere in the middle, add all the replacement meshes to higher LODs as well
		for(auto i=(posInserted +1);i<m_lods.size();i++)
		{
			auto &next = m_lods.at(i);
			for(auto it=next.meshReplacements.begin();it!=next.meshReplacements.end();++it)
			{
				auto itCur = info->meshReplacements.find(it->first);
				if(itCur == info->meshReplacements.end())
					info->meshReplacements[it->first] = it->second;
			}
		}
		//
	}
	return info;
}

LODInfo *Model::GetLODInfo(uint32_t lod)
{
	if(m_lods.empty() || lod == 0)
		return nullptr;
	for(auto i=decltype(m_lods.size()){0};i<m_lods.size();++i)
	{
		auto &info = m_lods[i];
		if(lod == info.lod)
			return &info;
		else if(info.lod > lod)
			return (i == 0) ? nullptr : &m_lods[i -1];
	}
	return &m_lods.back();
}

const std::vector<LODInfo> &Model::GetLODs() const {return m_lods;}

bool Model::TranslateLODMeshes(uint32_t lod,std::vector<uint32_t> &meshIds)
{
	auto *lodInfo = GetLODInfo(lod);
	if(lodInfo != nullptr)
	{
		for(auto it=lodInfo->meshReplacements.begin();it!=lodInfo->meshReplacements.end();++it)
		{
			for(auto itMesh=meshIds.begin();itMesh!=meshIds.end();)
			{
				if(*itMesh == it->first)
				{
					if(it->second == MODEL_NO_MESH)
					{
						itMesh = meshIds.erase(itMesh);
						break;
					}
					else
					{
						*itMesh = it->second;
						break;
					}
				}
				++itMesh;
			}
		}
	}
	return (lodInfo != nullptr) ? true : false;
}

const std::vector<uint32_t> &Model::GetBaseMeshes() const {return const_cast<Model*>(this)->GetBaseMeshes();}
std::vector<uint32_t> &Model::GetBaseMeshes() {return m_baseMeshes;}
uint32_t Model::GetMeshGroupCount() const {return static_cast<uint32_t>(m_meshGroups.size());}
uint32_t Model::GetMeshCount() const {return m_meshCount;}
uint32_t Model::GetSubMeshCount() const {return m_subMeshCount;}
uint32_t Model::GetCollisionMeshCount() const {return static_cast<uint32_t>(m_collisionMeshes.size());}
Bool Model::GetMesh(uint32_t bodyGroupId,uint32_t groupId,uint32_t &outMeshId)
{
	auto *bodyGroup = GetBodyGroup(bodyGroupId);
	if(bodyGroup == nullptr)
		return false;
	if(groupId >= bodyGroup->meshGroups.size())
		return false;
	outMeshId = bodyGroup->meshGroups[groupId];
	return true;
}
void Model::GetMeshes(const std::vector<uint32_t> &meshIds,std::vector<std::shared_ptr<ModelMesh>> &outMeshes)
{
	auto numGroups = m_meshGroups.size();
	for(auto meshId : meshIds)
	{
		if(meshId < numGroups)
		{
			auto &group = m_meshGroups[meshId];
			auto &groupMeshes = group->GetMeshes();
			outMeshes.reserve(outMeshes.size() +groupMeshes.size());
			for(auto it=groupMeshes.begin();it!=groupMeshes.end();++it)
				outMeshes.push_back(*it);
		}
	}
}
void Model::SetCollisionBounds(const Vector3 &min,const Vector3 &max)
{
	m_collisionMin = min;
	m_collisionMax = max;
}
void Model::SetRenderBounds(const Vector3 &min,const Vector3 &max)
{
	m_renderMin = min;
	m_renderMax = max;
}

void Model::CalculateRenderBounds()
{
	Vector3 min(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
	Vector3 max(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
	auto numMeshGroups = m_meshGroups.size();
	if(numMeshGroups == 0)
	{
		min = Vector3{0.f,0.f,0.f};
		max = Vector3{0.f,0.f,0.f};
	}
	else
	{
		for(auto &group : m_meshGroups)
		{
			auto &meshes = group->GetMeshes();
			for(auto &mesh : meshes)
			{
				Vector3 meshMin;
				Vector3 meshMax;
				mesh->GetBounds(meshMin,meshMax);
				uvec::min(&min,meshMin);
				uvec::max(&max,meshMax);
			}
		}
	}
	m_renderMin = min;
	m_renderMax = max;
}

Vector3 Model::GetOrigin() const
{
	if(m_collisionMeshes.empty() || m_collisionMeshes.front() == nullptr)
		return {};
	return m_collisionMeshes.front()->GetOrigin();
}

const Vector3 &Model::GetEyeOffset() const {return m_eyeOffset;}
void Model::SetEyeOffset(const Vector3 &offset) {m_eyeOffset = offset;}

void Model::CalculateCollisionBounds()
{
	Vector3 min(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
	Vector3 max(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
	auto numMeshes = m_collisionMeshes.size();
	if(numMeshes == 0)
	{
		min = Vector3{0.f,0.f,0.f};
		max = Vector3{0.f,0.f,0.f};
	}
	else
	{
		for(auto &colMesh : m_collisionMeshes)
		{
			Vector3 colMin;
			Vector3 colMax;
			colMesh->GetAABB(&colMin,&colMax);
			uvec::min(&min,colMin);
			uvec::max(&max,colMax);
		}
	}
	m_collisionMin = min;
	m_collisionMax = max;
}

void Model::Update(ModelUpdateFlags flags)
{
	if((flags &ModelUpdateFlags::UpdateChildren) != ModelUpdateFlags::None)
	{
		for(auto &group : m_meshGroups)
		{
			auto &meshes = group->GetMeshes();
			for(auto &mesh : meshes)
				mesh->Update(flags);
		}
		for(auto &colMesh : m_collisionMeshes)
			colMesh->Update(flags);
	}
	if((flags &ModelUpdateFlags::UpdateBounds) != ModelUpdateFlags::None)
	{
		CalculateCollisionBounds();
		CalculateRenderBounds();
	}
	if((flags &ModelUpdateFlags::UpdatePrimitiveCounts) != ModelUpdateFlags::None)
	{
		m_meshCount = 0;
		m_subMeshCount = 0;
		m_vertexCount = 0;
		m_triangleCount = 0;
		for(auto &group : m_meshGroups)
		{
			auto &meshes = group->GetMeshes();
			m_meshCount += static_cast<uint32_t>(meshes.size());
			for(auto &mesh : meshes)
			{
				m_subMeshCount += mesh->GetSubMeshCount();
				m_vertexCount += mesh->GetVertexCount();
				m_triangleCount += mesh->GetTriangleCount();
			}
		}
	}
}

void Model::GetCollisionBounds(Vector3 &min,Vector3 &max)
{
	min = m_collisionMin;
	max = m_collisionMax;
}

void Model::GetRenderBounds(Vector3 &min,Vector3 &max)
{
	min = m_renderMin;
	max = m_renderMax;
}

bool Model::IntersectAABB(Vector3 &min,Vector3 &max)
{
	if(!Intersection::AABBAABB(m_collisionMin,m_collisionMax,min,max))
		return false;
	for(int i=0;i<m_collisionMeshes.size();i++)
		if(m_collisionMeshes[i]->IntersectAABB(&min,&max))
			return true;
	return false;
}

void Model::ClearCache()
{
	m_models.clear();
}

const std::string &Model::GetName() const {return m_name;}

bool Model::IsValid() const {return m_bValid;}

void Model::AddMesh(const std::string &meshGroup,const std::shared_ptr<ModelMesh> &mesh)
{
	auto group = AddMeshGroup(meshGroup);
	group->AddMesh(mesh);
}

std::vector<std::shared_ptr<ModelMesh>> *Model::GetMeshes(const std::string &meshGroup)
{
	auto group = GetMeshGroup(meshGroup);
	if(group == nullptr)
		return nullptr;
	return &group->GetMeshes();
}

std::vector<std::shared_ptr<ModelMeshGroup>> &Model::GetMeshGroups() {return m_meshGroups;}
const std::vector<std::shared_ptr<ModelMeshGroup>> &Model::GetMeshGroups() const {return m_meshGroups;}
std::shared_ptr<ModelMeshGroup> Model::GetMeshGroup(const std::string &meshGroup)
{
	auto it = std::find_if(m_meshGroups.begin(),m_meshGroups.end(),[&meshGroup](std::shared_ptr<ModelMeshGroup> &group) {
		return ustring::compare(group->GetName(),meshGroup,false);
	});
	if(it == m_meshGroups.end())
		return nullptr;
	return *it;
}
std::shared_ptr<ModelMeshGroup> Model::GetMeshGroup(uint32_t groupId)
{
	if(groupId >= m_meshGroups.size())
		return nullptr;
	return m_meshGroups[groupId];
}

bool Model::GetMeshGroupId(const std::string &meshGroup,uint32_t &groupId) const
{
	auto it = std::find_if(m_meshGroups.begin(),m_meshGroups.end(),[&meshGroup](const std::shared_ptr<ModelMeshGroup> &group) {
		return ustring::compare(group->GetName(),meshGroup,false);
	});
	if(it == m_meshGroups.end())
		return false;
	groupId = it -m_meshGroups.begin();
	return true;
}

std::shared_ptr<ModelMeshGroup> Model::AddMeshGroup(const std::string &meshGroup,uint32_t &meshGroupId)
{
	auto it = std::find_if(m_meshGroups.begin(),m_meshGroups.end(),[&meshGroup](const std::shared_ptr<ModelMeshGroup> &group) {
		return ustring::compare(group->GetName(),meshGroup,false);
	});
	if(it == m_meshGroups.end())
	{
		auto mg = ModelMeshGroup::Create(meshGroup);
		AddMeshGroup(mg);
		it = m_meshGroups.end() -1;
	}
	meshGroupId = it -m_meshGroups.begin();
	return *it;
}

std::shared_ptr<ModelMeshGroup> Model::AddMeshGroup(const std::string &meshGroup)
{
	uint32_t meshGroupId = 0u;
	return AddMeshGroup(meshGroup,meshGroupId);
}
void Model::AddMeshGroup(std::shared_ptr<ModelMeshGroup> &meshGroup) {m_meshGroups.push_back(meshGroup);}

std::vector<std::shared_ptr<CollisionMesh>> &Model::GetCollisionMeshes() {return m_collisionMeshes;}
const std::vector<std::shared_ptr<CollisionMesh>> &Model::GetCollisionMeshes() const {return m_collisionMeshes;}

void Model::AddCollisionMesh(const std::shared_ptr<CollisionMesh> &mesh) {m_collisionMeshes.push_back(mesh);}

uint32_t Model::AddAnimation(const std::string &name,const std::shared_ptr<Animation> &anim)
{
	auto lname = name;
	ustring::to_lower(lname);
	auto it = m_animationIDs.find(lname);
	if(it != m_animationIDs.end())
	{
		m_animations.at(it->second) = anim;
		return it->second;
	}
	m_animations.push_back(anim);
	return m_animationIDs[name] = static_cast<uint32_t>(m_animations.size() -1);
}

int32_t Model::LookupAnimation(const std::string &name) const
{
	auto it = m_animationIDs.find(name);
	if(it == m_animationIDs.end())
		return -1;
	return it->second;
}

void Model::GetAnimations(std::unordered_map<std::string,uint32_t> **anims) {*anims = &m_animationIDs;}

const Skeleton &Model::GetSkeleton() const {return *m_skeleton;}
Skeleton &Model::GetSkeleton() {return *m_skeleton;}

std::shared_ptr<Animation> Model::GetAnimation(uint32_t ID)
{
	if(ID >= m_animations.size())
		return nullptr;
	return m_animations[ID];
}
uint32_t Model::GetAnimationCount() const {return static_cast<uint32_t>(m_animations.size());}
const std::vector<std::shared_ptr<Animation>> &Model::GetAnimations() const {return const_cast<Model*>(this)->GetAnimations();}
std::vector<std::shared_ptr<Animation>> &Model::GetAnimations() {return m_animations;}
bool Model::GetAnimationName(uint32_t animId,std::string &name) const
{
	auto it = std::find_if(m_animationIDs.begin(),m_animationIDs.end(),[animId](const std::pair<std::string,uint32_t> &pair) {
		return (pair.second == animId) ? true : false;
	});
	if(it == m_animationIDs.end())
	{
		name = "";
		return false;
	}
	name = it->first;
	return true;
}
std::string Model::GetAnimationName(uint32_t animId) const
{
	std::string r;
	GetAnimationName(animId,r);
	return r;	
}

const std::vector<Attachment> &Model::GetAttachments() const {return const_cast<Model*>(this)->GetAttachments();}
std::vector<Attachment> &Model::GetAttachments() {return m_attachments;}

void Model::AddAttachment(const std::string &name,uint32_t boneID,Vector3 offset,EulerAngles angles)
{
	auto lname = name;
	ustring::to_lower(lname);
	m_attachments.push_back(Attachment{});
	auto &att = m_attachments.back();
	att.name = lname;
	att.bone = boneID;
	att.offset = offset;
	att.angles = angles;
}
Attachment *Model::GetAttachment(uint32_t attachmentID)
{
	if(attachmentID >= m_attachments.size())
		return nullptr;
	return &m_attachments[attachmentID];
}
Attachment *Model::GetAttachment(const std::string &name)
{
	auto id = LookupAttachment(name);
	return GetAttachment(id);
}
void Model::RemoveAttachment(const std::string &name)
{
	auto attId = LookupAttachment(name);
	if(attId < 0)
		return;
	RemoveAttachment(attId);
}
void Model::RemoveAttachment(uint32_t idx)
{
	if(idx >= m_attachments.size())
		return;
	m_attachments.erase(m_attachments.begin() +idx);
}
int32_t Model::LookupBone(const std::string &name) const
{
	auto &skeleton = GetSkeleton();
	return skeleton.LookupBone(name);
}
int32_t Model::LookupAttachment(const std::string &name)
{
	auto lname = name;
	ustring::to_lower(lname);
	for(auto i=decltype(m_attachments.size()){0};i<m_attachments.size();++i)
	{
		auto &att = m_attachments[i];
		if(att.name == lname)
			return static_cast<int32_t>(i);
	}
	return -1;
}

uint32_t Model::GetBoneCount() const {return m_skeleton->GetBoneCount();}

Mat4 *Model::GetBindPoseBoneMatrix(uint32_t boneID)
{
	if(boneID >= m_bindPose.size())
		return nullptr;
	return &m_bindPose[boneID];
}

void Model::SetBindPoseBoneMatrix(uint32_t boneID,Mat4 mat)
{
	if(boneID >= m_bindPose.size())
	{
		auto numBones = m_skeleton->GetBoneCount();
		if(numBones > m_bindPose.size() && boneID < numBones)
			m_bindPose.resize(numBones);
		else
			return;
	}
	m_bindPose[boneID] = mat;
}

void Model::SetMass(float mass) {m_mass = mass;}
float Model::GetMass() const {return m_mass;}
uint8_t Model::GetAnimationActivityWeight(uint32_t animation) const
{
	if(animation >= m_animations.size())
		return 0;
	return m_animations[animation]->GetActivityWeight();
}
Activity Model::GetAnimationActivity(uint32_t animation) const
{
	if(animation >= m_animations.size())
		return Activity::Invalid;
	return m_animations[animation]->GetActivity();
}
float Model::GetAnimationDuration(uint32_t animation)
{
	if(animation >= m_animations.size())
		return 0.f;
	return m_animations[animation]->GetDuration();
}
int Model::SelectFirstAnimation(Activity activity) const
{
	auto it = std::find_if(m_animations.begin(),m_animations.end(),[activity](const std::shared_ptr<Animation> &anim) {
		return anim->GetActivity() == activity;
	});
	if(it == m_animations.end())
		return -1;
	return it -m_animations.begin();
}
int32_t Model::SelectWeightedAnimation(Activity activity,int32_t animIgnore)
{
	std::vector<int32_t> animations;
	std::vector<uint8_t> weights;
	uint32_t weightSum = 0;
	auto bIgnoreIsCandidate = false;
	for(auto i=decltype(m_animations.size()){0};i<m_animations.size();++i)
	{
		auto &anim = m_animations[i];
		if(i != animIgnore || !anim->HasFlag(FAnim::NoRepeat))
		{
			if(anim->GetActivity() == activity)
			{
				weightSum += anim->GetActivityWeight();
				animations.push_back(static_cast<int32_t>(i));
				weights.push_back(anim->GetActivityWeight());
			}
		}
		else
			bIgnoreIsCandidate = true;
	}
	if(animations.empty())
	{
		if(bIgnoreIsCandidate == true)
			return animIgnore;
		return -1;
	}
	if(animations.size() == 1 || weightSum == 0)
		return animations.front();
	int r = umath::random(0,weightSum -1);
	for(auto animId : animations)
	{
		auto &anim = m_animations[animId];
		auto weight = anim->GetActivityWeight();
		if(r < weight)
			return animId;
		r -= weight;
	}
	return -1;
}
void Model::GetAnimations(Activity activity,std::vector<uint32_t> &animations)
{
	for(auto i=decltype(m_animations.size()){0};i<m_animations.size();++i)
	{
		auto &anim = m_animations[i];
		if(anim->GetActivity() == activity)
			animations.push_back(static_cast<int32_t>(i));
	}
}
void Model::AddBlendController(const std::string &name,int32_t min,int32_t max,bool loop)
{
	auto lname = name;
	ustring::to_lower(lname);
	m_blendControllers.push_back(BlendController{});
	auto &blend = m_blendControllers.back();
	blend.name = name;
	blend.min = min;
	blend.max = max;
	blend.loop = loop;
}
const BlendController *Model::GetBlendController(uint32_t id) const {return const_cast<Model*>(this)->GetBlendController(id);}
BlendController *Model::GetBlendController(uint32_t id)
{
	if(id >= m_blendControllers.size())
		return nullptr;
	return &m_blendControllers[id];
}
const BlendController *Model::GetBlendController(const std::string &name) const {return const_cast<Model*>(this)->GetBlendController(name);}
BlendController *Model::GetBlendController(const std::string &name)
{
	auto it = std::find_if(m_blendControllers.begin(),m_blendControllers.end(),[&name](BlendController &bc) {
		return (bc.name == name) ? true : false;
	});
	if(it == m_blendControllers.end())
		return nullptr;
	return &(*it);
}
int32_t Model::LookupBlendController(const std::string &name)
{
	for(auto i=decltype(m_blendControllers.size()){0};i<m_blendControllers.size();++i)
	{
		if(m_blendControllers[i].name == name)
			return static_cast<int32_t>(i);
	}
	return -1;
}
const std::vector<BlendController> &Model::GetBlendControllers() const {return const_cast<Model*>(this)->GetBlendControllers();}
std::vector<BlendController> &Model::GetBlendControllers() {return m_blendControllers;}

void Model::UpdateShape(const std::vector<SurfaceMaterial>*)
{
	for(auto &cmesh : m_collisionMeshes)
		cmesh->UpdateShape();
}

//void Model::GetWeights(std::vector<VertexWeight*> **weights) {*weights = &m_weights;}
