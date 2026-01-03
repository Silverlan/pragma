// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"
#include <cassert>

module pragma.shared;

import :model.model_manager;

#define WMD_VERSION 38

namespace pragma {
	enum class CollisionMeshLoadFlags : uint64_t { None = 0u, SoftBody = 1u, Convex = SoftBody << 1u };
	using namespace pragma::math::scoped_enum::bitwise;
}
REGISTER_ENUM_FLAGS(pragma::CollisionMeshLoadFlags)

struct DLLNETWORK FWMDBone {
	std::string name;
	std::unordered_map<unsigned int, FWMDBone *> children;
};

struct DLLNETWORK FWMDSkeleton {
	std::vector<FWMDBone> bones;
	std::unordered_map<unsigned int, FWMDBone *> hierarchy;
};

struct DLLNETWORK FWMDFaceVertex {
	unsigned long long vertexID;
	Vector3 uvw = {};
};

struct DLLNETWORK FWMDFace {
	FWMDFaceVertex verts[3];
	unsigned int matID;
};

struct DLLNETWORK FWMDTexture {
	FWMDTexture() {};
	FWMDTexture(unsigned int ID, std::string name)
	{
		this->ID = ID;
		this->name = name;
	}
	unsigned int ID;
	std::string name;
};

struct DLLNETWORK FWMDMaterial {
	FWMDMaterial(FWMDTexture diffusemap, FWMDTexture bumpmap)
	{
		this->diffusemap = diffusemap;
		this->bumpmap = bumpmap;
	}
	FWMDTexture diffusemap;
	FWMDTexture bumpmap;
};

struct DLLNETWORK FWMDMesh {
	FWMDMesh(FWMDTexture diffusemap, FWMDTexture bumpmap) : material(diffusemap, bumpmap) {}
	std::vector<Vector3> vertexList;
	std::vector<FWMDFace> faces;
	FWMDMaterial material;
};

struct DLLNETWORK FWMDVertex {
	Vector3 position = {};
	Vector3 normal = {};
	std::unordered_map<unsigned long long, float> weights;
};

pragma::asset::WmdFormatHandler::WmdFormatHandler(util::IAssetManager &assetManager) : IModelFormatHandler {assetManager} { m_gameState = static_cast<ModelManager &>(assetManager).GetNetworkState().GetGameState(); }

bool pragma::asset::WmdFormatHandler::LoadData(ModelProcessor &processor, ModelLoadInfo &info)
{
	auto &f = *m_file;
	std::array<char, 4> hd;
	for(int i = 0; i < 3; i++)
		hd[i] = f.Read<char>();
	auto &mdlName = processor.identifier;
	if(hd[0] != 'W' || hd[1] != 'M' || hd[2] != 'D') {
		Con::CWAR << "Invalid file format for model '" << mdlName << "'!" << Con::endl;
		return false;
	}
	unsigned short ver = f.Read<unsigned short>();
	if(ver > WMD_VERSION) {
		Con::CWAR << "Incompatible model format version " << ver << "!" << Con::endl;
		return false;
	}
	auto flags = f.Read<Model::Flags>();
	m_bStatic = math::is_flag_set(flags, Model::Flags::Static);

	Vector3 eyeOffset {};
	if(ver > 0x0007)
		eyeOffset = f.Read<Vector3>();

	unsigned long long offModelData = f.Read<unsigned long long>();
	// UNUSED(offModelData);
	unsigned long long offMeshes = f.Read<unsigned long long>();
	// UNUSED(offMeshes);
	unsigned long long offLODData = f.Read<unsigned long long>();
	// UNUSED(offLODData);
	if(ver >= 0x0004) {
		auto offBodygroups = f.Read<unsigned long long>();
		// UNUSED(offBodygroups);
	}
	if(ver >= 38)
		f.Read<unsigned long long>(); // Joints
	unsigned long long offCollisionMesh = f.Read<unsigned long long>();
	if(!m_bStatic) {
		f.Seek(f.Tell() + sizeof(unsigned long long) * 2);
		if(ver >= 0x0015)
			f.Seek(f.Tell() + sizeof(uint64_t) * 4u);
		if(ver >= 0x0016)
			f.Seek(f.Tell() + sizeof(uint64_t) * 1u);
		if(ver >= 28)
			f.Seek(f.Tell() + sizeof(uint64_t) * 1u);
		if(ver >= 37)
			f.Seek(f.Tell() + sizeof(uint64_t) * 1u);
	}

	unsigned char numTexturePaths = f.Read<unsigned char>();
	std::vector<std::string> texturePaths;
	texturePaths.reserve(numTexturePaths);
	for(unsigned char i = 0; i < numTexturePaths; i++) {
		std::string texturePath = f.ReadString();
		texturePaths.push_back(texturePath);
	}
	// Bones
	unsigned int numBones = 0;
	if(!m_bStatic)
		numBones = f.Read<unsigned int>();
	auto &mdlManager = static_cast<ModelManager &>(processor.handler->GetAssetManager());
	auto mdl = mdlManager.CreateModel(math::max(numBones, (unsigned int)(1)), mdlName);
	mdl->SetEyeOffset(eyeOffset);
	auto &meta = mdl->GetMetaInfo();
	meta.flags = flags;
	for(auto &path : texturePaths)
		mdl->AddTexturePath(path);
	LoadBones(ver, numBones, *mdl);
	if(!m_bStatic) {
		LoadAttachments(*mdl);
		if(ver >= 0x0017)
			LoadObjectAttachments(*mdl);
		LoadHitboxes(ver, *mdl);
	}

	// Textures
	unsigned short numBaseTextures = f.Read<unsigned short>();
	unsigned short numTextures = f.Read<unsigned short>();
	meta.textures.reserve(numTextures);
	for(unsigned short i = 0; i < numTextures; i++) {
		std::string texture = f.ReadString();
		meta.textures.push_back(texture);
	}

	unsigned short numSkins = f.Read<unsigned short>();
	for(unsigned short i = 0; i < numSkins; i++) {
		TextureGroup *texGroup = mdl->CreateTextureGroup();
		for(unsigned short j = 0; j < numBaseTextures; j++) {
			unsigned short texID = f.Read<unsigned short>();
			texGroup->textures.push_back(texID);
		}
	}

	//
	// Meshes
	LoadMeshes(ver, *mdl, [&mdlManager]() { return mdlManager.CreateMesh(); }, [&mdlManager]() { return mdlManager.CreateSubMesh(); });

	// LOD Data
	LoadLODData(ver, *mdl);

	// Bodygroups
	if(ver >= 0x0004)
		LoadBodygroups(*mdl);

	// Joints
	if(ver >= 38)
		LoadJoints(*mdl);

	// Collision Meshes
	if(offCollisionMesh > 0)
		LoadCollisionMeshes(m_gameState, ver, *mdl);
	else
		mdl->SetCollisionBounds(Vector3(0, 0, 0), Vector3(0, 0, 0));

	if(!m_bStatic) {
		LoadBlendControllers(*mdl);
		LoadIKControllers(ver, *mdl);
		LoadAnimations(ver, *mdl);
	}
	unsigned char numIncludes = f.Read<unsigned char>();
	for(unsigned char i = 0; i < numIncludes; i++) {
		auto inc = f.ReadString();
		meta.includes.push_back(inc);
	}
	m_file.reset();

	model = mdl;
	return true;
}

