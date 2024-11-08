/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "asset/fbx_loader.hpp"
#include <panima/animation.hpp>
#include <panima/channel.hpp>
#include <pragma/model/c_modelmesh.h>
#include <pragma/model/c_model.h>
#include <pragma/model/animation/bone.hpp>
#include <pragma/game/game_resources.hpp>
#include <pragma/asset/util_asset.hpp>
#include <cmaterial.h>
#include <ofbx.h>
#include <util_image.hpp>
#include <fsys/ifile.hpp>
#include "pragma/asset/c_util_model.hpp"

using namespace pragma::asset::fbx;
extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;

static std::string_view to_string_view(ofbx::DataView data) { return std::string_view {(const char *)data.begin, (const char *)data.end}; }

static std::optional<std::string> import_texture(const std::string &mdlPath, const std::string &mdlName, const std::string &texPath, bool isAbsPath)
{
	auto fullTexFilePath = util::FilePath(texPath);
	if(!isAbsPath)
		fullTexFilePath = mdlPath + fullTexFilePath;
	std::string convertPath = util::CONVERT_PATH;
	auto matPath = util::DirPath(pragma::asset::get_asset_root_directory(pragma::asset::Type::Material), "models");
	auto texFileName = ufile::get_file_from_filename(texPath);
	auto fullOutputPath = util::DirPath(convertPath, matPath);
	auto ext = fullTexFilePath.GetFileExtension();
	if(ext && (*ext == "dds" || *ext == "ktx")) {
		auto srcPath = fullTexFilePath.GetString();
		auto dstPath = fullOutputPath + std::string {fullTexFilePath.GetFileName()};
		if(isAbsPath) {
			auto absFullPath = (util::get_program_path() + dstPath).GetString();
			auto res = filemanager::copy_system_file(srcPath, absFullPath);
			return res ? absFullPath : std::optional<std::string> {};
		}
		auto res = filemanager::copy_file(srcPath, dstPath.GetString());
		return res ? dstPath.GetString() : std::optional<std::string> {};
	}
	auto f = filemanager::open_file(fullTexFilePath.GetString(), filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f)
		f = filemanager::open_system_file(fullTexFilePath.GetString(), filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f)
		return {};
	fsys::File file {f};
	auto img = uimg::load_image(file);
	if(!img)
		return {};
	auto greyScale = false;
	auto normalMap = false;
	auto alphaMode = AlphaMode::Opaque;
	auto texInfo = pragma::asset::get_texture_info(greyScale, normalMap, alphaMode);

	const auto &importExtensions = pragma::asset::get_supported_extensions(pragma::asset::Type::Texture, pragma::asset::FormatType::All);
	fullOutputPath += util::DirPath(mdlName);
	auto outputFilePath = fullOutputPath + texFileName;
	outputFilePath.RemoveFileExtension(importExtensions);
	auto res = c_game->SaveImage(*img, outputFilePath.GetString(), texInfo);
	return res ? outputFilePath.GetString() : std::optional<std::string> {};
}

static umath::ScaledTransform get_transform(const ofbx::DMatrix &fbxMat)
{
	Mat4 mat;
	for(size_t x = 0; x < 4; ++x) {
		for(size_t y = 0; y < 4; ++y) {
			auto i = x * 4 + y;
			mat[x][y] = fbxMat.m[i];
		}
	}
	return umath::ScaledTransform {mat};
}

static bool is_const_curve(const ofbx::AnimationCurve *curve)
{
	if(!curve)
		return true;
	if(curve->getKeyCount() <= 1)
		return true;
	const float *values = curve->getKeyValue();
	if(curve->getKeyCount() == 2 && fabsf(values[1] - values[0]) < 1e-6)
		return true;
	return false;
}

FbxImporter::~FbxImporter() { m_fbxScene->destroy(); }

