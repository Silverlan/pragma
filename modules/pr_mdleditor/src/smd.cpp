#include "stdafx_mdleditor.h"
#include "smd.h"
#include "util.h"
#include <pragma/networkstate/networkstate.h>
#include <pragma/model/model.h>
#include <pragma/model/modelmesh.h>
#include <pragma/physics/collisionmesh.h>
#include <algorithm>
#include <functional>
#include <array>

static uint32_t add_bone(const SMDModel::Node &node,Skeleton &skeleton,std::shared_ptr<Bone> *bone=nullptr)
{
	Model mdl;
	auto &bones = skeleton.GetBones();
	auto it = std::find_if(bones.begin(),bones.end(),[&node](const std::shared_ptr<Bone> &bone) {
		return (node.name == bone->name) ? true : false;
	});
	if(it == bones.end())
	{
		auto *ptrBone = new Bone;
		ptrBone->name = node.name;
		skeleton.AddBone(ptrBone);
		it = bones.end() -1;
	}
	if(bone != nullptr)
		*bone = *it;
	return it -bones.begin();
}

static void update_skeletal_hierarchy(SMDModel &smd,Skeleton &skeleton)
{
	/*auto &smdNodes = smd.GetNodes();
	std::function<void(std::vector<uint64_t> &nodes,std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones,const std::shared_ptr<Bone> &parent)> fUpdate;
	fUpdate = [&smdNodes,&skeleton,&fUpdate](std::vector<uint64_t> &nodes,std::unordered_map<uint32_t,std::shared_ptr<Bone>> &bones,const std::shared_ptr<Bone> &parent) {
		for(auto &nodeId : nodes)
		{
			auto &node = smdNodes[nodeId];
			std::shared_ptr<Bone> bone = nullptr;
			auto boneId = add_bone(node,skeleton,&bone);
			bone->parent = parent;
			auto it = bones.find(boneId);
			if(it == bones.end())
				it = bones.insert(std::remove_reference_t<decltype(bones)>::value_type(boneId,bone)).first; // Can cause crash! TODO Why??
			fUpdate(node.children,it->second->children,bone);
		}
	};
	auto &rootNodes = smd.GetSkeleton().rootNodes;
	auto &rootBones = skeleton.GetRootBones();
	fUpdate(rootNodes,rootBones,nullptr);*/
}

static void add_nodes_to_skeleton(SMDModel &smd,Skeleton &skeleton,Animation *anim)
{
	auto &smdSkeleton = smd.GetSkeleton();
	auto &smdNodes = smd.GetNodes();
	auto &bones = skeleton.GetBones();
	auto *boneList = (anim != nullptr) ? &anim->GetBoneList() : nullptr;
	anim->ReserveBoneIds(anim->GetBoneCount() +smdNodes.size());
	for(auto i=decltype(smdNodes.size()){0};i<smdNodes.size();++i)
	{
		auto &node = smdNodes[i];
		auto boneId = add_bone(node,skeleton);
		if(boneList != nullptr)
			anim->AddBoneId(boneId);
	}
	update_skeletal_hierarchy(smd,skeleton);
}

static void to_local_bone_system(const SMDModel::Frame &frame,const SMDModel::Node &node,Vector3 &pos,Quat &rot)
{
	auto parentId = node.parent;
	if(parentId != -1)
	{
		auto &tParent = frame.transforms[parentId];
		pos -= tParent.position;
		auto inv = uquat::get_inverse(tParent.rotation);
		uvec::rotate(&pos,inv);

		rot = inv *rot;
	}
}

static void generate_reference(SMDModel &smd,Model &mdl)
{
	auto animId = mdl.LookupAnimation("reference");
	if(animId == -1)
	{
		mdl.AddAnimation("reference",Animation::Create());
		animId = mdl.LookupAnimation("reference");
	}

	auto &smdFrame = smd.GetFrames().front();
	auto anim = mdl.GetAnimation(animId);
	auto frame = anim->GetFrame(0);
	if(frame == nullptr)
	{
		frame = Frame::Create(smdFrame.transforms.size());
		anim->AddFrame(frame);
	}

	auto &nodes = smd.GetNodes();
	auto &skeleton = mdl.GetSkeleton();
	for(auto i=decltype(smdFrame.transforms.size()){0};i<smdFrame.transforms.size();++i)
	{
		auto &t = smdFrame.transforms[i];
		auto &node = nodes[i];
		auto boneId = add_bone(node,skeleton);
		frame->SetBoneCount(umath::max(frame->GetBoneCount(),boneId +1));

		auto &pos = t.position;
		auto &rot = t.rotation;
		//to_local_bone_system(smdFrame,node,pos,rot);

		frame->SetBonePosition(boneId,pos);
		frame->SetBoneOrientation(boneId,rot);

		auto m = glm::toMat4(t.rotation);
		m = glm::translate(m,t.position);
		mdl.SetBindPoseBoneMatrix(boneId,glm::inverse(m));
	}
	update_skeletal_hierarchy(smd,skeleton);
	auto refFrame = Frame::Create(*frame);
	frame->Localize(*anim,mdl.GetSkeleton());
	mdl.SetReference(refFrame);
}

