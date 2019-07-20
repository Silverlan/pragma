#include "stdafx_shared.h"
#include "pragma/file_formats/wmd.h"
#include "pragma/model/modelmesh.h"
#include "pragma/physics/collisionmesh.h"
#include "pragma/model/animation/vertex_animation.hpp"
#include "pragma/physics/physsoftbodyinfo.hpp"

void FWMD::LoadBones(unsigned short version,unsigned int numBones,Model *mdl)
{
	auto &skeleton = mdl->GetSkeleton();
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
		mdl->SetBindPoseBoneMatrix(0,glm::inverse(umat::identity()));
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
			//mdl->SetBindPoseBoneMatrix(i,glm::inverse(Mat4(trans)));
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
	mdl->AddAnimation("reference",reference);
	mdl->SetReference(refFrame);
	mdl->GenerateBindPoseMatrices();
}

void FWMD::LoadHitboxes(uint16_t version,Model *mdl)
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
		mdl->AddHitbox(boneId,static_cast<HitGroup>(group),min,max);
	}
}

void FWMD::LoadObjectAttachments(Model *mdl)
{
	auto numObjectAttachments = Read<uint32_t>();
	mdl->GetObjectAttachments().reserve(numObjectAttachments);
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
		mdl->AddObjectAttachment(type,name,attachment,keyValues);
	}
}

void FWMD::LoadAttachments(Model *mdl)
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
		mdl->AddAttachment(name,boneID,offset,angles);
	}
}