bool FbxImporter::LoadMaterial(const ofbx::Material &mat, std::string &outErr)
{
	auto importTexture = [this, &mat](ofbx::Texture::TextureType etex) -> std::optional<std::string> {
		auto *tex = mat.getTexture(etex);
		if(!tex)
			return {};
		ofbx::DataView filename = tex->getRelativeFileName();
		auto isAbsPath = false;
		if(filename == "") {
			filename = tex->getFileName();
			isAbsPath = true;
		}
		auto path = util::FilePath(std::string {to_string_view(filename)});
		return ::import_texture(m_mdlPath, m_mdlName, path.GetString(), isAbsPath);
	};
	auto texPath = importTexture(ofbx::Texture::DIFFUSE);
	if(texPath) {
		std::string fbxMatName = mat.name;
		auto matPath = *texPath;
		if(!fbxMatName.empty()) {
			matPath = ufile::get_path_from_filename(matPath);
			matPath += fbxMatName;
		}
		else
			ufile::remove_extension_from_filename(matPath, pragma::asset::get_supported_extensions(pragma::asset::Type::Texture, pragma::asset::FormatType::All));
		auto mat = client->CreateMaterial(matPath, "pbr");
		auto *cmat = static_cast<CMaterial *>(mat.get());

		auto &dataBlock = mat->GetDataBlock();

		auto relTexPath = util::FilePath(*texPath);
		relTexPath.MakeRelative(util::CONVERT_PATH);
		relTexPath.MakeRelative(std::string {pragma::asset::get_asset_root_directory(pragma::asset::Type::Material)});
		cmat->SetTexture(Material::ALBEDO_MAP_IDENTIFIER, relTexPath.GetString());

		mat->UpdateTextures();

		std::string err;
		mat->Save(matPath, err, true);

		auto idx = m_model->AddMaterial(0, mat.get());
	}
	return true;
}

std::optional<std::string> FbxImporter::Finalize(std::string &outErr)
{
	m_model->Update(ModelUpdateFlags::All);
	std::string mdlPath = "fbx_test";
	auto fullMdlPath = util::FilePath(get_asset_root_directory(Type::Model), mdlPath);
	if(!m_model->Save(*c_game, fullMdlPath.GetString(), outErr))
		return {};
	return mdlPath;
}