static void update_reference(Model &mdl,Animation &anim) // Adds all new bones to the reference pose
{
	auto numBones = mdl.GetBoneCount();
	auto &animBoneList = anim.GetBoneList();
	auto &reference = mdl.GetReference();
	for(auto i=decltype(numBones){0};i<numBones;++i)
	{
		auto *mat = mdl.GetBindPoseBoneMatrix(i);
		if(mat == nullptr)
		{
			reference.SetBoneCount(umath::max(reference.GetBoneCount(),i +1));
			auto it = std::find(animBoneList.begin(),animBoneList.end(),i);
			if(it != animBoneList.end())
			{
				auto animBoneId = it -animBoneList.begin();
				auto &frame = *anim.GetFrame(0);
				auto &pos = *frame.GetBonePosition(animBoneId);
				auto &rot = *frame.GetBoneOrientation(animBoneId);

				reference.SetBonePosition(i,pos);
				reference.SetBoneOrientation(i,rot);

				auto m = glm::toMat4(rot);
				m = glm::translate(m,pos);
				mdl.SetBindPoseBoneMatrix(i,glm::inverse(m));
			}
			else
			{
				reference.SetBonePosition(i,{});
				reference.SetBoneOrientation(i,{});
				mdl.SetBindPoseBoneMatrix(i,{});
			}
		}
	}
}

static void generate_animation(SMDModel &smd,Model &mdl,Animation &anim)
{
	auto &smdFrames = smd.GetFrames();
	auto numBones = anim.GetBoneCount();
	auto &nodes = smd.GetNodes();
	for(auto &smdFrame : smdFrames)
	{
		auto frame = Frame::Create(numBones);
		for(auto i=decltype(smdFrame.transforms.size()){0};i<smdFrame.transforms.size();++i)
		{
			auto &node = nodes[i];
			auto &t = smdFrame.transforms[i];

			auto pos = t.position;
			auto rot = t.rotation;
			to_local_bone_system(smdFrame,node,pos,rot);

			frame->SetBonePosition(i,pos);
			frame->SetBoneOrientation(i,rot);
		}
		anim.AddFrame(frame);
	}
	update_reference(mdl,anim);
}

static std::vector<uint32_t> get_skeleton_translation_table(SMDModel &smd,Model &mdl)
{
	auto &nodes = smd.GetNodes();
	auto &skeleton = mdl.GetSkeleton();
	auto &bones = skeleton.GetBones();

	std::vector<uint32_t> t;
	t.reserve(nodes.size());
	for(auto &node : nodes)
	{
		auto it = std::find_if(bones.begin(),bones.end(),[&node](const std::shared_ptr<Bone> &bone) {
			return (node.name == bone->name) ? true : false;
		});
		t.push_back((*it)->ID);
	}
	return t;
}

bool import::load_smd(NetworkState *nw,const std::string &name,Model &mdl,SMDModel &smd,bool bCollision,std::vector<std::string> &textures)
{
	auto *game = nw->GetGameState();
	auto &meshes = smd.GetMeshes();
	if(bCollision == true)
	{
		for(auto &mesh : meshes)
		{
			auto colMesh = CollisionMesh::Create(game);
			for(auto &tri : mesh.triangles)
			{
				for(uint8_t i=0;i<3;++i)
					colMesh->AddVertex(tri.vertices[i].position);
			}
			colMesh->Update();
			mdl.AddCollisionMesh(colMesh);
		}
		mdl.Update();
	}
	else if(!meshes.empty())
	{
		add_nodes_to_skeleton(smd,mdl.GetSkeleton(),nullptr);
		auto mdlMesh = std::shared_ptr<ModelMesh>(nw->CreateMesh());
		auto boneTranslation = get_skeleton_translation_table(smd,mdl);

		auto numMats = mdl.GetMaterials().size();
		textures.reserve(textures.size() +smd.GetMeshes().size());
		for(auto &mesh : smd.GetMeshes())
		{
			auto subMesh = std::shared_ptr<ModelSubMesh>(nw->CreateSubMesh());
			auto &verts = subMesh->GetVertices();
			for(auto &tri : mesh.triangles)
			{
				std::array<uint32_t,3> triangleIndices = {0,0,0};
				for(uint8_t i=0;i<3;++i)
				{
					auto &triVert = tri.vertices[i];
					auto it = std::find_if(verts.begin(),verts.end(),[&triVert](const Vertex &v) {
						return (umath::abs(triVert.position.x -v.position.x) <= VERTEX_EPSILON && umath::abs(triVert.position.y -v.position.y) <= VERTEX_EPSILON && umath::abs(triVert.position.z -v.position.z) <= VERTEX_EPSILON) ? true : false;
					});
					if(it == verts.end())
					{
						auto vertId = subMesh->AddVertex({triVert.position,{triVert.uv.x,-triVert.uv.y},triVert.normal});
						triangleIndices[i] = subMesh->GetVertexCount() -1;

						VertexWeight w {};
						uint8_t c = 0;
						for(auto &pair : triVert.weights)
						{
							if(pair.first != -1)
							{
								w.boneIds[c] = boneTranslation[pair.first];
								w.weights[c] = pair.second;
								if(++c == 4)
									break;
							}
						}
						subMesh->SetVertexWeight(vertId,w);

						it = verts.end() -1;
					}
					else
						triangleIndices[i] = it -verts.begin();
				}
				subMesh->AddTriangle(triangleIndices[0],triangleIndices[1],triangleIndices[2]);
			}
			textures.push_back(mesh.texture);
			auto texId = import::util::add_texture(mdl,mesh.texture);
			subMesh->SetTexture(texId);
			subMesh->Update();
			mdlMesh->AddSubMesh(subMesh);
		}
		mdlMesh->Update();
		auto meshGroup = mdl.GetMeshGroup(0);
		if(meshGroup == nullptr)
			meshGroup = mdl.AddMeshGroup("reference");
		meshGroup->AddMesh(mdlMesh);
		generate_reference(smd,mdl);
		return true;
	}
	else
	{
		auto anim = Animation::Create();
		add_nodes_to_skeleton(smd,mdl.GetSkeleton(),anim.get());
		generate_animation(smd,mdl,*anim);
		mdl.AddAnimation(name,anim);
	}
	return true;
}
