/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/file_formats/wmd.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/model/animation/vertex_animation.hpp"
#include "pragma/model/animation/flex_animation.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"

void FWMD::LoadBones(unsigned short version,unsigned int numBones,Model &mdl)
{
	auto &skeleton = mdl.GetSkeleton();
	auto reference = Animation::Create();
	if(!m_bStatic)
	{
		reference->ReserveBoneIds(reference->GetBoneCount() +numBones);
		for(unsigned int i=0;i<numBones;i++)
		{
			auto *bone = new Bone;
			bone->name = ReadString();
			skeleton.AddBone(bone);
			reference->AddBoneId(i);
		}
	}
	auto frame = Frame::Create((numBones == 0) ? 1 : numBones);
	if(numBones == 0)
	{
		auto *root = new Bone;
		root->name = "root";
		unsigned int rootID = skeleton.AddBone(root);
		mdl.SetBindPoseBoneMatrix(0,glm::inverse(umat::identity()));
		auto &rootBones = skeleton.GetRootBones();
		rootBones[0] = skeleton.GetBone(rootID).lock();
		reference->AddBoneId(0);

		Vector3 pos(0,0,0);
		auto orientation = uquat::identity();
		frame->SetBonePosition(0,pos);
		frame->SetBoneOrientation(0,orientation);
	}
	else
	{
		for(unsigned int i=0;i<numBones;i++)
		{
			Quat orientation = uquat::identity();
			for(unsigned char j=0;j<4;j++)
				orientation[j] = Read<float>();
			Vector3 pos;
			for(unsigned char j=0;j<3;j++)
				pos[j] = Read<float>();
			frame->SetBonePosition(i,pos);
			frame->SetBoneOrientation(i,orientation);

			if(version <= 0x0001)
			{
				unsigned char joint = Read<unsigned char>();
				UNUSED(joint);
			}
			//trans = glm::translate(trans,pos);
			//trans = glm::rotate(trans,orientation);
			/*Matex4x3 trans;
			for(int x=0;x<4;x++)
			{
				for(int y=0;y<3;y++)
					trans[x][y] = Read<float>();
			}*/
			//frame->SetBoneMatrix(i,trans);
			//mdl.SetBindPoseBoneMatrix(i,glm::inverse(Mat4(trans)));
		}
	}
	if(!m_bStatic)
	{
		unsigned int numRoot = Read<unsigned int>();
		auto &rootBones = skeleton.GetRootBones();
		for(unsigned int i=0;i<numRoot;i++)
		{
			unsigned int rootID = Read<unsigned int>();
			auto root = skeleton.GetBone(rootID).lock();
			rootBones[rootID] = root;
			ReadChildBones(skeleton,root);
		}
	}
	auto refFrame = Frame::Create(*frame);
	frame->Localize(*reference,skeleton);
	reference->AddFrame(frame);
	mdl.AddAnimation("reference",reference);
	mdl.SetReference(refFrame);
	mdl.GenerateBindPoseMatrices();
}

void FWMD::LoadHitboxes(uint16_t version,Model &mdl)
{
	if(version <= 0x0004) // Hitboxes aren't supported until version 0x0005
		return;
	auto numHitboxes = Read<uint32_t>();
	for(auto i=decltype(numHitboxes){0};i<numHitboxes;++i)
	{
		auto boneId = Read<uint32_t>();
		auto group = Read<uint32_t>();
		auto min = Read<Vector3>();
		auto max = Read<Vector3>();
		mdl.AddHitbox(boneId,static_cast<HitGroup>(group),min,max);
	}
}

void FWMD::LoadObjectAttachments(Model &mdl)
{
	auto numObjectAttachments = Read<uint32_t>();
	mdl.GetObjectAttachments().reserve(numObjectAttachments);
	for(auto i=decltype(numObjectAttachments){0u};i<numObjectAttachments;++i)
	{
		auto type = Read<ObjectAttachment::Type>();
		auto name = ReadString();
		auto attachment = ReadString();
		auto numKeyValues = Read<uint32_t>();
		std::unordered_map<std::string,std::string> keyValues;
		keyValues.reserve(numKeyValues);
		for(auto j=decltype(numKeyValues){0u};j<numKeyValues;++j)
		{
			auto key = ReadString();
			auto val = ReadString();
			keyValues.insert(std::make_pair(key,val));
		}
		mdl.AddObjectAttachment(type,name,attachment,keyValues);
	}
}