bool FbxImporter::LoadMeshes(std::string &outErr)
{
	int mesh_count = m_fbxScene->getMeshCount();
	auto mg = m_model->AddMeshGroup("reference");
	for(int mesh_idx = 0; mesh_idx < mesh_count; ++mesh_idx) {
		const ofbx::Mesh &fbxMesh = *m_fbxScene->getMesh(mesh_idx);
		const ofbx::GeometryData &geom = fbxMesh.getGeometryData();
		const ofbx::Vec3Attributes positions = geom.getPositions();
		const ofbx::Vec3Attributes normals = geom.getNormals();
		const ofbx::Vec2Attributes uvs = geom.getUVs();
		auto tangents = geom.getTangents();

		std::vector<umath::VertexWeight> vertWeights {};
		std::vector<umath::VertexWeight> extendedVertWeights {};
		auto initVertexWeights = [&positions](std::vector<umath::VertexWeight> &vws) {
			if(!vws.empty())
				return;
			vws.resize(positions.values_count);
			for(auto &vw : vws) {
				for(size_t i = 0; i < 4; ++i)
					vw.boneIds[i] = -1;
			}
		};

		auto meshBone = AddBone(fbxMesh);
		FillSkinInfo(vertWeights, &fbxMesh, meshBone->ID);

		auto numMats = fbxMesh.getMaterialCount();
		if(numMats == 0) {

			auto idx = m_model->AddMaterial(0, client->LoadMaterial("white"));
		}
		for(auto i = decltype(numMats) {0u}; i < numMats; ++i) {
			auto *fbxMat = fbxMesh.getMaterial(i);
			if(!fbxMat)
				continue;
			std::string err;
			LoadMaterial(*fbxMat, err);
		}

		// each ofbx::Mesh can have several materials == partitions
		auto mesh = c_game->CreateModelMesh();
		for(int partition_idx = 0; partition_idx < geom.getPartitionCount(); ++partition_idx) {
			//fprintf(fp, "o obj%d_%d\ng grp%d\n", mesh_idx, partition_idx, mesh_idx);
			const ofbx::GeometryPartition &partition = geom.getPartition(partition_idx);

			auto subMesh = c_game->CreateModelSubMesh();
			auto &verts = subMesh->GetVertices();

			// partitions most likely have several polygons, they are not triangles necessarily, use ofbx::triangulate if you want triangles
			for(int polygon_idx = 0; polygon_idx < partition.polygon_count; ++polygon_idx) {
				const ofbx::GeometryPartition::Polygon &polygon = partition.polygons[polygon_idx];

				/*for(int i = polygon.from_vertex; i < polygon.from_vertex + polygon.vertex_count; ++i) {
					umath::Vertex v {};
					ofbx::Vec3 pos = positions.get(i);
					v.position = {pos.x, pos.y, pos.z};
					if(normals.values != nullptr) {
						ofbx::Vec3 n = normals.get(i);
						v.normal = {n.x, n.y, n.z};
					}
					if(uvs.values != nullptr) {
						ofbx::Vec2 uv = uvs.get(i);
						v.uv = {uv.x, uv.y};
					}
					if(tangents.values != nullptr) {
						ofbx::Vec3 t = tangents.get(i);
						v.tangent = {t.x, t.y, t.z, 1.f  handedness };
					}
					subMesh->AddVertex(v);
					if(i < vertWeights.size()) {
						auto &vw = vertWeights[positions.indices[i]];
						subMesh->SetVertexWeight(i, vw);
					}
				}*/

				std::vector<int> triIndices;
				triIndices.resize(3 * (polygon.vertex_count - 2));
				auto numIndices = ofbx::triangulate(geom, polygon, triIndices.data());

				auto startIdx = subMesh->GetVertexCount();
				for(size_t i = 0; i < numIndices; ++i) {
					auto idx = triIndices[i];
					umath::Vertex v {};
					ofbx::Vec3 pos = positions.get(idx);
					v.position = {pos.x, pos.y, pos.z};
					if(normals.values != nullptr) {
						ofbx::Vec3 n = normals.get(idx);
						v.normal = {n.x, n.y, n.z};
					}
					if(uvs.values != nullptr) {
						ofbx::Vec2 uv = uvs.get(idx);
						v.uv = {uv.x, uv.y};
					}
					if(tangents.values != nullptr) {
						ofbx::Vec3 t = tangents.get(idx);
						v.tangent = {t.x, t.y, t.z, 1.f /* handedness */};
					}
					auto vertIdx = subMesh->AddVertex(v);
					if(!vertWeights.empty()) {
						auto &vw = vertWeights[positions.indices[idx]];
						subMesh->SetVertexWeight(vertIdx, vw);
					}
				}

				for(size_t i = 0; i < numIndices; i += 3) {
					subMesh->AddTriangle(startIdx, startIdx + 1, startIdx + 2);
				}
			}
			mesh->AddSubMesh(subMesh);
		}

		mg->AddMesh(mesh);
	}
	return true;
}