void pragma::asset::WmdFormatHandler::LoadBones(unsigned short version, unsigned int numBones, Model &mdl)
{
	auto &skeleton = mdl.GetSkeleton();
	auto reference = animation::Animation::Create();
	if(!m_bStatic) {
		reference->ReserveBoneIds(reference->GetBoneCount() + numBones);
		for(unsigned int i = 0; i < numBones; i++) {
			auto *bone = new animation::Bone;
			bone->name = m_file->ReadString();
			skeleton.AddBone(bone);
			reference->AddBoneId(i);
		}
	}
	auto frame = Frame::Create((numBones == 0) ? 1 : numBones);
	if(numBones == 0) {
		auto *root = new animation::Bone;
		root->name = "root";
		unsigned int rootID = skeleton.AddBone(root);
		mdl.SetBindPoseBoneMatrix(0, glm::inverse(umat::identity()));
		auto &rootBones = skeleton.GetRootBones();
		rootBones[0] = skeleton.GetBone(rootID).lock();
		reference->AddBoneId(0);

		Vector3 pos(0, 0, 0);
		auto orientation = uquat::identity();
		frame->SetBonePosition(0, pos);
		frame->SetBoneOrientation(0, orientation);
	}
	else {
		for(unsigned int i = 0; i < numBones; i++) {
			Quat orientation = uquat::identity();
			for(unsigned char j = 0; j < 4; j++)
				orientation[j] = m_file->Read<float>();
			Vector3 pos;
			for(unsigned char j = 0; j < 3; j++)
				pos[j] = m_file->Read<float>();
			frame->SetBonePosition(i, pos);
			frame->SetBoneOrientation(i, orientation);

			if(version <= 0x0001) {
				unsigned char joint = m_file->Read<unsigned char>();
				// UNUSED(joint);
			}
			//trans = glm::gtc::translate(trans,pos);
			//trans = glm::gtc::rotate(trans,orientation);
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
	if(!m_bStatic) {
		unsigned int numRoot = m_file->Read<unsigned int>();
		auto &rootBones = skeleton.GetRootBones();
		for(unsigned int i = 0; i < numRoot; i++) {
			unsigned int rootID = m_file->Read<unsigned int>();
			auto root = skeleton.GetBone(rootID).lock();
			rootBones[rootID] = root;
			LoadChildBones(skeleton, root);
		}
	}
	auto refFrame = Frame::Create(*frame);
	frame->Localize(*reference, skeleton);
	reference->AddFrame(frame);
	mdl.AddAnimation("reference", reference);
	mdl.SetReference(refFrame);
	mdl.GenerateBindPoseMatrices();
}

void pragma::asset::WmdFormatHandler::LoadHitboxes(uint16_t version, Model &mdl)
{
	if(version <= 0x0004) // Hitboxes aren't supported until version 0x0005
		return;
	auto numHitboxes = m_file->Read<uint32_t>();
	for(auto i = decltype(numHitboxes) {0}; i < numHitboxes; ++i) {
		auto boneId = m_file->Read<uint32_t>();
		auto group = m_file->Read<uint32_t>();
		auto min = m_file->Read<Vector3>();
		auto max = m_file->Read<Vector3>();
		mdl.AddHitbox(boneId, static_cast<physics::HitGroup>(group), min, max);
	}
}

void pragma::asset::WmdFormatHandler::LoadObjectAttachments(Model &mdl)
{
	auto numObjectAttachments = m_file->Read<uint32_t>();
	mdl.GetObjectAttachments().reserve(numObjectAttachments);
	for(auto i = decltype(numObjectAttachments) {0u}; i < numObjectAttachments; ++i) {
		auto type = m_file->Read<ObjectAttachment::Type>();
		auto name = m_file->ReadString();
		auto attachment = m_file->ReadString();
		auto numKeyValues = m_file->Read<uint32_t>();
		std::unordered_map<std::string, std::string> keyValues;
		keyValues.reserve(numKeyValues);
		for(auto j = decltype(numKeyValues) {0u}; j < numKeyValues; ++j) {
			auto key = m_file->ReadString();
			auto val = m_file->ReadString();
			keyValues.insert(std::make_pair(key, val));
		}
		mdl.AddObjectAttachment(type, name, attachment, keyValues);
	}
}

void pragma::asset::WmdFormatHandler::LoadAttachments(Model &mdl)
{
	unsigned int numAttachments = m_file->Read<unsigned int>();
	for(unsigned int i = 0; i < numAttachments; i++) {
		std::string name = m_file->ReadString();
		unsigned int boneID = m_file->Read<unsigned int>();
		Vector3 offset;
		offset.x = m_file->Read<float>();
		offset.y = m_file->Read<float>();
		offset.z = m_file->Read<float>();
		EulerAngles angles;
		angles.p = m_file->Read<float>();
		angles.y = m_file->Read<float>();
		angles.r = m_file->Read<float>();
		mdl.AddAttachment(name, boneID, offset, angles);
	}
}

static void clamp_bounds(Vector3 &min, Vector3 &max, unsigned short version)
{
	if(version >= 36)
		return;
	for(auto &v : {&min, &max}) {
		for(uint8_t i = 0; i < 3; ++i) {
			if((*v)[i] == std::numeric_limits<float>::lowest() || (*v)[i] == std::numeric_limits<float>::max())
				(*v)[i] = 0.f;
		}
	}
}

void pragma::asset::WmdFormatHandler::LoadMeshes(unsigned short version, Model &mdl, const std::function<std::shared_ptr<geometry::ModelMesh>()> &meshFactory, const std::function<std::shared_ptr<geometry::ModelSubMesh>()> &subMeshFactory)
{
	Vector3 renderMin, renderMax;
	for(char i = 0; i < 3; i++)
		renderMin[i] = m_file->Read<float>();
	for(char i = 0; i < 3; i++)
		renderMax[i] = m_file->Read<float>();
	clamp_bounds(renderMin, renderMax, version);
	mdl.SetRenderBounds(renderMin, renderMax);
	unsigned int numMeshGroups = m_file->Read<unsigned int>();
	for(unsigned int i = 0; i < numMeshGroups; i++) {
		std::string name = m_file->ReadString();
		auto group = mdl.AddMeshGroup(name);
		uint32_t numMeshes = (version < 30) ? m_file->Read<unsigned char>() : m_file->Read<uint32_t>();
		for(uint32_t j = 0; j < numMeshes; j++) {
			auto mesh = meshFactory();
			if(version >= 35) {
				auto refId = m_file->Read<uint32_t>();
				mesh->SetReferenceId(refId);
			}
			if(version <= 0x0017) {
				unsigned long long numVerts = m_file->Read<unsigned long long>();

				std::vector<FWMDVertex> wmdVerts;
				wmdVerts.reserve(numVerts);
				for(unsigned long long j = 0; j < numVerts; j++) {
					FWMDVertex v;
					v.position.x = m_file->Read<float>();
					v.position.y = m_file->Read<float>();
					v.position.z = m_file->Read<float>();
					v.normal.x = m_file->Read<float>();
					v.normal.y = m_file->Read<float>();
					v.normal.z = m_file->Read<float>();

					wmdVerts.push_back(v);
				}
				unsigned int numBones = m_file->Read<unsigned int>();
				for(unsigned int j = 0; j < numBones; j++) {
					unsigned int boneID = m_file->Read<unsigned int>();
					unsigned long long numVerts = m_file->Read<unsigned long long>();
					for(unsigned long long k = 0; k < numVerts; k++) {
						unsigned long long vertID = m_file->Read<unsigned long long>();
						float weight = m_file->Read<float>();
						if(vertID < wmdVerts.size())
							wmdVerts[vertID].weights.insert(std::unordered_map<unsigned long long, float>::value_type(boneID, weight));
					}
				}

				unsigned int numMaps = m_file->Read<unsigned int>();
				for(unsigned int j = 0; j < numMaps; j++) {
					auto subMesh = subMeshFactory();
					auto &subVertices = subMesh->GetVertices();
					auto &vertexWeights = subMesh->GetVertexWeights();

					auto texID = m_file->Read<unsigned short>();
					subMesh->SetSkinTextureIndex(texID);

					auto numMeshVerts = m_file->Read<unsigned long long>();
					subVertices.reserve(numMeshVerts);

					std::vector<unsigned long long> meshVertIDs(numMeshVerts);
					for(unsigned long long k = 0; k < numMeshVerts; k++) {
						auto meshVertID = m_file->Read<unsigned long long>();
						meshVertIDs[k] = meshVertID;

						auto &vert = wmdVerts[meshVertID];
						subVertices.push_back(math::Vertex {vert.position, vert.normal});
						if(!vert.weights.empty()) {
							vertexWeights.push_back(math::VertexWeight {Vector4i {-1, -1, -1, -1}, Vector4 {0.f, 0.f, 0.f, 0.f}});
							auto &weight = vertexWeights.back();
							int numWeights = 0;
							for(auto m = vert.weights.begin(); m != vert.weights.end(); m++) {
								weight.weights[numWeights] = m->second;
								weight.boneIds[numWeights] = CInt32(m->first);
								numWeights++;
								if(numWeights == 4)
									break;
							}
						}
					}

					std::vector<Vector2> uvs(numMeshVerts);
					m_file->Read(uvs.data(), uvs.size() * sizeof(Vector2));
					if(version <= 0x0005) // Version 5 and lower used OpenGL texture coordinates (lower left origin), they'll have to be flipped first
					{
						for(auto &uv : uvs)
							uv.y = 1.f - uv.y;
					}
					for(auto i = decltype(subVertices.size()) {0}; i < subVertices.size(); ++i) {
						auto &v = subVertices[i];
						v.uv = uvs[i];
					}
					// float w = Read<float>(); // We don't need 3D-Mapping for the time being

					auto numFaces = m_file->Read<unsigned int>();
					for(unsigned int k = 0; k < numFaces; k++) {
						for(int l = 0; l < 3; l++) {
							auto localID = m_file->Read<unsigned long long>();
							//FWMDVertex &vert = vertices[meshVertIDs[localID]];
							subMesh->AddIndex(localID);
						}
					}
					if(subVertices.size() > 0)
						mesh->AddSubMesh(subMesh);
				}
			}
			else {
				auto numSubMeshes = m_file->Read<uint32_t>();
				for(auto i = decltype(numSubMeshes) {0u}; i < numSubMeshes; ++i) {
					auto subMesh = subMeshFactory();

					if(version >= 26) {
						auto pose = m_file->Read<math::ScaledTransform>();
						subMesh->SetPose(pose);
					}

					auto texId = m_file->Read<uint16_t>();
					subMesh->SetSkinTextureIndex(texId);

					if(version >= 27) {
						auto geometryType = m_file->Read<geometry::ModelSubMesh::GeometryType>();
						subMesh->SetGeometryType(geometryType);
					}

					if(version >= 35) {
						auto refId = m_file->Read<uint32_t>();
						subMesh->SetReferenceId(refId);
					}

					auto &verts = subMesh->GetVertices();
					auto &vertWeights = subMesh->GetVertexWeights();

					auto numVerts = m_file->Read<uint64_t>();
					verts.reserve(numVerts);
					for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
						verts.push_back({});
						auto &v = verts.back();
						v.position = m_file->Read<Vector3>();
						v.normal = m_file->Read<Vector3>();
						if(version < 30)
							v.uv = m_file->Read<Vector2>();
					}

					if(version >= 30) {
						auto &uvSets = subMesh->GetUVSets();
						auto numUvSets = m_file->Read<uint8_t>();
						assert(numUvSets > 0); // There always has to be at least one uv set!
						uvSets.reserve(numUvSets - 1);
						for(auto i = decltype(numUvSets) {0u}; i < numUvSets; ++i) {
							auto name = m_file->ReadString();
							if(name == "base") {
								for(auto j = decltype(numVerts) {0u}; j < numVerts; ++j)
									verts.at(j).uv = m_file->Read<Vector2>();
							}
							else {
								auto it = uvSets.insert(std::make_pair(name, std::vector<Vector2> {})).first;
								auto &uvSet = it->second;

								uvSet.resize(numVerts);
								m_file->Read(uvSet.data(), uvSet.size() * sizeof(uvSet.front()));
							}
						}
					}

					auto numVertWeights = m_file->Read<uint64_t>();
					vertWeights.resize(numVertWeights);
					static_assert(sizeof(decltype(vertWeights.front())) == sizeof(Vector4) * 2);
					m_file->Read(vertWeights.data(), vertWeights.size() * sizeof(decltype(vertWeights.front())));

					if(version >= 27) {
						auto numExtVertWeights = m_file->Read<uint64_t>();
						auto &extVertWeights = subMesh->GetExtendedVertexWeights();
						extVertWeights.resize(numExtVertWeights);
						static_assert(sizeof(decltype(extVertWeights.front())) == sizeof(Vector4) * 2);
						m_file->Read(extVertWeights.data(), extVertWeights.size() * sizeof(decltype(extVertWeights.front())));
					}

					if(version >= 30) {
						auto numAlphas = m_file->Read<uint8_t>();
						if(numAlphas > 0) {
							subMesh->SetAlphaCount(numAlphas);
							auto &alphas = subMesh->GetAlphas();
							alphas.resize(numVerts, Vector2 {});
							for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
								alphas.at(i).x = m_file->Read<float>();
								if(numAlphas > 1)
									alphas.at(i).y = m_file->Read<float>();
							}
						}
					}

					uint32_t numIndices = m_file->Read<uint32_t>();
					if(version < 30)
						numIndices *= 3;
					subMesh->SetIndexType(geometry::IndexType::UInt16);
					subMesh->SetIndexCount(numIndices);
					auto &indexData = subMesh->GetIndexData();
					m_file->Read(indexData.data(), indexData.size());

					mesh->AddSubMesh(subMesh);
				}
			}

			group->AddMesh(mesh);
		}
	}

	if(version >= 0x0004) {
		auto &baseMeshes = mdl.GetBaseMeshes();
		auto numBaseMeshes = m_file->Read<unsigned short>();
		baseMeshes.reserve(baseMeshes.size() + numBaseMeshes);
		for(unsigned short i = 0; i < numBaseMeshes; i++) {
			auto meshGroupId = m_file->Read<unsigned int>();
			baseMeshes.push_back(meshGroupId);
		}
	}
}