void FWMD::LoadMeshes(unsigned short version,Model *mdl,const std::function<std::shared_ptr<ModelMesh>()> &meshFactory,const std::function<std::shared_ptr<ModelSubMesh>()> &subMeshFactory)
{
	Vector3 renderMin,renderMax;
	for(char i=0;i<3;i++)
		renderMin[i] = Read<float>();
	for(char i=0;i<3;i++)
		renderMax[i] = Read<float>();
	mdl->SetRenderBounds(renderMin,renderMax);
	unsigned int numMeshGroups = Read<unsigned int>();
	for(unsigned int i=0;i<numMeshGroups;i++)
	{
		std::string name = ReadString();
		auto group = mdl->AddMeshGroup(name);
		unsigned char numSubMeshes = Read<unsigned char>();
		for(unsigned char j=0;j<numSubMeshes;j++)
		{
			unsigned long long numVerts = Read<unsigned long long>();
			std::vector<FWMDVertex> vertices;
			std::vector<Vector3> uvws;
			for(unsigned long long j=0;j<numVerts;j++)
			{
				FWMDVertex v;
				v.position.x = Read<float>();
				v.position.y = Read<float>();
				v.position.z = Read<float>();
				v.normal.x = Read<float>();
				v.normal.y = Read<float>();
				v.normal.z = Read<float>();

				vertices.push_back(v);
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
					if(vertID < vertices.size())
						vertices[vertID].weights.insert(std::unordered_map<unsigned long long,float>::value_type(boneID,weight));
				}
			}
			unsigned int numMaps = Read<unsigned int>();
			auto mesh = meshFactory();
			for(unsigned int j=0;j<numMaps;j++)
			{
				auto subMesh = subMeshFactory();
				auto &subVertices = subMesh->GetVertices();
				auto &triangles = subMesh->GetTriangles();
				auto &vertexWeights = subMesh->GetVertexWeights();

				auto texID = Read<unsigned short>();
				subMesh->SetTexture(texID);

				auto numMeshVerts = Read<unsigned long long>();
				std::vector<unsigned long long> meshVertIDs(numMeshVerts);
				for(unsigned long long k=0;k<numMeshVerts;k++)
				{
					auto meshVertID = Read<unsigned long long>();
					meshVertIDs[k] = meshVertID;

					auto &vert = vertices[meshVertID];
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
			mesh->Update(ModelUpdateFlags::All);
			group->AddMesh(mesh);
		}
	}
	if(version >= 0x0004)
	{
		auto &baseMeshes = mdl->GetBaseMeshes();
		auto numBaseMeshes = Read<unsigned short>();
		baseMeshes.reserve(baseMeshes.size() +numBaseMeshes);
		for(unsigned short i=0;i<numBaseMeshes;i++)
		{
			auto meshGroupId = Read<unsigned int>();
			baseMeshes.push_back(meshGroupId);
		}
	}
}

void FWMD::LoadCollisionMeshes(Game *game,unsigned short version,Model *mdl,SurfaceMaterial *smDefault)
{
	if(smDefault == nullptr)
		smDefault = m_gameState->GetSurfaceMaterial(0);
	float mass = Read<float>();
	mdl->SetMass(mass);
	Vector3 collisionMin(std::numeric_limits<float>::max(),std::numeric_limits<float>::max(),std::numeric_limits<float>::max());
	Vector3 collisionMax(std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest(),std::numeric_limits<float>::lowest());
	unsigned char numMeshes = Read<unsigned char>();
	auto massPerMesh = mass /static_cast<float>(numMeshes); // TODO: Allow individual mass per collision mesh
	for(unsigned char i=0;i<numMeshes;i++)
	{
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
		if(version >= 0x0002)
		{
			auto numConstraints = Read<unsigned char>();
			for(unsigned char iConstraint=0;iConstraint<numConstraints;iConstraint++)
			{
				auto type = Read<unsigned char>();
				auto idTgt = Read<unsigned int>();
				auto &joint = mdl->AddJoint(type,i,idTgt);
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
			LoadSoftBodyData(mdl,*mesh);
		//
		mesh->CalculateBounds();
		mdl->AddCollisionMesh(mesh);
	}
	mdl->SetCollisionBounds(collisionMin,collisionMax);
	mdl->UpdateShape();
}

void FWMD::LoadBlendControllers(Model *mdl)
{
	unsigned short numControllers = Read<unsigned short>();
	for(unsigned short i=0;i<numControllers;i++)
	{
		std::string name = ReadString();
		int min = Read<int>();
		int max = Read<int>();
		bool loop = Read<bool>();
		mdl->AddBlendController(name,min,max,loop);
	}
}

void FWMD::LoadIKControllers(uint16_t version,Model *mdl)
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
		auto *controller = mdl->AddIKController(name,chainLength,type,static_cast<util::ik::Method>(method));

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

void FWMD::LoadAnimations(unsigned short version,Model *mdl)
{
	unsigned int numAnimations = Read<unsigned int>();
	for(unsigned int i=0;i<numAnimations;i++)
	{
		std::string name = ReadString();
		FWAD wad;
		auto anim = std::shared_ptr<Animation>(wad.ReadData(version,m_file));
		if(version < 0x0007)
		{
			Vector3 min,max;
			mdl->GetRenderBounds(min,max);
			anim->SetRenderBounds(min,max);
		}
		mdl->AddAnimation(name,anim);
	}

	if(version >= 0x0015)
	{
		auto numVertexAnims = Read<uint32_t>();
		mdl->GetVertexAnimations().reserve(numVertexAnims);
		for(auto i=decltype(numVertexAnims){0};i<numVertexAnims;++i)
		{
			auto name = ReadString();
			auto va = mdl->AddVertexAnimation(name);
			auto numMeshAnims = Read<uint32_t>();
			va->GetMeshAnimations().reserve(numMeshAnims);
			for(auto j=decltype(numMeshAnims){0};j<numMeshAnims;++j)
			{
				auto meshGroupId = Read<uint32_t>();
				auto meshId = Read<uint32_t>();
				auto subMeshId = Read<uint32_t>();

				auto meshGroup = mdl->GetMeshGroup(meshGroupId);
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
					auto numUsedVerts = Read<uint16_t>();
					if(subMesh != nullptr)
					{
						auto meshFrame = va->AddMeshFrame(*mesh,*subMesh);
						for(auto l=decltype(numUsedVerts){0};l<numUsedVerts;++l)
						{
							auto idx = Read<uint16_t>();
							auto v = Read<std::array<uint16_t,3>>();
							meshFrame->SetVertexPosition(idx,v);
						}
					}
					else
					{
						Con::cwar<<"WARNING: Invalid mesh reference in vertex animation '"<<name<<"'! Skipping..."<<Con::endl;
						m_file->Seek(m_file->Tell() +numUsedVerts *(sizeof(uint16_t) +sizeof(std::array<uint16_t,3>)));
					}
				}
			}
		}

		auto numFlexControllers = Read<uint32_t>();
		auto &flexControllers = mdl->GetFlexControllers();
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
		auto &flexes = mdl->GetFlexes();
		for(auto i=decltype(numFlexes){0};i<numFlexes;++i)
		{
			auto name = ReadString();
			flexes.push_back(Flex{name});
			auto &flex = flexes.back();

			auto vaIdx = Read<uint32_t>();
			auto maIdx = Read<uint32_t>();
			auto frIdx = Read<uint32_t>();
			auto &vertAnims = mdl->GetVertexAnimations();
			if(vaIdx < vertAnims.size())
			{
				auto &va = vertAnims.at(vaIdx);
				auto &meshAnims = va->GetMeshAnimations();
				if(maIdx < meshAnims.size())
				{
					auto &ma = meshAnims.at(maIdx);
					auto &frames = ma->GetFrames();
					if(frIdx < frames.size())
					{
						auto &frame = frames.at(frIdx);
						flex.SetVertexAnimation(*va,*ma,*frame);
					}
				}
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

		auto &phonemeMap = mdl->GetPhonemeMap();
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
	}
}

void FWMD::LoadLODData(unsigned short version,Model *mdl)
{
	if(version >= 0x0004)
	{
		auto numLods = Read<unsigned char>();
		for(UChar i=0;i<numLods;i++)
		{
			auto lod = Read<unsigned char>();
			auto numReplace = Read<unsigned char>();
			std::unordered_map<unsigned int,unsigned int> meshReplacements;
			meshReplacements.reserve(numReplace);
			for(UChar i=0;i<numReplace;i++)
			{
				auto origId = Read<unsigned int>();
				auto repId = Read<unsigned int>();
				meshReplacements[origId] = repId;
			}
			mdl->AddLODInfo(lod,meshReplacements);
		}
		return;
	}

	// Version 0x0003 and lower
	auto numLODs = Read<unsigned char>();
	auto &baseMeshes = mdl->GetBaseMeshes();
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
	auto &baseMeshes = mdl->GetBaseMeshes();
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
			LODInfo *info = mdl->AddLODInfo(lod,meshIDs);
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
			mdl->AddLODInfo(lod,meshIDs);
			lodBase = mdl->GetLODInfo(0);
		}
	}*/
	//
	//
}

void FWMD::LoadSoftBodyData(Model *mdl,CollisionMesh &colMesh)
{
	auto bSoftBodyData = Read<bool>();
	if(bSoftBodyData == false)
		return;
	auto meshGroupId = Read<uint32_t>();
	auto meshId = Read<uint32_t>();
	auto subMeshId = Read<uint32_t>();
	colMesh.SetSoftBody(true);
	auto bValid = false;
	auto meshGroup = mdl->GetMeshGroup(meshGroupId);
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

void FWMD::LoadBodygroups(Model *mdl)
{
	auto numBodygroups = Read<unsigned short>();
	for(unsigned short i=0;i<numBodygroups;i++)
	{
		auto name = ReadString();
		auto &bg = mdl->AddBodyGroup(name);
		auto numMeshes = Read<unsigned char>();
		for(unsigned char j=0;j<numMeshes;j++)
		{
			auto meshGroupId = Read<unsigned int>();
			bg.meshGroups.push_back(meshGroupId);
		}
	}
}