bool FbxImporter::LoadAnimations(std::string &outErr)
{
	auto &skel = m_model->GetSkeleton();
	auto &ref = m_model->GetReference();
	auto refAnimFrame = Frame::Create(ref);
	//refAnimFrame->Localize(skel);
	ref.Globalize(skel);
	auto refAnim = pragma::animation::Animation::Create();
	auto numBones = skel.GetBoneCount();
	refAnim->ReserveBoneIds(numBones);
	for(auto i = decltype(numBones) {0u}; i < numBones; ++i)
		refAnim->AddBoneId(i);
	refAnim->AddFrame(refAnimFrame);
	m_model->AddAnimation("reference", refAnim);

	for(int i = 0, n = m_fbxScene->getAnimationStackCount(); i < n; ++i) {
		const ofbx::AnimationStack *stack = m_fbxScene->getAnimationStack(i);
		const ofbx::TakeInfo *take_info = m_fbxScene->getTakeInfo(stack->name);
		std::string name;
		if(take_info) {
			if(take_info->name.begin != take_info->name.end) {
				name = to_string_view(take_info->name);
			}
			if(name.empty() && take_info->filename.begin != take_info->filename.end) {
				auto tmp = to_string_view(take_info->filename);
				name = util::DirPath(tmp).GetFileName();
			}
			if(name.empty())
				name = "anim";
		}
		else {
			name = "";
		}

		const ofbx::AnimationLayer *anim_layer = stack->getLayer(0);
		if(!anim_layer || !anim_layer->getCurveNode(0)) {
			//m_animations.pop();
			continue;
		}

		bool data_found = false;
		for(int k = 0; anim_layer->getCurveNode(k); ++k) {
			const ofbx::AnimationCurveNode *node = anim_layer->getCurveNode(k);
			auto boneLinkProperty = node->getBoneLinkProperty();
			// TODO: Scale?
			auto isBoneTranslation = (boneLinkProperty == "Lcl Translation");
			auto isBoneRotation = (boneLinkProperty == "Lcl Rotation");
			auto isBoneScaling = (boneLinkProperty == "Lcl Scaling");
			if(isBoneTranslation || isBoneRotation || isBoneScaling) {
				auto *fbxBone = node->getBone();
				auto bone = fbxBone ? AddBone(*fbxBone) : nullptr;
				if(bone) {
					auto *curve0 = node->getCurve(0);
					auto *curve1 = node->getCurve(1);
					auto *curve2 = node->getCurve(2);
					// TODO
					if(!is_const_curve(curve0) && !is_const_curve(curve1) && !is_const_curve(curve2)) {

						auto numKeys0 = curve0->getKeyCount();
						auto *times0 = curve0->getKeyTime();
						auto *values0 = curve0->getKeyValue();

						auto numKeys1 = curve0->getKeyCount();
						auto *times1 = curve1->getKeyTime();
						auto *values1 = curve1->getKeyValue();

						auto numKeys2 = curve0->getKeyCount();
						auto *times2 = curve2->getKeyTime();
						auto *values2 = curve2->getKeyValue();

						auto calcInterpolatedValue = [](size_t count, const ofbx::i64 *times, const float *values, ofbx::i64 timestamp) {
							// If the timestamp is outside the range, clamp to the nearest value
							if(timestamp <= times[0])
								return values[0];
							if(timestamp >= times[count - 1])
								return values[count - 1];

							// Find the index of the first element in `times` that is greater than `timestamp`
							const ofbx::i64 *upper = std::lower_bound(times, times + count, timestamp);
							int i = int(upper - times) - 1;

							// Linear interpolation formula
							float t = float(timestamp - times[i]) / float(times[i + 1] - times[i]);
							return values[i] * (1 - t) + values[i + 1] * t;
						};

						std::vector<ofbx::i64> times;
						times.reserve(numKeys0 + numKeys1 + numKeys2);
						for(int l = 0; l < numKeys0; ++l)
							times.push_back(times0[l]);
						for(int l = 0; l < numKeys1; ++l)
							times.push_back(times1[l]);
						for(int l = 0; l < numKeys2; ++l)
							times.push_back(times2[l]);
						std::sort(times.begin(), times.end());
						if(times.size() > 1) {
							// Remove duplicate timestamps
							for(auto it = times.begin(); it != times.end() - 1;) {
								auto itNext = it;
								++itNext;
								auto t0 = *it;
								auto t1 = *itNext;
								if(t0 == t1) {
									// TODO: Check panima epsilon
									// if(umath::abs(t1 - t0) < 0.001f) {
									it = times.erase(it);
									continue;
								}
								++it;
							}
						}

						std::vector<Vector3> values;
						values.reserve(times.size());
						std::vector<float> ftimes;
						ftimes.reserve(times.size());
						for(auto t : times) {
							auto v0 = calcInterpolatedValue(numKeys0, times0, values0, t);
							auto v1 = calcInterpolatedValue(numKeys1, times1, values1, t);
							auto v2 = calcInterpolatedValue(numKeys2, times2, values2, t);
							Vector3 v {v0, v1, v2};
							values.push_back(v);
						}

						auto anim = std::make_shared<panima::Animation>();
						auto channel = std::make_shared<panima::Channel>();
						std::string basePath = "ec/animated/bone/" + std::string {bone->name} + "/";
						if(isBoneTranslation || isBoneScaling) {
							channel->SetValueType(udm::Type::Vector3);
							channel->InsertValues<Vector3>(ftimes.size(), ftimes.data(), values.data());
							if(isBoneTranslation)
								channel->targetPath = panima::ChannelPath {basePath + "position"};
							else
								channel->targetPath = panima::ChannelPath {basePath + "scale"};
						}
						else {
							channel->SetValueType(udm::Type::Quaternion);
							std::vector<Quat> qvalues;
							qvalues.reserve(values.size());
							for(auto &v : values) {
								EulerAngles ang {v.x, v.y, v.z};
								// TODO: Radian?
								auto rot = uquat::create(ang);
								qvalues.push_back(rot);
							}
							channel->InsertValues<Quat>(ftimes.size(), ftimes.data(), qvalues.data());
							channel->targetPath = panima::ChannelPath {basePath + "rotation"};
						}
					}
				}
			}
		}
	}
	return true;
}

