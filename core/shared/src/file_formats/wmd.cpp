#include "stdafx_shared.h"
#include "pragma/file_formats/wmd.h"

FWMD::FWMD(Game *game)
	: m_bStatic(false),m_gameState(game)
{}

FWMD::~FWMD()
{
	for(int i=0;i<m_meshes.size();i++)
		delete m_meshes[i];
}

void FWMD::ReadChildBones(const Skeleton &skeleton,std::shared_ptr<Bone> bone)
{
	unsigned int numChildren = Read<unsigned int>();
	for(unsigned int i=0;i<numChildren;i++)
	{
		unsigned int boneID = Read<unsigned int>();
		auto child = skeleton.GetBone(boneID).lock();
		bone->children[boneID] = child;
		child->parent = bone;
		ReadChildBones(skeleton,child);
	}
}

void FWMD::GetMeshes(std::vector<FWMDMesh*> **meshes) {*meshes = &m_meshes;}