void FWMD::LoadAttachments(Model &mdl)
{
	unsigned int numAttachments = Read<unsigned int>();
	for(unsigned int i=0;i<numAttachments;i++)
	{
		std::string name = ReadString();
		unsigned int boneID = Read<unsigned int>();
		Vector3 offset;
		offset.x = Read<float>();
		offset.y = Read<float>();
		offset.z = Read<float>();
		EulerAngles angles;
		angles.p = Read<float>();
		angles.y = Read<float>();
		angles.r = Read<float>();
		mdl.AddAttachment(name,boneID,offset,angles);
	}
}

static void clamp_bounds(Vector3 &min,Vector3 &max,unsigned short version)
{
	if(version >= 36)
		return;
	for(auto &v : {&min,&max})
	{
		for(uint8_t i=0;i<3;++i)
		{
			if((*v)[i] == std::numeric_limits<float>::lowest() || (*v)[i] == std::numeric_limits<float>::max())
				(*v)[i] = 0.f;
		}
	}
}

void FWMD::LoadMeshes(unsigned short version,Model &mdl,const std::function<std::shared_ptr<ModelMesh>()> &meshFactory,const std::function<std::shared_ptr<ModelSubMesh>()> &subMeshFactory)
{
	Vector3 renderMin,renderMax;
	for(char i=0;i<3;i++)
		renderMin[i] = Read<float>();
	for(char i=0;i<3;i++)
		renderMax[i] = Read<float>();
	clamp_bounds(renderMin,renderMax,version);
	mdl.SetRenderBounds(renderMin,renderMax);
	unsigned int numMeshGroups = Read<unsigned int>();
	for(unsigned int i=0;i<numMeshGroups;i++)
	{
		std::string name = ReadString();
		auto group = mdl.AddMeshGroup(name);
		uint32_t numMeshes = (version < 30) ? Read<unsigned char>() : Read<uint32_t>();
		for(uint32_t j=0;j<numMeshes;j++)
		{
			auto mesh = meshFactory();
			if(version >= 35)
			{
				auto refId = Read<uint32_t>();
				mesh->SetReferenceId(refId);
			}
			if(version <= 0x0017)
			{
				unsigned long long numVerts = Read<unsigned long long>();

				std::vector<FWMDVertex> wmdVerts;
				wmdVerts.reserve(numVerts);
				for(unsigned long long j=0;j<numVerts;j++)
				{
					FWMDVertex v;
					v.position.x = Read<float>();
					v.position.y = Read<float>();
					v.position.z = Read<float>();
					v.normal.x = Read<float>();
					v.normal.y = Read<float>();
					v.normal.z = Read<float>();

					wmdVerts.push_back(v);
				}
				unsigned int numBones = Read<unsigned int>();
				for(unsigned int j=0;j<numBones;j++)
				{
					unsigned int boneID = Read<unsigned int>();
					unsigned long long numVerts = Read<unsigned long long>();
					for(unsigned long long k=0;k<numVerts;k++)
					{
						unsigned long long vertID = Read<unsigned long long>();
						float weight = Read<float>();
						if(vertID < wmdVerts.size())
							wmdVerts[vertID].weights.insert(std::unordered_map<unsigned long long,float>::value_type(boneID,weight));
					}
				}

				unsigned int numMaps = Read<unsigned int>();
				for(unsigned int j=0;j<numMaps;j++)
				{
					auto subMesh = subMeshFactory();
					auto &subVertices = subMesh->GetVertices();
					auto &triangles = subMesh->GetTriangles();
					auto &vertexWeights = subMesh->GetVertexWeights();

					auto texID = Read<unsigned short>();
					subMesh->SetSkinTextureIndex(texID);

					auto numMeshVerts = Read<unsigned long long>();
					subVertices.reserve(numMeshVerts);

					std::vector<unsigned long long> meshVertIDs(numMeshVerts);
					for(unsigned long long k=0;k<numMeshVerts;k++)
					{
						auto meshVertID = Read<unsigned long long>();
						meshVertIDs[k] = meshVertID;

						auto &vert = wmdVerts[meshVertID];
						subVertices.push_back(Vertex{
							vert.position,
							vert.normal
							});
						if(!vert.weights.empty())
						{
							vertexWeights.push_back(VertexWeight{
								Vector4i{-1,-1,-1,-1},
								Vector4{0.f,0.f,0.f,0.f}
								});
							auto &weight = vertexWeights.back();
							int numWeights = 0;
							for(auto m=vert.weights.begin();m!=vert.weights.end();m++)
							{
								weight.weights[numWeights] = m->second;
								weight.boneIds[numWeights] = CInt32(m->first);
								numWeights++;
								if(numWeights == 4)
									break;
							}
						}
					}

					std::vector<Vector2> uvs(numMeshVerts);
					Read(uvs.data(),uvs.size() *sizeof(Vector2));
					if(version <= 0x0005) // Version 5 and lower used OpenGL texture coordinates (lower left origin), they'll have to be flipped first
					{
						for(auto &uv : uvs)
							uv.y = 1.f -uv.y;
					}
					for(auto i=decltype(subVertices.size()){0};i<subVertices.size();++i)
					{
						auto &v = subVertices[i];
						v.uv = uvs[i];
					}
					// float w = Read<float>(); // We don't need 3D-Mapping for the time being

					auto numFaces = Read<unsigned int>();
					for(unsigned int k=0;k<numFaces;k++)
					{
						for(int l=0;l<3;l++)
						{
							auto localID = Read<unsigned long long>();
							//FWMDVertex &vert = vertices[meshVertIDs[localID]];
							triangles.push_back(static_cast<uint16_t>(localID));
						}
					}
					if(subVertices.size() > 0)
						mesh->AddSubMesh(subMesh);
				}
			}
			else
			{
				auto numSubMeshes = Read<uint32_t>();
				for(auto i=decltype(numSubMeshes){0u};i<numSubMeshes;++i)
				{
					auto subMesh = subMeshFactory();

					if(version >= 26)
					{
						auto pose = Read<umath::ScaledTransform>();
						subMesh->SetPose(pose);
					}

					auto texId = Read<uint16_t>();
					subMesh->SetSkinTextureIndex(texId);

					if(version >= 27)
					{
						auto geometryType = Read<ModelSubMesh::GeometryType>();
						subMesh->SetGeometryType(geometryType);
					}

					if(version >= 35)
					{
						auto refId = Read<uint32_t>();
						subMesh->SetReferenceId(refId);
					}

					auto &verts = subMesh->GetVertices();
					auto &tris = subMesh->GetTriangles();
					auto &vertWeights = subMesh->GetVertexWeights();

					auto numVerts = Read<uint64_t>();
					verts.reserve(numVerts);
					for(auto i=decltype(numVerts){0u};i<numVerts;++i)
					{
						verts.push_back({});
						auto &v = verts.back();
						v.position = Read<Vector3>();
						v.normal = Read<Vector3>();
						if(version < 30)
							v.uv = Read<Vector2>();
					}

					if(version >= 30)
					{
						auto &uvSets = subMesh->GetUVSets();
						auto numUvSets = Read<uint8_t>();
						assert(numUvSets > 0); // There always has to be at least one uv set!
						uvSets.reserve(numUvSets -1);
						for(auto i=decltype(numUvSets){0u};i<numUvSets;++i)
						{
							auto name = ReadString();
							if(name == "base")
							{
								for(auto j=decltype(numVerts){0u};j<numVerts;++j)
									verts.at(j).uv = Read<Vector2>();
							}
							else
							{
								auto it = uvSets.insert(std::make_pair(name,std::vector<Vector2>{})).first;
								auto &uvSet = it->second;

								uvSet.resize(numVerts);
								Read(uvSet.data(),uvSet.size() *sizeof(uvSet.front()));
							}
						}
					}

					auto numVertWeights = Read<uint64_t>();
					vertWeights.resize(numVertWeights);
					static_assert(sizeof(decltype(vertWeights.front())) == sizeof(Vector4) *2);
					Read(vertWeights.data(),vertWeights.size() *sizeof(decltype(vertWeights.front())));

					if(version >= 27)
					{
						auto numExtVertWeights = Read<uint64_t>();
						auto &extVertWeights = subMesh->GetExtendedVertexWeights();
						extVertWeights.resize(numExtVertWeights);
						static_assert(sizeof(decltype(extVertWeights.front())) == sizeof(Vector4) *2);
						Read(extVertWeights.data(),extVertWeights.size() *sizeof(decltype(extVertWeights.front())));
					}


					if(version >= 30)
					{
						auto numAlphas = Read<uint8_t>();
						if(numAlphas > 0)
						{
							subMesh->SetAlphaCount(numAlphas);
							auto &alphas = subMesh->GetAlphas();
							alphas.resize(numVerts,Vector2{});
							for(auto i=decltype(numVerts){0u};i<numVerts;++i)
							{
								alphas.at(i).x = Read<float>();
								if(numAlphas > 1)
									alphas.at(i).y = Read<float>();
							}
						}
					}

					uint32_t numIndices = Read<uint32_t>();
					if(version < 30)
						numIndices *= 3;
					tris.resize(numIndices);
					static_assert(std::is_same_v<std::remove_reference_t<decltype(tris.front())>,uint16_t>);
					Read(tris.data(),tris.size() *sizeof(decltype(tris.front())));

					mesh->AddSubMesh(subMesh);
				}
			}

			mesh->Update(ModelUpdateFlags::All);
			group->AddMesh(mesh);
		}
	}

	if(version >= 0x0004)
	{
		auto &baseMeshes = mdl.GetBaseMeshes();
		auto numBaseMeshes = Read<unsigned short>();
		baseMeshes.reserve(baseMeshes.size() +numBaseMeshes);
		for(unsigned short i=0;i<numBaseMeshes;i++)
		{
			auto meshGroupId = Read<unsigned int>();
			baseMeshes.push_back(meshGroupId);
		}
	}
}