std::optional<pragma::asset::AssetImportResult> FbxImporter::Load(std::string &outErr)
{
	auto mdl = c_game->CreateModel(false);
	m_model = mdl;
	mdl->GetBaseMeshes() = {0u};
	mdl->CreateTextureGroup();

	if(!LoadMeshes(outErr) || !LoadAnimations(outErr))
		return {};
	auto mdlPath = Finalize(outErr);
	if(!mdlPath)
		return {};
	AssetImportResult result {};
	result.models.push_back(*mdlPath);
	return result;
}
static umath::ScaledTransform get_pose(const ofbx::Object &o)
{
	auto fbxTrans = o.getLocalTranslation();
	auto fbxRot = o.getLocalRotation();
	auto fbxScale = o.getLocalScaling();

	EulerAngles ang {static_cast<float>(fbxRot.x), static_cast<float>(fbxRot.y), static_cast<float>(fbxRot.z)};
	Vector3 scale {fbxScale.x, fbxScale.y, fbxScale.z};
	for(size_t i = 0; i < 3; ++i) {
		if(umath::abs(scale[i] - 1.f) < 0.0001f)
			scale[i] = 1.f;
	}
	umath::ScaledTransform pose {};
	pose.SetOrigin({fbxTrans.x, fbxTrans.y, fbxTrans.z});
	pose.SetRotation(uquat::create(ang));
	pose.SetScale(scale);
	return pose;
}
std::shared_ptr<pragma::animation::Bone> FbxImporter::AddBone(const ofbx::Object &o)
{
	std::string name = o.name;
	if(name.empty())
		name = "bone" + std::to_string(o.id);
	auto &skel = m_model->GetSkeleton();
	auto &ref = m_model->GetReference();
	auto boneId = skel.LookupBone(name);
	if(boneId != -1)
		return skel.GetBone(boneId).lock();
	auto *bone = new pragma::animation::Bone {};
	bone->name = name;
	boneId = skel.AddBone(bone);

	ref.SetBoneCount(skel.GetBoneCount());
	auto pose = get_pose(o); //get_transform(o.getGlobalTransform());
	auto rot = pose.GetRotation();
	if(isnan(rot.w) || isnan(rot.x) || isnan(rot.y) || isnan(rot.z))
		std::cout << "";
	ref.SetBonePose(boneId, pose);
	auto *parent = o.getParent();
	if(parent) {
		bone->parent = AddBone(*parent);
		if(!bone->parent.expired())
			bone->parent.lock()->children[bone->ID] = skel.GetBone(bone->ID).lock();
	}
	if(bone->parent.expired())
		skel.GetRootBones()[boneId] = skel.GetBone(boneId).lock();
	return skel.GetBone(boneId).lock();
}
void FbxImporter::FillSkinInfo(std::vector<umath::VertexWeight> &skinning, const ofbx::Mesh *mesh, int32_t boneId)
{
	const ofbx::Skin *fbx_skin = mesh->getSkin();
	const ofbx::GeometryData &geom = mesh->getGeometryData();
	skinning.resize(geom.getPositions().values_count);
	for(auto &vw : skinning) {
		for(size_t i = 0; i < 4; ++i) {
			vw.boneIds[i] = -1;
			vw.weights[i] = 0.f;
		}
	}

	if(!fbx_skin) {
		for(auto &skin : skinning) {
			skin.weights[0] = 1;
			skin.boneIds[0] = boneId;
		}
		return;
	}
	auto getWeightCount = [](const umath::VertexWeight &vw) {
		size_t count = 0;
		for(size_t i = 0; i < 4; ++i) {
			if(vw.boneIds[i] == -1)
				break;
			++count;
		}
		return count;
	};

	for(int i = 0, c = fbx_skin->getClusterCount(); i < c; ++i) {
		const ofbx::Cluster *cluster = fbx_skin->getCluster(i);
		if(cluster->getIndicesCount() == 0)
			continue;
		if(!cluster->getLink())
			continue;

		auto bone = AddBone(*cluster->getLink());
		const int *cp_indices = cluster->getIndices();
		const double *weights = cluster->getWeights();
		for(int j = 0; j < cluster->getIndicesCount(); ++j) {
			int idx = cp_indices[j];
			float weight = (float)weights[j];
			auto &s = skinning[idx];
			if(getWeightCount(s) < 4) {
				s.weights[getWeightCount(s)] = weight;
				s.boneIds[getWeightCount(s)] = bone->ID;
			}
			else {
				int min = 0;
				for(int m = 1; m < 4; ++m) {
					if(s.weights[m] < s.weights[min])
						min = m;
				}

				if(s.weights[min] < weight) {
					s.weights[min] = weight;
					s.boneIds[min] = bone->ID;
				}
			}
		}
	}

	for(auto &s : skinning) {
		float sum = 0;
		for(size_t i = 0; i < 4; ++i)
			sum += s.weights[i];
		if(sum > 0.0001f) {
			for(size_t i = 0; i < 4; ++i)
				s.weights[i] /= sum;
		}
	}
}