void pragma::asset::WmdFormatHandler::LoadCollisionMeshes(Game *game, unsigned short version, Model &mdl, physics::SurfaceMaterial *smDefault)
{
	if(smDefault == nullptr)
		smDefault = m_gameState->GetSurfaceMaterial(0);
	float mass = m_file->Read<float>();
	mdl.SetMass(mass);
	Vector3 collisionMin(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
	Vector3 collisionMax(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest());
	uint32_t numMeshes = (version < 30) ? m_file->Read<uint8_t>() : m_file->Read<uint32_t>();
	auto massPerMesh = mass / static_cast<float>(numMeshes); // TODO: Allow individual mass per collision mesh
	std::vector<physics::JointInfo> oldJointSystemJoints;
	for(auto i = decltype(numMeshes) {0u}; i < numMeshes; ++i) {
		auto flags = CollisionMeshLoadFlags::None;
		if(version >= 30)
			flags = m_file->Read<CollisionMeshLoadFlags>();
		int boneParent = m_file->Read<int>();
		if(boneParent == -1)
			boneParent = 0;
		Vector3 origin(0.f, 0.f, 0.f);
		if(version >= 0x0002) {
			origin.x = m_file->Read<float>();
			origin.y = m_file->Read<float>();
			origin.z = m_file->Read<float>();
		}
		auto *matSurface = smDefault;
		std::string surfaceMaterial;
		if(version >= 0x0003)
			surfaceMaterial = m_file->ReadString();
		auto *matCustom = m_gameState->GetSurfaceMaterial(surfaceMaterial);
		if(matCustom != nullptr && matCustom->GetIndex() != 0)
			matSurface = matCustom;
		for(char j = 0; j < 3; j++)
			collisionMin[j] = math::min(collisionMin[j], m_file->Read<float>() - origin[j]);
		for(char j = 0; j < 3; j++)
			collisionMax[j] = math::max(collisionMax[j], m_file->Read<float>() - origin[j]);
		auto mesh = physics::CollisionMesh::Create(game);
		if(matSurface != nullptr)
			mesh->SetSurfaceMaterial(CInt32(matSurface->GetIndex()));
		mesh->SetMass(massPerMesh);
		mesh->SetOrigin(origin);
		std::vector<Vector3> &colVerts = mesh->GetVertices();
		mesh->SetBoneParent(boneParent);
		mesh->SetConvex(math::is_flag_set(flags, CollisionMeshLoadFlags::Convex));

		auto numVerts = m_file->Read<unsigned long long>();
		colVerts.resize(numVerts);
		m_file->Read(colVerts.data(), numVerts * sizeof(colVerts.front()));

		if(version >= 0x0010) {
			auto &triangles = mesh->GetTriangles();

			auto numTriangles = m_file->Read<uint64_t>();
			triangles.resize(numTriangles);
			m_file->Read(triangles.data(), numTriangles * sizeof(uint16_t));
			if(version >= 0x0011) {
				auto volume = m_file->Read<double>();
				auto com = m_file->Read<Vector3>();
				mesh->SetVolume(volume);
				mesh->SetCenterOfMass(com);
			}
			else
				mesh->CalculateVolumeAndCom();
		}
		if(version >= 0x0002 && version < 38) {
			auto numConstraints = m_file->Read<unsigned char>();
			for(unsigned char iConstraint = 0; iConstraint < numConstraints; iConstraint++) {
				auto type = m_file->Read<physics::JointType>();
				auto idTgt = m_file->Read<unsigned int>();
				oldJointSystemJoints.push_back(physics::JointInfo {type, static_cast<uint16_t>(i), static_cast<uint16_t>(idTgt)});
				auto &joint = oldJointSystemJoints.back();
				joint.collide = m_file->Read<bool>();
				auto numArgs = m_file->Read<unsigned char>();
				for(unsigned char i = 0; i < numArgs; i++) {
					auto k = m_file->ReadString();
					auto v = m_file->ReadString();
					joint.args[k] = v;
				}
			}
		}
		// Version 0x0014
		if(version >= 0x0014) {
			auto bSoftBodyData = (version < 30) ? m_file->Read<bool>() : math::is_flag_set(flags, CollisionMeshLoadFlags::SoftBody);
			if(bSoftBodyData)
				LoadSoftBodyData(mdl, *mesh);
		}
		//
		mesh->CalculateBounds();
		mdl.AddCollisionMesh(mesh);
	}
	mdl.SetCollisionBounds(collisionMin, collisionMax);
	mdl.UpdateShape();

	// Old joint system, where joints referenced collision meshes instead of bones.
	// We'll translate it into the new system here
	auto &colMeshes = mdl.GetCollisionMeshes();
	for(auto &jointInfo : oldJointSystemJoints) {
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
		auto &jointInfoMdl = mdl.AddJoint(jointInfo.type, srcBoneId, dstBoneId);
		jointInfoMdl.args = jointInfo.args;
		jointInfoMdl.collide = jointInfo.collide;
	}
}

void pragma::asset::WmdFormatHandler::LoadBlendControllers(Model &mdl)
{
	unsigned short numControllers = m_file->Read<unsigned short>();
	for(unsigned short i = 0; i < numControllers; i++) {
		std::string name = m_file->ReadString();
		int min = m_file->Read<int>();
		int max = m_file->Read<int>();
		bool loop = m_file->Read<bool>();
		mdl.AddBlendController(name, min, max, loop);
	}
}

void pragma::asset::WmdFormatHandler::LoadIKControllers(uint16_t version, Model &mdl)
{
	if(version < 0x0016)
		return;
	auto numIkControllers = m_file->Read<uint32_t>();
	for(auto i = decltype(numIkControllers) {0}; i < numIkControllers; ++i) {
		auto name = m_file->ReadString();
		auto type = m_file->ReadString();
		auto chainLength = m_file->Read<uint32_t>();
		auto method = m_file->Read<uint32_t>();
		auto *controller = mdl.AddIKController(name, chainLength, type, static_cast<physics::ik::Method>(method));

		auto *keyValues = (controller != nullptr) ? &controller->GetKeyValues() : nullptr;
		auto numKeyValues = m_file->Read<uint32_t>();
		for(auto j = decltype(numKeyValues) {0}; j < numKeyValues; ++j) {
			auto key = m_file->ReadString();
			auto val = m_file->ReadString();
			if(keyValues != nullptr)
				(*keyValues)[key] = val;
		}
	}
}

void pragma::asset::WmdFormatHandler::LoadAnimations(unsigned short version, Model &mdl)
{
	unsigned int numAnimations = m_file->Read<unsigned int>();
	for(unsigned int i = 0; i < numAnimations; i++) {
		std::string name = m_file->ReadString();
		FWAD wad;
		auto anim = std::shared_ptr<animation::Animation>(wad.ReadData(version, *m_file));
		if(anim) {
			if(version < 0x0007) {
				Vector3 min, max;
				mdl.GetRenderBounds(min, max);
				clamp_bounds(min, max, version);
				anim->SetRenderBounds(min, max);
			}
			mdl.AddAnimation(name, anim);
		}
	}

	if(version >= 0x0015) {
		auto numVertexAnims = m_file->Read<uint32_t>();
		mdl.GetVertexAnimations().reserve(numVertexAnims);
		for(auto i = decltype(numVertexAnims) {0}; i < numVertexAnims; ++i) {
			auto name = m_file->ReadString();
			auto va = mdl.AddVertexAnimation(name);
			auto numMeshAnims = m_file->Read<uint32_t>();
			va->GetMeshAnimations().reserve(numMeshAnims);
			for(auto j = decltype(numMeshAnims) {0}; j < numMeshAnims; ++j) {
				auto meshGroupId = m_file->Read<uint32_t>();
				auto meshId = m_file->Read<uint32_t>();
				auto subMeshId = m_file->Read<uint32_t>();

				auto meshGroup = mdl.GetMeshGroup(meshGroupId);
				auto &meshes = meshGroup->GetMeshes();
				std::shared_ptr<geometry::ModelMesh> mesh = (meshId < meshes.size()) ? meshes.at(meshId) : nullptr;
				std::shared_ptr<geometry::ModelSubMesh> subMesh = nullptr;
				if(mesh != nullptr) {
					auto &subMeshes = mesh->GetSubMeshes();
					if(subMeshId < subMeshes.size())
						subMesh = subMeshes.at(subMeshId);
				}
				auto numFrames = m_file->Read<uint32_t>();
				for(auto k = decltype(numFrames) {0}; k < numFrames; ++k) {
					uint64_t endOfFrameOffset = 0;
					if(version >= 32)
						endOfFrameOffset = m_file->Read<uint64_t>();
					auto flags = animation::MeshVertexFrame::Flags::None;
					if(version >= 25)
						flags = m_file->Read<animation::MeshVertexFrame::Flags>();

					if(version >= 32) {
						if(subMesh == nullptr) {
							m_file->Seek(endOfFrameOffset);
							Con::CWAR << "Invalid mesh reference in vertex animation '" << name << "'! Skipping..." << Con::endl;
						}
						else {
							auto meshFrame = va->AddMeshFrame(*mesh, *subMesh);
							meshFrame->SetFlags(flags);

							auto numUsedVerts = m_file->Read<uint16_t>();
							std::vector<uint16_t> usedVertIndices {};
							usedVertIndices.resize(numUsedVerts);
							m_file->Read(usedVertIndices.data(), usedVertIndices.size() * sizeof(usedVertIndices.front()));

							auto numAttributes = m_file->Read<uint16_t>();
							for(auto i = decltype(numAttributes) {0u}; i < numAttributes; ++i) {
								auto attrName = m_file->ReadString();
								std::vector<std::array<uint16_t, 4>> vdata {};
								vdata.resize(numUsedVerts);
								m_file->Read(vdata.data(), vdata.size() * sizeof(vdata.front()));
								if(attrName == "position") {
									for(auto j = decltype(numUsedVerts) {0u}; j < numUsedVerts; ++j) {
										auto vertIdx = usedVertIndices.at(j);
										meshFrame->SetVertexPosition(vertIdx, vdata.at(j));
									}
								}
								else if(attrName == "normal") {
									for(auto j = decltype(numUsedVerts) {0u}; j < numUsedVerts; ++j) {
										auto vertIdx = usedVertIndices.at(j);
										meshFrame->SetVertexNormal(vertIdx, vdata.at(j));
									}
								}
							}
						}
					}
					else {
						auto numUsedVerts = m_file->Read<uint16_t>();
						if(subMesh != nullptr) {
							auto meshFrame = va->AddMeshFrame(*mesh, *subMesh);
							meshFrame->SetFlags(flags);
							for(auto l = decltype(numUsedVerts) {0}; l < numUsedVerts; ++l) {
								auto idx = m_file->Read<uint16_t>();
								auto v = m_file->Read<std::array<uint16_t, 3>>();
								meshFrame->SetVertexPosition(idx, v);
								if(math::is_flag_set(flags, animation::MeshVertexFrame::Flags::HasDeltaValues)) {
									auto deltaVal = m_file->Read<uint16_t>();
									meshFrame->SetDeltaValue(idx, deltaVal);
								}
							}
						}
						else {
							Con::CWAR << "Invalid mesh reference in vertex animation '" << name << "'! Skipping..." << Con::endl;
							auto szPerVertex = sizeof(uint16_t) * 3;
							if(math::is_flag_set(flags, animation::MeshVertexFrame::Flags::HasDeltaValues))
								szPerVertex += sizeof(uint16_t);
							m_file->Seek(m_file->Tell() + numUsedVerts * szPerVertex);
						}
					}
				}
			}
		}

		auto numFlexControllers = m_file->Read<uint32_t>();
		auto &flexControllers = mdl.GetFlexControllers();
		flexControllers.reserve(numFlexControllers);
		for(auto i = decltype(numFlexControllers) {0}; i < numFlexControllers; ++i) {
			flexControllers.push_back({});
			auto &fc = flexControllers.back();
			fc.name = m_file->ReadString();
			fc.min = m_file->Read<float>();
			fc.max = m_file->Read<float>();
		}

		auto numFlexes = m_file->Read<uint32_t>();
		auto &flexes = mdl.GetFlexes();
		for(auto i = decltype(numFlexes) {0}; i < numFlexes; ++i) {
			auto name = m_file->ReadString();
			flexes.push_back(animation::Flex {name});
			auto &flex = flexes.back();

			auto vaIdx = m_file->Read<uint32_t>();
			uint32_t frameIndex = 0;
			if(version < 31)
				m_file->Seek(m_file->Tell() + sizeof(uint32_t) * 2);
			else
				frameIndex = m_file->Read<uint32_t>();
			auto &vertAnims = mdl.GetVertexAnimations();
			if(vaIdx < vertAnims.size()) {
				auto &va = vertAnims.at(vaIdx);
				flex.SetVertexAnimation(*va, frameIndex);
			}

			auto &ops = flex.GetOperations();
			auto numOps = m_file->Read<uint32_t>();
			ops.reserve(numOps);
			for(auto j = decltype(numOps) {0}; j < numOps; ++j) {
				ops.push_back({});
				auto &op = ops.back();
				op.type = static_cast<animation::Flex::Operation::Type>(m_file->Read<uint32_t>());
				op.d.value = m_file->Read<float>();
			}
		}

		auto &phonemeMap = mdl.GetPhonemeMap();
		auto numPhonemes = m_file->Read<uint32_t>();
		phonemeMap.phonemes.reserve(numPhonemes);
		for(auto i = decltype(numPhonemes) {0}; i < numPhonemes; ++i) {
			auto name = m_file->ReadString();
			auto &flexControllerValues = (*phonemeMap.phonemes.insert(std::make_pair(name, PhonemeInfo {})).first).second.flexControllers;
			auto numFlexControllers = m_file->Read<uint32_t>();
			flexControllerValues.reserve(numFlexControllers);
			for(auto j = decltype(numFlexControllers) {0}; j < numFlexControllers; ++j) {
				auto controllerId = m_file->Read<uint32_t>();
				auto val = m_file->Read<float>();
				auto controllerName = (controllerId < flexControllers.size()) ? flexControllers.at(controllerId).name : "";
				flexControllerValues.insert(std::make_pair(controllerName, val));
			}
		}

		if(version >= 28) {
			auto maxEyeDeflection = m_file->Read<float>();
			mdl.SetMaxEyeDeflection(maxEyeDeflection);
			auto &eyeballs = mdl.GetEyeballs();
			auto numEyeballs = m_file->Read<uint32_t>();
			eyeballs.reserve(numEyeballs);
			for(auto i = decltype(numEyeballs) {0u}; i < numEyeballs; ++i) {
				eyeballs.push_back({});
				auto &eyeball = eyeballs.back();
				eyeball.name = m_file->ReadString();
				m_file->Read(reinterpret_cast<uint8_t *>(&eyeball) + sizeof(std::string), sizeof(Eyeball) - sizeof(std::string));
			}
		}

		if(version >= 37) {
			auto numFlexAnims = m_file->Read<uint32_t>();
			auto &flexAnims = mdl.GetFlexAnimations();
			auto &flexAnimNames = mdl.GetFlexAnimationNames();
			flexAnimNames.reserve(numFlexAnims);
			flexAnims.reserve(numFlexAnims);
			for(auto i = decltype(numFlexAnims) {0u}; i < numFlexAnims; ++i) {
				auto name = m_file->ReadString();
				auto flexAnim = FlexAnimation::Load(*m_file);
				flexAnimNames.push_back(name);
				flexAnims.push_back(flexAnim);
			}
		}
	}
}

void pragma::asset::WmdFormatHandler::LoadLODData(unsigned short version, Model &mdl)
{
	if(version >= 0x0004) {
		auto numLods = m_file->Read<unsigned char>();
		for(UChar i = 0; i < numLods; i++) {
			auto lod = m_file->Read<unsigned char>();
			float dist;
			if(version >= 36)
				dist = m_file->Read<float>();
			else
				dist = (i + 1) * 500.f;
			auto numReplace = m_file->Read<unsigned char>();
			std::unordered_map<unsigned int, unsigned int> meshReplacements;
			meshReplacements.reserve(numReplace);
			for(UChar i = 0; i < numReplace; i++) {
				auto origId = m_file->Read<unsigned int>();
				auto repId = m_file->Read<unsigned int>();
				meshReplacements[origId] = repId;
			}
			mdl.AddLODInfo(lod, dist, meshReplacements);
		}
		return;
	}

	// Version 0x0003 and lower
	auto numLODs = m_file->Read<unsigned char>();
	auto &baseMeshes = mdl.GetBaseMeshes();
	for(UChar i = 0; i < numLODs; i++) {
		auto lod = m_file->Read<unsigned char>();
		auto numMeshes = m_file->Read<unsigned char>();
		for(UChar j = 0; j < numMeshes; j++) {
			auto meshGroupID = m_file->Read<unsigned int>();
			if(lod == 0)
				baseMeshes.push_back(meshGroupID); // Old format doesn't have base meshes stored separately, so just grab them from LOD 0
		}
		auto numMeshesHidden = m_file->Read<unsigned char>();
		for(UChar j = 0; j < numMeshesHidden; j++)
			m_file->Read<unsigned int>();
	}

	// Obsolete, because incompatible with new format (LODs won't work for these models)
	/*unsigned char numLODs = Read<unsigned char>();
	auto &baseMeshes = mdl.GetBaseMeshes();
	LODInfo *lodBase = nullptr;
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
		else if(lodBase != nullptr)
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

void pragma::asset::WmdFormatHandler::LoadChildBones(const animation::Skeleton &skeleton, std::shared_ptr<animation::Bone> bone)
{
	unsigned int numChildren = m_file->Read<unsigned int>();
	for(unsigned int i = 0; i < numChildren; i++) {
		unsigned int boneID = m_file->Read<unsigned int>();
		auto child = skeleton.GetBone(boneID).lock();
		bone->children[boneID] = child;
		child->parent = bone;
		LoadChildBones(skeleton, child);
	}
}

void pragma::asset::WmdFormatHandler::LoadSoftBodyData(Model &mdl, physics::CollisionMesh &colMesh)
{
	auto meshGroupId = m_file->Read<uint32_t>();
	auto meshId = m_file->Read<uint32_t>();
	auto subMeshId = m_file->Read<uint32_t>();
	colMesh.SetSoftBody(true);
	auto bValid = false;
	auto meshGroup = mdl.GetMeshGroup(meshGroupId);
	if(meshGroup != nullptr) {
		auto &meshes = meshGroup->GetMeshes();
		if(meshId < meshes.size()) {
			auto &subMeshes = meshes.at(meshId)->GetSubMeshes();
			if(subMeshId < subMeshes.size()) {
				auto &subMesh = subMeshes.at(subMeshId);
				colMesh.SetSoftBodyMesh(*subMesh);
				bValid = true;
			}
		}
	}
	auto &triangleIndices = *colMesh.GetSoftBodyTriangles();
	auto &anchors = *colMesh.GetSoftBodyAnchors();
	auto &sbInfo = *colMesh.GetSoftBodyInfo();
	m_file->Read(&sbInfo, sizeof(float) * 19 + sizeof(uint32_t) * 2);
	sbInfo.materialStiffnessCoefficient.clear();
	auto numMaterials = m_file->Read<uint32_t>();
	for(auto i = decltype(numMaterials) {0}; i < numMaterials; ++i) {
		auto matId = m_file->Read<uint32_t>();
		auto it = sbInfo.materialStiffnessCoefficient.find(matId);
		if(it == sbInfo.materialStiffnessCoefficient.end())
			it = sbInfo.materialStiffnessCoefficient.insert(std::make_pair(matId, physics::PhysSoftBodyInfo::MaterialStiffnessCoefficient {})).first;
		it->second.linear = m_file->Read<float>();
		it->second.angular = m_file->Read<float>();
		it->second.volume = m_file->Read<float>();
	}

	auto numTriangles = m_file->Read<uint32_t>();
	triangleIndices.reserve(numTriangles);
	m_file->Read(triangleIndices.data(), triangleIndices.size() * sizeof(triangleIndices.front()));

	auto numAnchors = m_file->Read<uint32_t>();
	anchors.resize(numAnchors);
	m_file->Read(anchors.data(), anchors.size() * sizeof(anchors.front()));

	if(bValid == false)
		colMesh.SetSoftBody(false);
}

void pragma::asset::WmdFormatHandler::LoadJoints(Model &mdl)
{
	auto numJoints = m_file->Read<uint32_t>();
	for(auto i = decltype(numJoints) {0u}; i < numJoints; ++i) {
		auto type = m_file->Read<physics::JointType>();
		auto child = m_file->Read<animation::BoneId>();
		auto parent = m_file->Read<animation::BoneId>();
		auto &joint = mdl.AddJoint(type, child, parent);
		joint.collide = m_file->Read<bool>();
		auto numArgs = m_file->Read<uint8_t>();
		for(auto i = decltype(numArgs) {0u}; i < numArgs; ++i) {
			auto k = m_file->ReadString();
			auto v = m_file->ReadString();
			joint.args[k] = v;
		}
	}
}

void pragma::asset::WmdFormatHandler::LoadBodygroups(Model &mdl)
{
	auto numBodygroups = m_file->Read<unsigned short>();
	for(unsigned short i = 0; i < numBodygroups; i++) {
		auto name = m_file->ReadString();
		auto &bg = mdl.AddBodyGroup(name);
		auto numMeshes = m_file->Read<unsigned char>();
		for(unsigned char j = 0; j < numMeshes; j++) {
			auto meshGroupId = m_file->Read<unsigned int>();
			bg.meshGroups.push_back(meshGroupId);
		}
	}
}