void FWMD::LoadCollisionMeshes(Game *game,unsigned short version,Model &mdl,SurfaceMaterial *smDefault)
{
	if(smDefault == nullptr)
		smDefault = m_gameState->GetSurfaceMaterial(0);
	float mass = Read<float>();
	mdl.SetMass(mass);
	Vector3 collisionMin(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
	Vector3 collisionMax(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
	uint32_t numMeshes = (version < 30) ? Read<uint8_t>() : Read<uint32_t>();
	auto massPerMesh = mass /static_cast<float>(numMeshes); // TODO: Allow individual mass per collision mesh
	std::vector<JointInfo> oldJointSystemJoints;
	for(auto i=decltype(numMeshes){0u};i<numMeshes;++i)
	{
		auto flags = CollisionMeshLoadFlags::None;
		if(version >= 30)
			flags = Read<CollisionMeshLoadFlags>();
		int boneParent = Read<int>();
		if(boneParent == -1)
			boneParent = 0;
		Vector3 origin(0.f,0.f,0.f);
		if(version >= 0x0002)
		{
			origin.x = Read<float>();
			origin.y = Read<float>();
			origin.z = Read<float>();
		}
		auto *matSurface = smDefault;
		std::string surfaceMaterial;
		if(version >= 0x0003)
			surfaceMaterial = ReadString();
		auto *matCustom = m_gameState->GetSurfaceMaterial(surfaceMaterial);
		if(matCustom != nullptr && matCustom->GetIndex() != 0)
			matSurface = matCustom;
		for(char j=0;j<3;j++)
			collisionMin[j] = umath::min(collisionMin[j],Read<float>() -origin[j]);
		for(char j=0;j<3;j++)
			collisionMax[j] = umath::max(collisionMax[j],Read<float>() -origin[j]);
		auto mesh = CollisionMesh::Create(game);
		if(matSurface != nullptr)
			mesh->SetSurfaceMaterial(CInt32(matSurface->GetIndex()));
		mesh->SetMass(massPerMesh);
		mesh->SetOrigin(origin);
		std::vector<Vector3> &colVerts = mesh->GetVertices();
		mesh->SetBoneParent(boneParent);
		mesh->SetConvex(umath::is_flag_set(flags,CollisionMeshLoadFlags::Convex));

		auto numVerts = Read<unsigned long long>();
		colVerts.resize(numVerts);
		m_file->Read(colVerts.data(),numVerts *sizeof(colVerts.front()));

		if(version >= 0x0010)
		{
			auto &triangles = mesh->GetTriangles();
			
			auto numTriangles = Read<uint64_t>();
			triangles.resize(numTriangles);
			m_file->Read(triangles.data(),numTriangles *sizeof(uint16_t));
			if(version >= 0x0011)
			{
				auto volume = m_file->Read<double>();
				auto com = m_file->Read<Vector3>();
				mesh->SetVolume(volume);
				mesh->SetCenterOfMass(com);
			}
			else
				mesh->CalculateVolumeAndCom();
		}
		if(version >= 0x0002 && version < 38)
		{
			auto numConstraints = Read<unsigned char>();
			for(unsigned char iConstraint=0;iConstraint<numConstraints;iConstraint++)
			{
				auto type = Read<JointType>();
				auto idTgt = Read<unsigned int>();
				oldJointSystemJoints.push_back(JointInfo{type,i,idTgt});
				auto &joint = oldJointSystemJoints.back();
				joint.collide = Read<bool>();
				auto numArgs = Read<unsigned char>();
				for(unsigned char i=0;i<numArgs;i++)
				{
					auto k = ReadString();
					auto v = ReadString();
					joint.args[k] = v;
				}
			}
		}
		// Version 0x0014
		if(version >= 0x0014)
		{
			auto bSoftBodyData = (version < 30) ? Read<bool>() : umath::is_flag_set(flags,CollisionMeshLoadFlags::SoftBody);
			if(bSoftBodyData)
				LoadSoftBodyData(mdl,*mesh);
		}
		//
		mesh->CalculateBounds();
		mdl.AddCollisionMesh(mesh);
	}
	mdl.SetCollisionBounds(collisionMin,collisionMax);
	mdl.UpdateShape();

	// Old joint system, where joints referenced collision meshes instead of bones.
	// We'll translate it into the new system here
	auto &colMeshes = mdl.GetCollisionMeshes();
	for(auto &jointInfo : oldJointSystemJoints)
	{
		auto colSrcId = jointInfo.child;
		auto colDstId = jointInfo.parent;
		if(colSrcId >= numMeshes || colDstId >= numMeshes)
			continue;
		auto &meshSrc = colMeshes[colSrcId];
		auto &meshDst = colMeshes[colDstId];
		auto srcBoneId = meshSrc->GetBoneParent();
		auto dstBoneId = meshDst->GetBoneParent();
		if(srcBoneId < 0 || dstBoneId < 0)
			continue;
		auto &jointInfoMdl = mdl.AddJoint(jointInfo.type,srcBoneId,dstBoneId);
		jointInfoMdl.args = jointInfo.args;
		jointInfoMdl.collide = jointInfo.collide;
	}
}

void FWMD::LoadBlendControllers(Model &mdl)
{
	unsigned short numControllers = Read<unsigned short>();
	for(unsigned short i=0;i<numControllers;i++)
	{
		std::string name = ReadString();
		int min = Read<int>();
		int max = Read<int>();
		bool loop = Read<bool>();
		mdl.AddBlendController(name,min,max,loop);
	}
}

void FWMD::LoadIKControllers(uint16_t version,Model &mdl)
{
	if(version < 0x0016)
		return;
	auto numIkControllers = Read<uint32_t>();
	for(auto i=decltype(numIkControllers){0};i<numIkControllers;++i)
	{
		auto name = ReadString();
		auto type = ReadString();
		auto chainLength = Read<uint32_t>();
		auto method = Read<uint32_t>();
		auto *controller = mdl.AddIKController(name,chainLength,type,static_cast<util::ik::Method>(method));

		auto *keyValues = (controller != nullptr) ? &controller->GetKeyValues() : nullptr;
		auto numKeyValues = Read<uint32_t>();
		for(auto j=decltype(numKeyValues){0};j<numKeyValues;++j)
		{
			auto key = ReadString();
			auto val = ReadString();
			if(keyValues != nullptr)
				(*keyValues)[key] = val;
		}
	}
}

void FWMD::LoadAnimations(unsigned short version,Model &mdl)
{
	unsigned int numAnimations = Read<unsigned int>();
	for(unsigned int i=0;i<numAnimations;i++)
	{
		std::string name = ReadString();
		FWAD wad;
		auto anim = std::shared_ptr<Animation>(wad.ReadData(version,m_file));
		if(anim)
		{
			if(version < 0x0007)
			{
				Vector3 min,max;
				mdl.GetRenderBounds(min,max);
				clamp_bounds(min,max,version);
				anim->SetRenderBounds(min,max);
			}
			mdl.AddAnimation(name,anim);
		}
	}

	if(version >= 0x0015)
	{
		auto numVertexAnims = Read<uint32_t>();
		mdl.GetVertexAnimations().reserve(numVertexAnims);
		for(auto i=decltype(numVertexAnims){0};i<numVertexAnims;++i)
		{
			auto name = ReadString();
			auto va = mdl.AddVertexAnimation(name);
			auto numMeshAnims = Read<uint32_t>();
			va->GetMeshAnimations().reserve(numMeshAnims);
			for(auto j=decltype(numMeshAnims){0};j<numMeshAnims;++j)
			{
				auto meshGroupId = Read<uint32_t>();
				auto meshId = Read<uint32_t>();
				auto subMeshId = Read<uint32_t>();

				auto meshGroup = mdl.GetMeshGroup(meshGroupId);
				auto &meshes = meshGroup->GetMeshes();
				std::shared_ptr<ModelMesh> mesh = (meshId < meshes.size()) ? meshes.at(meshId) : nullptr;
				std::shared_ptr<ModelSubMesh> subMesh = nullptr;
				if(mesh != nullptr)
				{
					auto &subMeshes = mesh->GetSubMeshes();
					if(subMeshId < subMeshes.size())
						subMesh = subMeshes.at(subMeshId);
				}
				auto numFrames = Read<uint32_t>();
				for(auto k=decltype(numFrames){0};k<numFrames;++k)
				{
					uint64_t endOfFrameOffset = 0;
					if(version >= 32)
						endOfFrameOffset = Read<uint64_t>();
					auto flags = MeshVertexFrame::Flags::None;
					if(version >= 25)
						flags = Read<MeshVertexFrame::Flags>();

					if(version >= 32)
					{
						if(subMesh == nullptr)
						{
							m_file->Seek(endOfFrameOffset);
							Con::cwar<<"WARNING: Invalid mesh reference in vertex animation '"<<name<<"'! Skipping..."<<Con::endl;
						}
						else
						{
							auto meshFrame = va->AddMeshFrame(*mesh,*subMesh);
							meshFrame->SetFlags(flags);

							auto numUsedVerts = Read<uint16_t>();
							std::vector<uint16_t> usedVertIndices {};
							usedVertIndices.resize(numUsedVerts);
							Read(usedVertIndices.data(),usedVertIndices.size() *sizeof(usedVertIndices.front()));

							auto numAttributes = Read<uint16_t>();
							for(auto i=decltype(numAttributes){0u};i<numAttributes;++i)
							{
								auto attrName = ReadString();
								std::vector<std::array<uint16_t,4>> vdata {};
								vdata.resize(numUsedVerts);
								Read(vdata.data(),vdata.size() *sizeof(vdata.front()));
								if(attrName == "position")
								{
									for(auto j=decltype(numUsedVerts){0u};j<numUsedVerts;++j)
									{
										auto vertIdx = usedVertIndices.at(j);
										meshFrame->SetVertexPosition(vertIdx,vdata.at(j));
									}
								}
								else if(attrName == "normal")
								{
									for(auto j=decltype(numUsedVerts){0u};j<numUsedVerts;++j)
									{
										auto vertIdx = usedVertIndices.at(j);
										meshFrame->SetVertexNormal(vertIdx,vdata.at(j));
									}
								}
							}
						}
					}
					else
					{
						auto numUsedVerts = Read<uint16_t>();
						if(subMesh != nullptr)
						{
							auto meshFrame = va->AddMeshFrame(*mesh,*subMesh);
							meshFrame->SetFlags(flags);
							for(auto l=decltype(numUsedVerts){0};l<numUsedVerts;++l)
							{
								auto idx = Read<uint16_t>();
								auto v = Read<std::array<uint16_t,3>>();
								meshFrame->SetVertexPosition(idx,v);
								if(umath::is_flag_set(flags,MeshVertexFrame::Flags::HasDeltaValues))
								{
									auto deltaVal = Read<uint16_t>();
									meshFrame->SetDeltaValue(idx,deltaVal);
								}
							}
						}
						else
						{
							Con::cwar<<"WARNING: Invalid mesh reference in vertex animation '"<<name<<"'! Skipping..."<<Con::endl;
							auto szPerVertex = sizeof(uint16_t) *3;
							if(umath::is_flag_set(flags,MeshVertexFrame::Flags::HasDeltaValues))
								szPerVertex += sizeof(uint16_t);
							m_file->Seek(m_file->Tell() +numUsedVerts *szPerVertex);
						}
					}
				}
			}
		}

		auto numFlexControllers = Read<uint32_t>();
		auto &flexControllers = mdl.GetFlexControllers();
		flexControllers.reserve(numFlexControllers);
		for(auto i=decltype(numFlexControllers){0};i<numFlexControllers;++i)
		{
			flexControllers.push_back({});
			auto &fc = flexControllers.back();
			fc.name = ReadString();
			fc.min = Read<float>();
			fc.max = Read<float>();
		}

		auto numFlexes = Read<uint32_t>();
		auto &flexes = mdl.GetFlexes();
		for(auto i=decltype(numFlexes){0};i<numFlexes;++i)
		{
			auto name = ReadString();
			flexes.push_back(Flex{name});
			auto &flex = flexes.back();

			auto vaIdx = Read<uint32_t>();
			uint32_t frameIndex = 0;
			if(version < 31)
				m_file->Seek(m_file->Tell() +sizeof(uint32_t) *2);
			else
				frameIndex = Read<uint32_t>();
			auto &vertAnims = mdl.GetVertexAnimations();
			if(vaIdx < vertAnims.size())
			{
				auto &va = vertAnims.at(vaIdx);
				flex.SetVertexAnimation(*va,frameIndex);
			}

			auto &ops = flex.GetOperations();
			auto numOps = Read<uint32_t>();
			ops.reserve(numOps);
			for(auto j=decltype(numOps){0};j<numOps;++j)
			{
				ops.push_back({});
				auto &op = ops.back();
				op.type = static_cast<Flex::Operation::Type>(Read<uint32_t>());
				op.d.value = Read<float>();
			}
		}

		auto &phonemeMap = mdl.GetPhonemeMap();
		auto numPhonemes = Read<uint32_t>();
		phonemeMap.phonemes.reserve(numPhonemes);
		for(auto i=decltype(numPhonemes){0};i<numPhonemes;++i)
		{
			auto name = ReadString();
			auto &flexControllerValues = (*phonemeMap.phonemes.insert(std::make_pair(name,PhonemeInfo{})).first).second.flexControllers;
			auto numFlexControllers = Read<uint32_t>();
			flexControllerValues.reserve(numFlexControllers);
			for(auto j=decltype(numFlexControllers){0};j<numFlexControllers;++j)
			{
				auto controllerId = Read<uint32_t>();
				auto val = Read<float>();
				auto controllerName = (controllerId < flexControllers.size()) ? flexControllers.at(controllerId).name : "";
				flexControllerValues.insert(std::make_pair(controllerName,val));
			}
		}

		if(version >= 28)
		{
			auto maxEyeDeflection = Read<float>();
			mdl.SetMaxEyeDeflection(maxEyeDeflection);
			auto &eyeballs = mdl.GetEyeballs();
			auto numEyeballs = Read<uint32_t>();
			eyeballs.reserve(numEyeballs);
			for(auto i=decltype(numEyeballs){0u};i<numEyeballs;++i)
			{
				eyeballs.push_back({});
				auto &eyeball = eyeballs.back();
				eyeball.name = ReadString();
				Read(reinterpret_cast<uint8_t*>(&eyeball) +sizeof(std::string),sizeof(Eyeball) -sizeof(std::string));
			}
		}

		if(version >= 37)
		{
			auto numFlexAnims = Read<uint32_t>();
			auto &flexAnims = mdl.GetFlexAnimations();
			auto &flexAnimNames = mdl.GetFlexAnimationNames();
			flexAnimNames.reserve(numFlexAnims);
			flexAnims.reserve(numFlexAnims);
			for(auto i=decltype(numFlexAnims){0u};i<numFlexAnims;++i)
			{
				auto name = ReadString();
				auto flexAnim = FlexAnimation::Load(m_file);
				flexAnimNames.push_back(name);
				flexAnims.push_back(flexAnim);
			}
		}
	}
}

void FWMD::LoadLODData(unsigned short version,Model &mdl)
{
	if(version >= 0x0004)
	{
		auto numLods = Read<unsigned char>();
		for(UChar i=0;i<numLods;i++)
		{
			auto lod = Read<unsigned char>();
			float dist;
			if(version >= 36)
				dist = Read<float>();
			else
				dist = (i +1) *500.f;
			auto numReplace = Read<unsigned char>();
			std::unordered_map<unsigned int,unsigned int> meshReplacements;
			meshReplacements.reserve(numReplace);
			for(UChar i=0;i<numReplace;i++)
			{
				auto origId = Read<unsigned int>();
				auto repId = Read<unsigned int>();
				meshReplacements[origId] = repId;
			}
			mdl.AddLODInfo(lod,dist,meshReplacements);
		}
		return;
	}

	// Version 0x0003 and lower
	auto numLODs = Read<unsigned char>();
	auto &baseMeshes = mdl.GetBaseMeshes();
	for(UChar i=0;i<numLODs;i++)
	{
		auto lod = Read<unsigned char>();
		auto numMeshes = Read<unsigned char>();
		for(UChar j=0;j<numMeshes;j++)
		{
			auto meshGroupID = Read<unsigned int>();
			if(lod == 0)
				baseMeshes.push_back(meshGroupID); // Old format doesn't have base meshes stored separately, so just grab them from LOD 0
		}
		auto numMeshesHidden = Read<unsigned char>();
		for(UChar j=0;j<numMeshesHidden;j++)
			Read<unsigned int>();
	}

	// Obsolete, because incompatible with new format (LODs won't work for these models)
	/*unsigned char numLODs = Read<unsigned char>();
	auto &baseMeshes = mdl.GetBaseMeshes();
	LODInfo *lodBase = NULL;
	for(unsigned char i=0;i<numLODs;i++)
	{
		unsigned char lod = Read<unsigned char>();
		unsigned char numMeshes = Read<unsigned char>();
		std::vector<unsigned int> meshIDs;
		for(unsigned char j=0;j<numMeshes;j++)
		{
			unsigned int meshGroupID = Read<unsigned int>();
			meshIDs.push_back(meshGroupID);
		}
		unsigned char numMeshesHidden = Read<unsigned char>();
		std::vector<unsigned int> meshIDsHidden;
		for(unsigned char j=0;j<numMeshesHidden;j++)
		{
			unsigned int meshGroupID = Read<unsigned int>();
			meshIDsHidden.push_back(meshGroupID);
		}
		if(lod == 0)
		{
			LODInfo *info = mdl.AddLODInfo(lod,meshIDs);
			lodBase = info;
		}
		else if(lodBase != NULL)
		{
			for(unsigned int j=0;j<lodBase->meshIDs.size();j++)
			{
				unsigned int meshID = lodBase->meshIDs[j];
				bool bHidden = false;
				for(unsigned int k=0;k<meshIDsHidden.size();k++)
				{
					if(meshIDsHidden[k] == meshID)
					{
						bHidden = true;
						break;
					}
				}
				if(bHidden == false)
					meshIDs.push_back(meshID);
			}
			mdl.AddLODInfo(lod,meshIDs);
			lodBase = mdl.GetLODInfo(0);
		}
	}*/
	//
	//
}

void FWMD::LoadSoftBodyData(Model &mdl,CollisionMesh &colMesh)
{
	auto meshGroupId = Read<uint32_t>();
	auto meshId = Read<uint32_t>();
	auto subMeshId = Read<uint32_t>();
	colMesh.SetSoftBody(true);
	auto bValid = false;
	auto meshGroup = mdl.GetMeshGroup(meshGroupId);
	if(meshGroup != nullptr)
	{
		auto &meshes = meshGroup->GetMeshes();
		if(meshId < meshes.size())
		{
			auto &subMeshes = meshes.at(meshId)->GetSubMeshes();
			if(subMeshId < subMeshes.size())
			{
				auto &subMesh = subMeshes.at(subMeshId);
				colMesh.SetSoftBodyMesh(*subMesh);
				bValid = true;
			}
		}
	}
	auto &triangleIndices = *colMesh.GetSoftBodyTriangles();
	auto &anchors = *colMesh.GetSoftBodyAnchors();
	auto &sbInfo = *colMesh.GetSoftBodyInfo();
	Read(&sbInfo,sizeof(float) *19 +sizeof(uint32_t) *2);
	sbInfo.materialStiffnessCoefficient.clear();
	auto numMaterials = Read<uint32_t>();
	for(auto i=decltype(numMaterials){0};i<numMaterials;++i)
	{
		auto matId = Read<uint32_t>();
		auto it = sbInfo.materialStiffnessCoefficient.find(matId);
		if(it == sbInfo.materialStiffnessCoefficient.end())
			it = sbInfo.materialStiffnessCoefficient.insert(std::make_pair(matId,PhysSoftBodyInfo::MaterialStiffnessCoefficient{})).first;
		it->second.linear = Read<float>();
		it->second.angular = Read<float>();
		it->second.volume = Read<float>();
	}

	auto numTriangles = Read<uint32_t>();
	triangleIndices.reserve(numTriangles);
	Read(triangleIndices.data(),triangleIndices.size() *sizeof(triangleIndices.front()));

	auto numAnchors = Read<uint32_t>();
	anchors.resize(numAnchors);
	Read(anchors.data(),anchors.size() *sizeof(anchors.front()));

	if(bValid == false)
		colMesh.SetSoftBody(false);
}

void FWMD::LoadJoints(Model &mdl)
{
	auto numJoints = Read<uint32_t>();
	for(auto i=decltype(numJoints){0u};i<numJoints;++i)
	{
		auto type = Read<JointType>();
		auto child = Read<BoneId>();
		auto parent = Read<BoneId>();
		auto &joint = mdl.AddJoint(type,child,parent);
		joint.collide = Read<bool>();
		auto numArgs = Read<uint8_t>();
		for(auto i=decltype(numArgs){0u};i<numArgs;++i)
		{
			auto k = ReadString();
			auto v = ReadString();
			joint.args[k] = v;
		}
	}
}

void FWMD::LoadBodygroups(Model &mdl)
{
	auto numBodygroups = Read<unsigned short>();
	for(unsigned short i=0;i<numBodygroups;i++)
	{
		auto name = ReadString();
		auto &bg = mdl.AddBodyGroup(name);
		auto numMeshes = Read<unsigned char>();
		for(unsigned char j=0;j<numMeshes;j++)
		{
			auto meshGroupId = Read<unsigned int>();
			bg.meshGroups.push_back(meshGroupId);
		}
	}
}