std::optional<pragma::asset::AssetImportResult> pragma::asset::import_fbx(ufile::IFile &f, std::string &outErrMsg)
{
	auto fileName = f.GetFileName();
	if(!fileName) {
		outErrMsg = "No file name.";
		return {};
	}
	std::vector<uint8_t> data;
	auto sz = f.GetSize();
	data.resize(sz);
	f.Read(data.data(), data.size());
	ofbx::LoadFlags flags = ofbx::LoadFlags::IGNORE_VIDEOS;
	auto *scene = ofbx::load(data.data(), data.size(), (ofbx::u16)flags);
	if(!scene) {
		outErrMsg = "Failed to load FBX data: " + std::string {ofbx::getError()};
		return {};
	}
	auto mdlPath = ufile::get_path_from_filename(*fileName);
	auto mdlName = ufile::get_file_from_filename(*fileName);
	ufile::remove_extension_from_filename(mdlName, std::vector<std::string> {"fbx"});
	FbxImporter importer {scene, mdlPath, mdlName};
	return importer.Load(outErrMsg);
}

std::optional<pragma::asset::AssetImportResult> pragma::asset::import_fbx(const std::string &fileName, std::string &outErrMsg)
{
	auto f = filemanager::open_file(fileName, filemanager::FileMode::Read | filemanager::FileMode::Binary);
	if(!f) {
		outErrMsg = "Failed to open file '" + fileName + "' for reading!";
		return {};
	}
	fsys::File fptr {f};
	return import_fbx(fptr, outErrMsg);
}
