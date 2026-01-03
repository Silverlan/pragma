// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include <tiny_gltf.h>

module pragma.client;

import :assets.import_export;
import :assets.gltf_writer;

import :client_state;
import :engine;
import :entities.components;
import :game;
import :rendering.shaders;
import :util;

void pragma::asset::MapExportInfo::AddCamera(CCameraComponent &cam) { m_cameras.push_back(cam.GetHandle<CCameraComponent>()); }
void pragma::asset::MapExportInfo::AddLightSource(CLightComponent &light) { m_lightSources.push_back(light.GetHandle<CLightComponent>()); }
void pragma::asset::ModelExportInfo::SetAnimationList(const std::vector<std::string> &animations)
{
	exportAnimations = true;
	m_animations = animations;
}
std::vector<std::string> *pragma::asset::ModelExportInfo::GetAnimationList() { return m_animations.has_value() ? &*m_animations : nullptr; }

struct GLTFBufferData {
	tinygltf::Accessor &accessor;
	tinygltf::BufferView &bufferView;
	tinygltf::Buffer &buffer;
	template<typename T>
	T GetValue(uint32_t offset, const T &def = {}) const
	{
		offset += bufferView.byteOffset + accessor.byteOffset;
		if(offset + sizeof(T) >= buffer.data.size())
			return def;
		return *reinterpret_cast<T *>(buffer.data.data() + offset);
	}
	template<typename T>
	T GetIndexedValue(uint32_t index, const T &def = {}) const
	{
		auto stride = bufferView.byteStride;
		if(stride == 0)
			stride = sizeof(T);
		return GetValue(index * stride, def);
	}
	template<uint32_t C>
	std::array<int64_t, C> GetIntArray(uint32_t index) const
	{
		auto stride = bufferView.byteStride;
		auto componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
		if(stride == 0)
			stride = componentSize * C;
		auto offset = index * stride;
		std::array<int64_t, C> result {};
		for(auto i = decltype(C) {0u}; i < C; ++i) {
			int64_t value = 0;
			switch(accessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				value = GetValue<uint8_t>(offset);
				break;
			case TINYGLTF_COMPONENT_TYPE_BYTE:
				value = GetValue<int8_t>(offset);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				value = GetValue<uint16_t>(offset);
				break;
			case TINYGLTF_COMPONENT_TYPE_SHORT:
				value = GetValue<int16_t>(offset);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
				value = GetValue<uint32_t>(offset);
				break;
			case TINYGLTF_COMPONENT_TYPE_INT:
				value = GetValue<int32_t>(offset);
				break;
			}
			result[i] = value;
			offset += componentSize;
		}
		return result;
	}
	template<uint32_t C>
	std::array<float, C> GetFloatArray(uint32_t index) const
	{
		auto stride = bufferView.byteStride;
		auto componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
		if(stride == 0)
			stride = componentSize * C;
		auto offset = index * stride;
		std::array<float, C> result {};
		for(auto i = decltype(C) {0u}; i < C; ++i) {
			float value = 0.f;
			switch(accessor.componentType) {
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				value = GetValue<float>(offset);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				value = GetValue<uint8_t>(offset) / static_cast<float>(std::numeric_limits<uint8_t>::max());
				break;
			case TINYGLTF_COMPONENT_TYPE_BYTE:
				value = pragma::math::max(GetValue<int8_t>(offset) / static_cast<float>(std::numeric_limits<int8_t>::max()), -1.f);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				value = GetValue<uint16_t>(offset) / static_cast<float>(std::numeric_limits<uint16_t>::max());
				break;
			case TINYGLTF_COMPONENT_TYPE_SHORT:
				value = pragma::math::max(GetValue<int16_t>(offset) / static_cast<float>(std::numeric_limits<int16_t>::max()), -1.f);
				break;
			}
			result[i] = value;
			offset += componentSize;
		}
		return result;
	}
	Vector3 GetVector(uint32_t offset) const
	{
		auto v = GetFloatArray<3>(offset);
		return Vector3 {v[0], v[1], v[2]};
	}
	Quat GetQuat(uint32_t offset) const
	{
		auto v = GetFloatArray<4>(offset);
		return Quat {v[3], v[0], v[1], v[2]};
	}
	Vector4 GetVector4(uint32_t offset) const
	{
		auto v = GetFloatArray<4>(offset);
		return Vector4 {v[0], v[1], v[2], v[3]};
	}
};

static pragma::image::TextureInfo get_texture_write_info(pragma::asset::ModelExportInfo::ImageFormat imageFormat, bool normalMap, bool srgb, pragma::image::TextureInfo::AlphaMode alphaMode, std::string &outExt)
{
	pragma::image::TextureInfo texWriteInfo {};
	switch(imageFormat) {
	case pragma::asset::ModelExportInfo::ImageFormat::DDS:
		texWriteInfo.containerFormat = pragma::image::TextureInfo::ContainerFormat::DDS;
		outExt = "dds";
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::KTX:
		texWriteInfo.containerFormat = pragma::image::TextureInfo::ContainerFormat::KTX;
		outExt = "ktx";
		break;
	}
	texWriteInfo.flags = pragma::image::TextureInfo::Flags::GenerateMipmaps;
	texWriteInfo.alphaMode = alphaMode;
	// pragma::math::set_flag(texWriteInfo.flags,image::TextureInfo::Flags::SRGB,srgb);
	texWriteInfo.inputFormat = pragma::image::TextureInfo::InputFormat::KeepInputImageFormat;
	texWriteInfo.outputFormat = pragma::image::TextureInfo::OutputFormat::KeepInputImageFormat;
	if(normalMap)
		texWriteInfo.SetNormalMap();
	return texWriteInfo;
}

static bool save_image(pragma::image::ImageBuffer &imgBuf, pragma::asset::ModelExportInfo::ImageFormat imageFormat, std::string &inOutImgOutputPath, bool normalMap, bool srgb, pragma::image::TextureInfo::AlphaMode alphaMode)
{
	imgBuf.Convert(pragma::image::Format::RGBA8); // TODO

	if(imageFormat == pragma::asset::ModelExportInfo::ImageFormat::DDS || imageFormat == pragma::asset::ModelExportInfo::ImageFormat::KTX) {
		std::string ext;
		auto texWriteInfo = get_texture_write_info(imageFormat, normalMap, srgb, alphaMode, ext);
		texWriteInfo.inputFormat = pragma::image::TextureInfo::InputFormat::R8G8B8A8_UInt;
		inOutImgOutputPath += '.' + ext;
		return pragma::get_cgame()->SaveImage(imgBuf, inOutImgOutputPath, texWriteInfo);
	}

	auto saveFormat = pragma::image::ImageFormat::PNG;
	switch(imageFormat) {
	case pragma::asset::ModelExportInfo::ImageFormat::PNG:
		saveFormat = pragma::image::ImageFormat::PNG;
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::BMP:
		saveFormat = pragma::image::ImageFormat::BMP;
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::TGA:
		saveFormat = pragma::image::ImageFormat::TGA;
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::JPG:
		saveFormat = pragma::image::ImageFormat::JPG;
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::HDR:
		saveFormat = pragma::image::ImageFormat::HDR;
		break;
	}
	auto ext = pragma::image::get_file_extension(saveFormat);
	inOutImgOutputPath += '.' + ext;

	if(alphaMode == pragma::image::TextureInfo::AlphaMode::None)
		imgBuf.Convert(pragma::image::ImageBuffer::ToRGBFormat(imgBuf.GetFormat()));
	pragma::fs::create_path(ufile::get_path_from_filename(inOutImgOutputPath));
	auto fImg = pragma::fs::open_file<pragma::fs::VFilePtrReal>(inOutImgOutputPath, pragma::fs::FileMode::Write | pragma::fs::FileMode::Binary);
	if(fImg == nullptr)
		return false;
	pragma::fs::File f {fImg};
	return pragma::image::save_image(f, imgBuf, saveFormat);
};

struct GLTFInputData {
	std::string path;
	std::vector<std::shared_ptr<prosper::Texture>> textures {};
};

static bool load_image(tinygltf::Image *image, const int imageIdx, std::string *outErr, std::string *outWarn, int reqWidth, int reqHeight, const unsigned char *bytes, int size, void *userData)
{
	if(image->mimeType.empty() == false) {
		// Embedded image; Let tinygltf handle it
		return tinygltf::LoadImageData(image, imageIdx, outErr, outWarn, reqWidth, reqHeight, bytes, size, nullptr);
	}
	auto &inputData = *static_cast<GLTFInputData *>(userData);
	auto imgPath = inputData.path + image->uri;
	std::string relImgPath = pragma::util::Path::CreateFile(imgPath).GetString();
	pragma::fs::find_relative_path(relImgPath, relImgPath);
	auto &texManager = static_cast<pragma::material::CMaterialManager &>(pragma::get_client_state()->GetMaterialManager()).GetTextureManager();
	auto texture = texManager.LoadAsset(relImgPath, pragma::util::AssetLoadFlags::AbsolutePath | pragma::util::AssetLoadFlags::DontCache);
	if(texture == nullptr) {
		if(outErr)
			*outErr = "Failed to load texture '" + relImgPath + "'!";
		return false;
	}
	if(texture->HasValidVkTexture() == false) {
		if(outErr)
			*outErr = "Texture '" + relImgPath + "' has no valid VK texture!";
		return false;
	}
	if(imageIdx >= inputData.textures.size())
		inputData.textures.resize(imageIdx + 1);
	inputData.textures.at(imageIdx) = std::static_pointer_cast<pragma::material::Texture>(texture)->GetVkTexture();
	return true;
}

struct OutputData {
	std::shared_ptr<pragma::asset::Model> model;
	std::vector<std::string> models;
	std::string mapName;
};

pragma::image::TextureInfo pragma::asset::get_texture_info(bool isGreyScale, bool isNormalMap, AlphaMode alphaMode)
{
	image::TextureInfo texInfo {};
	texInfo.containerFormat = image::TextureInfo::ContainerFormat::DDS;
	texInfo.flags |= image::TextureInfo::Flags::GenerateMipmaps;
	if(isGreyScale)
		texInfo.outputFormat = image::TextureInfo::OutputFormat::GradientMap;
	else {
		switch(alphaMode) {
		case AlphaMode::Opaque:
			texInfo.outputFormat = image::TextureInfo::OutputFormat::BC1;
			break;
		case AlphaMode::Mask:
			texInfo.outputFormat = image::TextureInfo::OutputFormat::BC1a;
			break;
		case AlphaMode::Blend:
			texInfo.outputFormat = image::TextureInfo::OutputFormat::BC3;
			break;
		}
	}
	if(isNormalMap)
		texInfo.SetNormalMap();
	return texInfo;
}

void pragma::asset::assign_texture(material::CMaterial &mat, const std::string &textureRootPath, const std::string &matIdentifier, const std::string &texName, prosper::IImage &img, bool greyScale, bool normalMap, AlphaMode alphaMode)
{
	auto texInfo = get_texture_info(greyScale, normalMap, alphaMode);
	get_cgame()->SaveImage(img, textureRootPath + texName, texInfo);

	auto path = util::FilePath(texName);
	path.PopFront();
	mat.SetTexture(matIdentifier, path.GetString());
}

static std::optional<OutputData> import_model(ufile::IFile *optFile, const std::string &optFileName, std::string &outErrMsg, const pragma::util::Path &outputPath, bool importAsMap)
{
	auto scale = static_cast<float>(pragma::metres_to_units(1.f));

	std::string fileName = optFileName;
	std::string absPathToFile;
	if(optFile) {
		auto fname = optFile->GetFileName();
		if(fname.has_value()) {
			fileName = *fname;
			absPathToFile = fileName;
			fileName = ufile::get_file_from_filename(fileName);
		}
		else {
			fileName = "unknown";
			absPathToFile = pragma::fs::get_program_path() + '/' + fileName;
		}
	}
	else if(!pragma::fs::find_absolute_path(fileName, absPathToFile)) {
		spdlog::debug("Unable to determine absolute path for '{}'!", fileName);
		outErrMsg = "Unable to determine absolute path for '" + fileName + "'!";
		return {};
	}
	auto absPath = ufile::get_path_from_filename(absPathToFile);

	auto mdlName = ufile::get_file_from_filename(fileName);
	ufile::remove_extension_from_filename(mdlName);

	auto matPath = "materials/models/" + outputPath.GetString() + mdlName + '/';

	tinygltf::TinyGLTF reader {};

	GLTFInputData inputData {};
	inputData.path = absPath;

	reader.SetImageLoader(load_image, &inputData);

	std::string err;
	std::string warn;

	spdlog::debug("Loading file '{}'...", absPathToFile);

	tinygltf::Model gltfMdl {};
	auto result = false;
	if(optFile) {
		std::vector<uint8_t> fileData {};
		auto sz = optFile->GetSize();
		fileData.resize(sz);
		optFile->Read(fileData.data(), fileData.size());
		auto binary = (sz >= 4) ? pragma::string::compare(reinterpret_cast<char *>(fileData.data()), "glTF", true, 4) : false;
		result = binary ? reader.LoadBinaryFromMemory(&gltfMdl, &err, &warn, fileData.data(), fileData.size(), absPath) : reader.LoadASCIIFromString(&gltfMdl, &err, &warn, reinterpret_cast<char *>(fileData.data()), fileData.size(), absPath);
	}
	else {
		std::string ext;
		ufile::get_extension(fileName, &ext);

		auto binary = pragma::string::compare<std::string>(ext, "glb", false);
		result = binary ? reader.LoadBinaryFromFile(&gltfMdl, &err, &warn, absPathToFile) : reader.LoadASCIIFromFile(&gltfMdl, &err, &warn, absPathToFile);
	}
	if(result)
		spdlog::debug("Successfully loaded file '{}'! Creating model...", absPathToFile);
	else if(err.empty() == false)
		spdlog::debug("Unable to load file '{}': {}", absPathToFile, err);
	else
		spdlog::debug("Unable to load file '{}': {}", absPathToFile, warn);
	if(result == false) {
		if(err.empty() == false)
			outErrMsg = err;
		else
			outErrMsg = warn;
		return {};
	}

	auto TransformPos = [scale](const Vector3 &v) -> Vector3 { return v * scale; };

	auto mdl = std::shared_ptr<pragma::asset::Model> {pragma::get_cgame()->CreateModel(false)};
	mdl->GetBaseMeshes() = {0u};

	// Materials
	mdl->CreateTextureGroup();
	// TODO: Load materials/textures lazily
	auto numImages = gltfMdl.images.size();
	if(numImages >= inputData.textures.size())
		inputData.textures.resize(numImages);
	for(auto i = decltype(inputData.textures.size()) {0u}; i < inputData.textures.size(); ++i) {
		auto &tex = inputData.textures.at(i);

		auto &gltfImg = gltfMdl.images.at(i);
		if(gltfImg.image.empty() == false) {
			// It's an embedded image
			auto *data = gltfImg.image.data();

			std::shared_ptr<pragma::image::ImageBuffer> imgBuf = nullptr;
			auto numPixels = gltfImg.width * gltfImg.height;
			auto bytesPerChannel = gltfImg.bits / 8;
			auto format = pragma::image::Format::RGBA8;
			if(bytesPerChannel == 2)
				format = pragma::image::Format::RGBA16;
			else if(bytesPerChannel == 4)
				format = pragma::image::Format::RGBA32;
			auto numChannels = gltfImg.component;
			if(numChannels < 4) {
				imgBuf = pragma::image::ImageBuffer::Create(gltfImg.width, gltfImg.height, format);
				auto *convertedData = static_cast<uint8_t *>(imgBuf->GetData());
				for(auto i = decltype(numPixels) {0u}; i < numPixels; ++i) {
					auto *pIn = data + i * bytesPerChannel * numChannels;
					auto *pOut = convertedData + i * bytesPerChannel * 4;
					memcpy(pOut, pIn, bytesPerChannel * numChannels);
				}
			}
			else
				imgBuf = pragma::image::ImageBuffer::Create(data, gltfImg.width, gltfImg.height, format);

			auto img = pragma::get_cengine()->GetRenderContext().CreateImage(*imgBuf);
			prosper::util::TextureCreateInfo texCreateInfo {};
			prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
			prosper::util::SamplerCreateInfo samplerCreateInfo {};
			tex = pragma::get_cengine()->GetRenderContext().CreateTexture(texCreateInfo, *img, imgViewCreateInfo, samplerCreateInfo);
		}
	}
	uint32_t matIdx = 0;
	for(auto &gltfMat : gltfMdl.materials) {
		auto name = gltfMat.name;
		if(name.empty())
			name = "material_" + std::to_string(matIdx);

		auto alphaMode = AlphaMode::Opaque;
		if(gltfMat.alphaMode == "OPAQUE")
			alphaMode = AlphaMode::Opaque;
		else if(gltfMat.alphaMode == "MASK")
			alphaMode = AlphaMode::Mask;
		else if(gltfMat.alphaMode == "BLEND")
			alphaMode = AlphaMode::Blend;

		auto fGetTexture = [&gltfMdl, &inputData](int32_t index) -> std::shared_ptr<prosper::Texture> {
			if(index == -1)
				return nullptr;
			return inputData.textures.at(gltfMdl.textures.at(index).source);
		};

		auto fGetImage = [&fGetTexture, &inputData](int32_t index) -> std::shared_ptr<prosper::IImage> {
			auto tex = fGetTexture(index);
			if(tex == nullptr)
				return nullptr;
			return tex->GetImage().shared_from_this();
		};

		auto matName = matPath + name;
		pragma::util::Path matPathRelative {matName};
		matPathRelative.PopFront();

		auto mat = pragma::get_client_state()->CreateMaterial(matPathRelative.GetString(), "pbr");
		auto *cmat = static_cast<pragma::material::CMaterial *>(mat.get());
		mat->SetProperty("alpha_mode", alphaMode);
		mat->SetProperty("alpha_cutoff", gltfMat.alphaCutoff);

		auto fWriteImage = [cmat](const std::string &matIdentifier, const std::string &texName, prosper::IImage &img, bool greyScale, bool normalMap, AlphaMode alphaMode = AlphaMode::Opaque) {
			pragma::asset::assign_texture(*cmat, pragma::util::CONVERT_PATH, matIdentifier, texName, img, greyScale, normalMap, alphaMode);
		};

		auto isHandled = false;
		for(auto &pair : gltfMat.extensions) {
			if(pair.first == "KHR_materials_pbrSpecularGlossiness") {
				if(pair.second.IsObject()) {
					pragma::ShaderSpecularGlossinessToMetalnessRoughness::PushConstants pushConstants {};
					if(pair.second.Has("diffuseFactor")) {
						auto &vDiffuseFactor = pair.second.Get("diffuseFactor");
						if(vDiffuseFactor.IsArray()) {
							for(uint8_t i = 0; i < 4; ++i)
								pushConstants.diffuseFactor[i] = vDiffuseFactor.Get(i).GetNumberAsDouble();
						}
					}
					if(pair.second.Has("specularFactor")) {
						auto &vSpecularFactor = pair.second.Get("specularFactor");
						if(vSpecularFactor.IsArray()) {
							for(uint8_t i = 0; i < 3; ++i)
								pushConstants.specularFactor[i] = vSpecularFactor.Get(i).GetNumberAsDouble();
						}
					}
					if(pair.second.Has("glossinessFactor"))
						pushConstants.specularFactor.a = pair.second.Get("glossinessFactor").GetNumberAsDouble();

					prosper::Texture *diffuseTex = nullptr;
					if(pair.second.Has("diffuseTexture")) {
						auto diffuseTexture = pair.second.Get("diffuseTexture");
						if(diffuseTexture.Has("index")) {
							auto diffuseIndex = static_cast<int32_t>(diffuseTexture.Get("index").GetNumberAsInt());
							diffuseTex = fGetTexture(diffuseIndex).get();
						}
					}

					prosper::Texture *specularGlossinessTex = nullptr;
					if(pair.second.Has("specularGlossinessTexture")) {
						auto specularGlossinessTexture = pair.second.Get("specularGlossinessTexture");
						if(specularGlossinessTexture.Has("index")) {
							auto specularGlossinessIndex = static_cast<int32_t>(specularGlossinessTexture.Get("index").GetNumberAsInt());
							specularGlossinessTex = fGetTexture(specularGlossinessIndex).get();
						}
					}

					auto occlusionTex = fGetTexture(gltfMat.occlusionTexture.index);
					if(occlusionTex == nullptr)
						mat->SetProperty("rma_info/requires_ao_update", true);

					auto *shader = static_cast<pragma::ShaderSpecularGlossinessToMetalnessRoughness *>(pragma::get_cengine()->GetShader("specular_glossiness_to_metalness_roughness").get());
					if(shader) {
						auto metallicRoughnessSet = shader->ConvertToMetalnessRoughness(pragma::get_cengine()->GetRenderContext(), diffuseTex, specularGlossinessTex, pushConstants, occlusionTex.get());
						if(metallicRoughnessSet.has_value()) {
							fWriteImage(pragma::material::ematerial::ALBEDO_MAP_IDENTIFIER, matName + "_albedo", *metallicRoughnessSet->albedoMap, false /* greyScale */, false /* normalMap */, alphaMode);
							fWriteImage(pragma::material::ematerial::RMA_MAP_IDENTIFIER, matName + "_rma", *metallicRoughnessSet->rmaMap, false /* greyScale */, false /* normalMap */);
						}
					}

					isHandled = true;
				}
				break;
			}
		}

		if(isHandled == false) {
			auto &baseColorTexture = gltfMat.pbrMetallicRoughness.baseColorTexture;
			auto baseColorImage = fGetImage(baseColorTexture.index);
			if(baseColorImage)
				fWriteImage(pragma::material::ematerial::ALBEDO_MAP_IDENTIFIER, matName + "_albedo", *baseColorImage, false /* greyScale */, false /* normalMap */, alphaMode);
			else
				cmat->SetTexture(pragma::material::ematerial::ALBEDO_MAP_IDENTIFIER, "white");

			auto &baseColorFactor = gltfMat.pbrMetallicRoughness.baseColorFactor;
			if(baseColorFactor != std::vector<double> {1.0, 1.0, 1.0, 1.0})
				mat->SetProperty("color_factor", Vector4 {baseColorFactor.at(0), baseColorFactor.at(1), baseColorFactor.at(2), baseColorFactor.at(3)});

			auto metallicRoughnessImg = fGetImage(gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index);
			if(metallicRoughnessImg) {
				auto rmaName = matName + "_rma";
				fWriteImage(pragma::material::ematerial::RMA_MAP_IDENTIFIER, rmaName, *metallicRoughnessImg, false /* greyScale */, false /* normalMap */);

				auto occlusionImg = fGetImage(gltfMat.occlusionTexture.index);
				if(occlusionImg) {
					// Separate ao texture; Merge it with rma texture
					auto *shaderComposeRMA = static_cast<pragma::ShaderComposeRMA *>(pragma::get_cengine()->GetShader("compose_rma").get());
					if(shaderComposeRMA)
						shaderComposeRMA->InsertAmbientOcclusion(pragma::get_cengine()->GetRenderContext(), rmaName, *occlusionImg);
				}
				else
					mat->SetProperty("rma_info/requires_ao_update", true);
			}

			mat->SetProperty("roughness_factor", gltfMat.pbrMetallicRoughness.roughnessFactor);
			mat->SetProperty("metalness_factor", gltfMat.pbrMetallicRoughness.metallicFactor);
		}

		if(gltfMat.normalTexture.index != -1) {
			auto &tex = inputData.textures.at(gltfMdl.textures.at(gltfMat.normalTexture.index).source);
			if(tex)
				fWriteImage(pragma::material::ematerial::NORMAL_MAP_IDENTIFIER, matName + "_normal", tex->GetImage(), false /* greyScale */, true /* normalMap */);
		}

		if(gltfMat.emissiveTexture.index != -1) {
			auto &tex = inputData.textures.at(gltfMdl.textures.at(gltfMat.emissiveTexture.index).source);
			if(tex)
				fWriteImage(pragma::material::ematerial::EMISSION_MAP_IDENTIFIER, matName + "_emission", tex->GetImage(), false /* greyScale */, false /* normalMap */);
		}
		auto &emissiveFactor = gltfMat.emissiveFactor;
		if(emissiveFactor != std::vector<double> {1.0, 1.0, 1.0, 1.0})
			mat->SetProperty("emission_factor", Vector3 {emissiveFactor.at(0), emissiveFactor.at(1), emissiveFactor.at(2)});

		mat->UpdateTextures();
		mat->SetLoaded(true);
		auto savePath = pragma::asset::relative_path_to_absolute_path(matPathRelative, pragma::asset::Type::Material, pragma::util::CONVERT_PATH);
		std::string err;
		mat->Save(savePath.GetString(), err, true);

		mdl->AddMaterial(0, mat.get());

		++matIdx;
	}

	auto fGetBufferData = [&gltfMdl](int accessorIdx) -> GLTFBufferData {
		auto &accessor = gltfMdl.accessors.at(accessorIdx);
		auto &bufView = gltfMdl.bufferViews.at(accessor.bufferView);
		auto &buf = gltfMdl.buffers.at(bufView.buffer);
		return GLTFBufferData {accessor, bufView, buf};
	};

	auto getNodePose = [&](const tinygltf::Node &node) {
		pragma::math::ScaledTransform pose {};
		if(node.translation.size() == 3) {
			pose.SetOrigin(TransformPos(Vector3 {static_cast<float>(node.translation[0]), static_cast<float>(node.translation[1]), static_cast<float>(node.translation[2])}));
		}
		if(node.rotation.size() == 4) {
			pose.SetRotation(Quat {static_cast<float>(node.rotation[3]), static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2])});
		}
		if(node.scale.size() == 3) {
			pose.SetScale(Vector3 {static_cast<float>(node.scale[0]), static_cast<float>(node.scale[1]), static_cast<float>(node.scale[2])});
		}
		return pose;
	};

	auto &gltfMeshes = gltfMdl.meshes;
	uint32_t absUnnamedFcIdx = 0;
	struct NodeMeshData {
		std::optional<std::string> name;
		pragma::math::ScaledTransform pose;
	};
	std::vector<std::vector<NodeMeshData>> meshToNodes;
	meshToNodes.resize(gltfMeshes.size());
	for(auto &node : gltfMdl.nodes) {
		if(node.mesh >= meshToNodes.size() || node.name.empty())
			continue;
		auto pose = getNodePose(node);
		meshToNodes[node.mesh].push_back({node.name, pose});
	}

	struct InstanceInfo {
		std::string name;
		pragma::math::ScaledTransform pose;
	};
	std::unordered_map<pragma::asset::ModelMeshGroup *, std::vector<InstanceInfo>> meshInstances;
	for(uint32_t meshIdx = 0; auto &gltfMesh : gltfMeshes) {
		auto mesh = pragma::get_cgame()->CreateModelMesh();
		std::string name;
		auto &nodeMeshData = meshToNodes[meshIdx];
		std::shared_ptr<pragma::asset::ModelMeshGroup> firstMeshGroup = nullptr;
		for(auto nodeIdx = decltype(nodeMeshData.size()) {0u}; nodeIdx < nodeMeshData.size(); ++nodeIdx) {
			auto &nodeData = nodeMeshData[nodeIdx];
			auto pose = nodeData.pose;
			if(nodeData.name.has_value())
				name = *nodeData.name;
			else {
				if(!gltfMesh.name.empty())
					name = gltfMesh.name;
				else
					name = "mesh" + std::to_string(meshIdx);
				if(nodeIdx > 0)
					name += "_" + std::to_string(nodeIdx + 1);
			}
			if(importAsMap && nodeIdx > 0) {
				// There are multiple instances of the same mesh, no need to parse the mesh again
				InstanceInfo instanceInfo {};
				instanceInfo.name = name;
				instanceInfo.pose = pose;
				meshInstances[firstMeshGroup.get()].push_back(std::move(instanceInfo));
				continue;
			}
			uint32_t meshGroupId = 0;
			auto meshGroup = mdl->AddMeshGroup(name, meshGroupId);
			if(importAsMap) {
				assert(nodeIdx == 0);
				firstMeshGroup = meshGroup;

				InstanceInfo instanceInfo {};
				instanceInfo.name = name;
				instanceInfo.pose = pose;
				meshInstances[meshGroup.get()].push_back(instanceInfo);
				pose = {};
			}
			for(auto &primitive : gltfMesh.primitives) {
				auto itPos = primitive.attributes.find("POSITION");
				if(itPos == primitive.attributes.end())
					continue;
				auto &idxAccessor = gltfMdl.accessors.at(primitive.indices);
				auto &idxBufView = gltfMdl.bufferViews.at(idxAccessor.bufferView);
				auto &idxBuf = gltfMdl.buffers.at(idxBufView.buffer);

				auto *srcIndexData = idxBuf.data.data() + idxBufView.byteOffset + idxAccessor.byteOffset;

				auto subMesh = pragma::get_cgame()->CreateModelSubMesh();
				subMesh->SetSkinTextureIndex(primitive.material);
				auto numIndices = idxAccessor.count;
				switch(idxAccessor.componentType) {
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
					{
						subMesh->SetIndexType(pragma::geometry::IndexType::UInt16);
						subMesh->SetIndexCount(numIndices);
						auto &indexData = subMesh->GetIndexData();
						memcpy(indexData.data(), srcIndexData, indexData.size());
						break;
					}
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
					{
						subMesh->SetIndexType(pragma::geometry::IndexType::UInt16);
						subMesh->SetIndexCount(numIndices);
						subMesh->VisitIndices([srcIndexData](auto *indexData, uint32_t numIndices) {
							for(auto i = decltype(numIndices) {0u}; i < numIndices; ++i)
								indexData[i] = srcIndexData[i];
						});
						break;
					}
				case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
					{
						subMesh->SetIndexType(pragma::geometry::IndexType::UInt32);
						subMesh->SetIndexCount(numIndices);
						auto &indexData = subMesh->GetIndexData();
						memcpy(indexData.data(), srcIndexData, indexData.size());
						break;
					}
				}

				auto fGetVertexBufferData = [&primitive, &gltfMdl, &fGetBufferData](const std::string &identifier) -> std::optional<GLTFBufferData> {
					auto it = primitive.attributes.find(identifier);
					if(it == primitive.attributes.end())
						return {};
					return fGetBufferData(it->second);
				};
				auto posBufData = fGetVertexBufferData("POSITION");
				auto normBufData = fGetVertexBufferData("NORMAL");
				auto texCoordBufData = fGetVertexBufferData("TEXCOORD_0");
				auto texCoordBufData1 = fGetVertexBufferData("TEXCOORD_1");
				auto jointsBufData = fGetVertexBufferData("JOINTS_0");
				auto weightsBufData = fGetVertexBufferData("WEIGHTS_0");

				uint32_t iWeightChannel = 1;
				while(fGetVertexBufferData("JOINTS_" + std::to_string(iWeightChannel++)).has_value())
					Con::CWAR << "Model has more than 4 bone weights, this is not supported!" << Con::endl;

				auto &verts = subMesh->GetVertices();
				auto numVerts = posBufData->accessor.count;
				verts.resize(numVerts);

				std::vector<Vector2> *lightmapUvs = nullptr;
				if(texCoordBufData1.has_value()) {
					lightmapUvs = &subMesh->AddUVSet("lightmap");
					lightmapUvs->resize(numVerts);
				}

				for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
					auto &v = verts.at(i);
					v.position = TransformPos(posBufData->GetIndexedValue<Vector3>(i));
					if(normBufData.has_value())
						v.normal = normBufData->GetIndexedValue<Vector3>(i);
					if(texCoordBufData.has_value())
						v.uv = texCoordBufData->GetIndexedValue<Vector2>(i);
					if(lightmapUvs)
						(*lightmapUvs).at(i) = texCoordBufData1->GetIndexedValue<Vector2>(i);
				}

				if(pose != pragma::math::ScaledTransform {}) {
					for(auto &v : verts) {
						v.position *= pose.GetScale();
						v.position = pose * v.position;
						uvec::rotate(&v.normal, pose.GetRotation());
					}
				}

				if(jointsBufData.has_value() && weightsBufData.has_value()) {
					auto &vertWeights = subMesh->GetVertexWeights();
					vertWeights.resize(numVerts);

					for(auto i = decltype(numVerts) {0u}; i < numVerts; ++i) {
						auto &vw = vertWeights.at(i);

						auto weights = weightsBufData->GetFloatArray<4>(i);
						auto boneIds = jointsBufData->GetIntArray<4>(i);
						for(uint8_t j = 0; j < 4; ++j) {
							vw.weights[j] = weights[j];
							vw.boneIds[j] = boneIds[j];
						}
					}
					// JOINTS_1  -> +4
				}

				for(auto i = decltype(idxAccessor.count) {0u}; i < idxAccessor.count; ++i) {

					//idxAccessor.componentType == ;
					//idxAccessor.type
					//TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,TINYGLTF_TYPE_SCALAR

					//	The indices data type. Valid values correspond to WebGL enums: 5121 (UNSIGNED_BYTE), 5123 (UNSIGNED_SHORT), 5125 (UNSIGNED_INT).
				}
#if 0
			primitive.attributes["POSITION"] = posAccessor;
			primitive.attributes["NORMAL"] = normalAccessor;
			primitive.attributes["TEXCOORD_0"] = uvAccessor;

			if(isSkinned && mesh->GetVertexWeights().empty() == false)
			{
				auto jointsAccessor = AddAccessor("mesh" +std::to_string(meshIdx) +"_joints",TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT,TINYGLTF_TYPE_VEC4,vertexWeightOffset *sizeof(GLTFVertexWeight),verts.size(),BufferView::Joints);
				auto weightsAccessor = AddAccessor("mesh" +std::to_string(meshIdx) +"_weights",TINYGLTF_COMPONENT_TYPE_FLOAT,TINYGLTF_TYPE_VEC4,vertexWeightOffset *sizeof(GLTFVertexWeight),verts.size(),BufferView::Weights);
				primitive.attributes["JOINTS_0"] = jointsAccessor;
				primitive.attributes["WEIGHTS_0"] = weightsAccessor;

				vertexWeightOffset += verts.size();
			}
#endif

				uint32_t targetIdx = 0;
				for(auto &target : primitive.targets) {
					pragma::util::ScopeGuard sg {[&targetIdx]() { ++targetIdx; }};
					auto itPos = target.find("POSITION");
					auto itNormal = target.find("NORMAL");
					// auto itTangent = target.find("TANGENT");
					if(itPos == target.end())
						continue;
					auto &posAccessor = gltfMdl.accessors.at(itPos->second);
					if(posAccessor.bufferView == -1)
						continue;
					auto &posBufView = gltfMdl.bufferViews.at(posAccessor.bufferView);
					if(posBufView.buffer == -1)
						continue;
					auto &posBuf = gltfMdl.buffers.at(posBufView.buffer);
					auto posBufData = GLTFBufferData {posAccessor, posBufView, posBuf};

					std::unique_ptr<GLTFBufferData> normBufData {};
					if(itNormal != target.end()) {
						auto &normAccessor = gltfMdl.accessors.at(itNormal->second);
						if(normAccessor.bufferView != -1) {
							auto &normBufView = gltfMdl.bufferViews.at(normAccessor.bufferView);
							if(normBufView.buffer != -1) {
								auto &normBuf = gltfMdl.buffers.at(normBufView.buffer);
								normBufData = std::unique_ptr<GLTFBufferData> {new GLTFBufferData {normAccessor, normBufView, normBuf}};
							}
						}
					}

					auto isBeingUsed = false;
					for(auto i = decltype(posAccessor.count) {0u}; i < posAccessor.count; ++i) {
						auto pos = TransformPos(posBufData.GetIndexedValue<Vector3>(i));
						if(!isBeingUsed && uvec::length_sqr(pos) > 0.001f) {
							isBeingUsed = true;
							break;
						}
						if(normBufData) {
							auto n = normBufData->GetIndexedValue<Vector3>(i);
							if(!isBeingUsed && uvec::length_sqr(n) > 0.001f) {
								isBeingUsed = true;
								break;
							}
						}
					}
					if(!isBeingUsed)
						continue; // Skip this morph target if it's not actually doing anything

					std::string morphTargetName;

					if(gltfMesh.extras.Has("targetNames"))
						morphTargetName = gltfMesh.extras.Get("targetNames").Get(targetIdx).Get<std::string>();
					else
						morphTargetName = std::to_string(absUnnamedFcIdx + targetIdx);

					if(mdl->GetFlexController(morphTargetName) == nullptr) {
						auto defaultWeight = (targetIdx < gltfMesh.weights.size()) ? gltfMesh.weights.at(targetIdx) : 0.f;
						auto &fc = mdl->AddFlexController(morphTargetName);
						fc.min = 0.f;
						fc.max = 1.f;
						// TODO: Apply default
					}
					uint32_t fcId = 0;
					mdl->GetFlexControllerId(morphTargetName, fcId);

					if(mdl->GetFlex(morphTargetName) == nullptr) {
						auto &flex = mdl->AddFlex(morphTargetName);
						auto va = mdl->AddVertexAnimation(morphTargetName);
						flex.SetVertexAnimation(*va);

						auto &operations = flex.GetOperations();
						operations.push_back({});
						auto &op = flex.GetOperations().back();
						op.type = pragma::animation::Flex::Operation::Type::Fetch;
						op.d.index = fcId;
					}
					uint32_t flexId;
					mdl->GetFlexId(morphTargetName, flexId);
					auto &va = *mdl->GetFlex(flexId)->GetVertexAnimation();

					assert(posAccessor.count == numVerts);
					auto mva = va.AddMeshFrame(*mesh, *subMesh);
					mva->SetVertexCount(numVerts);
					if(normBufData)
						mva->SetFlagEnabled(pragma::animation::MeshVertexFrame::Flags::HasNormals);
					for(auto i = decltype(posAccessor.count) {0u}; i < posAccessor.count; ++i) {
						auto pos = TransformPos(posBufData.GetIndexedValue<Vector3>(i));
						mva->SetVertexPosition(i, pos);
						if(normBufData) {
							auto n = normBufData->GetIndexedValue<Vector3>(i);
							mva->SetVertexNormal(i, n);
						}
					}
				}
				//idxBuf.
				//primitive.indices
				//primitive.mode
				mesh->AddSubMesh(subMesh);
			}
			if(gltfMesh.primitives.empty() == false)
				absUnnamedFcIdx += gltfMesh.primitives.front().targets.size(); // All primitives have same number of targets
			meshGroup->AddMesh(mesh);
			auto &bg = mdl->AddBodyGroup(name);
			bg.meshGroups.push_back(meshGroupId);
			++meshIdx;
		}
	}

	std::unordered_map<tinygltf::Node *, uint32_t> nodeToBoneIndex;
	if(gltfMdl.skins.empty() == false) {
		auto &skeleton = mdl->GetSkeleton();
		auto &skin = gltfMdl.skins.front();
		skeleton.GetBones().reserve(skin.joints.size());
		std::unordered_map<int, pragma::animation::Bone *> nodeIdxToBone;
		for(auto i = decltype(skin.joints.size()) {0u}; i < skin.joints.size(); ++i) {
			auto nodeIdx = skin.joints[i];
			auto &node = gltfMdl.nodes[nodeIdx];
			nodeToBoneIndex[&node] = i;

			auto *bone = new pragma::animation::Bone {};
			bone->name = node.name;
			skeleton.AddBone(bone);
			nodeIdxToBone[nodeIdx] = bone;
		}

		auto bufferData = fGetBufferData(skin.inverseBindMatrices);

		// Build hierarchy
		std::vector<pragma::math::ScaledTransform> bindPoses {};
		bindPoses.reserve(skin.joints.size());
		for(auto i = decltype(skin.joints.size()) {0u}; i < skin.joints.size(); ++i) {
			auto nodeIdx = skin.joints[i];
			auto &node = gltfMdl.nodes[nodeIdx];
			auto &bone = *nodeIdxToBone[nodeIdx];
			for(auto childIdx : node.children) {
				auto it = nodeIdxToBone.find(childIdx);
				if(it == nodeIdxToBone.end())
					continue; // Not a bone?
				auto &child = *it->second;
				bone.children.insert(std::make_pair(child.ID, child.shared_from_this()));
				child.parent = bone.shared_from_this();
			}

			auto invBindMatrix = bufferData.GetIndexedValue<Mat4>(i);
			pragma::math::ScaledTransform bindPose {invBindMatrix};
			bindPose = bindPose.GetInverse();
			bindPose.SetOrigin(TransformPos(bindPose.GetOrigin()));
			bindPoses.push_back(bindPose);
		}

		// Build reference pose
		auto &reference = mdl->GetReference();
		reference.SetBoneCount(skin.joints.size());
		for(auto i = decltype(skin.joints.size()) {0u}; i < skin.joints.size(); ++i) {
			auto nodeIdx = skin.joints[i];
			auto &node = gltfMdl.nodes[nodeIdx];
			pragma::math::ScaledTransform pose {};
			if(!node.translation.empty())
				pose.SetOrigin(TransformPos(Vector3 {static_cast<float>(node.translation[0]), static_cast<float>(node.translation[1]), static_cast<float>(node.translation[2])}));
			if(!node.rotation.empty())
				pose.SetRotation(Quat {static_cast<float>(node.rotation[3]), static_cast<float>(node.rotation[0]), static_cast<float>(node.rotation[1]), static_cast<float>(node.rotation[2])});
			if(!node.scale.empty())
				pose.SetScale(Vector3 {static_cast<float>(node.scale[0]), static_cast<float>(node.scale[1]), static_cast<float>(node.scale[2])});
			reference.SetBonePose(i, pose);
		}

		for(auto &bone : skeleton.GetBones()) {
			if(bone->parent.expired() == false)
				continue;
			skeleton.GetRootBones().insert(std::make_pair(bone->ID, bone));
		}

		auto refAnim = pragma::animation::Animation::Create();
		refAnim->ReserveBoneIds(skin.joints.size());
		for(auto i = decltype(skin.joints.size()) {0u}; i < skin.joints.size(); ++i)
			refAnim->AddBoneId(i);

		auto frame = Frame::Create(reference);
		refAnim->AddFrame(frame);
		reference.Globalize(skeleton);
		mdl->AddAnimation("reference", refAnim);
	}

	auto &skeleton = mdl->GetSkeleton();
	auto numBones = skeleton.GetBoneCount();
	for(auto &gltfAnim : gltfMdl.animations) {
		auto &animName = gltfAnim.name;
		auto anim = pragma::animation::Animation::Create();
		float fps = 24.f;
		auto fGetFrame = [&](uint32_t frameIndex) -> Frame & {
			auto &frames = anim->GetFrames();
			if(frameIndex < frames.size())
				return *frames[frameIndex];
			frames.reserve(frameIndex + 1);
			for(auto i = frames.size(); i < frameIndex + 1; ++i) {
				auto frame = Frame::Create(numBones);
				frames.push_back(frame);
			}
			return *frames.back();
		};
		for(auto &channel : gltfAnim.channels) {
			auto &sampler = gltfAnim.samplers[channel.sampler];
			auto &node = gltfMdl.nodes[channel.target_node];

			auto it = nodeToBoneIndex.find(&node);
			if(it == nodeToBoneIndex.end())
				continue; // Not a bone
			auto boneId = it->second;

			auto bufTimes = fGetBufferData(sampler.input);
			auto bufValues = fGetBufferData(sampler.output);

			enum class Channel : uint8_t {
				Translation = 0,
				Rotation,
				Scale,
				Weights,

				Count
			};
			Channel eChannel;
			if(channel.target_path == "translation")
				eChannel = Channel::Translation;
			else if(channel.target_path == "rotation")
				eChannel = Channel::Rotation;
			else if(channel.target_path == "scale")
				eChannel = Channel::Scale;
			else if(channel.target_path == "weights")
				eChannel = Channel::Weights;

			auto n = bufTimes.accessor.count;
			std::vector<float> times;
			struct Value {
				Value(const Vector3 &translation) : translation {translation} {}
				Value(const Quat &rotation) : rotation {rotation} {}
				Value(float weight) : weight {weight} {}
				Value() {}
				Value(const Value &other) { memcpy(this, &other, sizeof(*this)); }
				union {
					Vector3 translation;
					Quat rotation;
					Vector3 scale;
					float weight;
				};
			};
			std::vector<Value> values;
			times.reserve(n);
			values.reserve(n);
			for(auto i = decltype(n) {0u}; i < n; ++i) {
				auto t = bufTimes.GetIndexedValue<float>(i);
				times.push_back(t);
				switch(eChannel) {
				case Channel::Translation:
					{
						auto translation = TransformPos(bufValues.GetVector(i));
						values.push_back({translation});
						break;
					}
				case Channel::Rotation:
					{
						auto rotation = bufValues.GetQuat(i);
						values.push_back({rotation});
						break;
					}
				case Channel::Scale:
					{
						auto scale = bufValues.GetVector(i);
						values.push_back({scale});
						break;
					}
				case Channel::Weights:
					{
						auto weight = bufValues.GetIndexedValue<float>(i);
						values.push_back({weight});
						break;
					}
				}
			}

			if(times.empty())
				continue;

			auto fGetInterpolatedValue = [&times, &values, eChannel](float tTgt) -> Value {
				auto it = std::find_if(times.begin(), times.end(), [tTgt](float t) { return t >= tTgt; });
				auto itNext = it;
				if(it == times.end()) {
					--it;
					itNext = it;
				}
				else {
					++itNext;
					if(itNext == times.end())
						itNext = it;
				}

				auto t0 = *it;
				auto t1 = *itNext;
				auto interp = (t1 > t0) ? pragma::math::clamp((tTgt - t0) / (t1 - t0), 0.f, 1.f) : 0.f;
				auto &v0 = values[it - times.begin()];
				auto &v1 = values[itNext - times.begin()];
				Value interpValue {};
				switch(eChannel) {
				case Channel::Translation:
					{
						interpValue.translation = uvec::lerp(v0.translation, v1.translation, interp);
						break;
					}
				case Channel::Rotation:
					{
						interpValue.rotation = uquat::slerp(v0.rotation, v1.rotation, interp);
						break;
					}
				case Channel::Scale:
					{
						interpValue.scale = uvec::lerp(v0.scale, v1.scale, interp);
						break;
					}
				case Channel::Weights:
					{
						interpValue.weight = pragma::math::lerp(v0.weight, v1.weight, interp);
						break;
					}
				}
				return interpValue;
			};

			auto numFrames = pragma::math::max(pragma::math::ceil(times.back() * 24), 1);
			for(auto i = decltype(numFrames) {0u}; i < numFrames; ++i) {
				auto &frame = fGetFrame(i);
				auto t = static_cast<float>(i) / fps;
				switch(eChannel) {
				case Channel::Translation:
					{
						frame.SetBonePosition(boneId, fGetInterpolatedValue(t).translation);
						break;
					}
				case Channel::Rotation:
					{
						frame.SetBoneOrientation(boneId, fGetInterpolatedValue(t).rotation);
						break;
					}
				case Channel::Scale:
					{
						frame.SetBoneScale(boneId, fGetInterpolatedValue(t).scale);
						break;
					}
				case Channel::Weights:
					{
						// TODO
						break;
					}
				}
			}
		}
		if(anim->GetFrameCount() == 0)
			continue;
		auto numBones = mdl->GetSkeleton().GetBoneCount();
		anim->ReserveBoneIds(numBones);
		for(auto i = decltype(numBones) {0u}; i < numBones; ++i)
			anim->AddBoneId(i);
		mdl->AddAnimation(animName, anim);
	}

	if(numBones > pragma::math::to_integral(pragma::GameLimits::MaxBones))
		Con::CWAR << "Model has " << numBones << ", but engine only supports " << pragma::math::to_integral(pragma::GameLimits::MaxBones) << ", this may cause rendering glitches!" << Con::endl;
#if 0
	for(auto &meshGroup : mdl->GetMeshGroups())
	{
		for(auto &mesh : meshGroup->GetMeshes())
		{
			for(auto &subMesh : mesh->GetSubMeshes())
			{
				auto &vertWeights = subMesh->GetVertexWeights();
				for(auto &vw : vertWeights)
				{
					for(uint8_t i=0;i<4;++i)
					{
						auto id = vw.boneIds[i];
						if(id >= numBones)
							Con::CWAR<<"Bone weight id "<<id<<" out of range ("<<numBones<<")!"<<Con::endl;
					}
				}
			}
		}
	}
#endif

	mdl->Update(pragma::asset::ModelUpdateFlags::All);

	OutputData outputData {};
	auto relFileName = outputPath + mdlName;
	auto mdlWritePath = pragma::util::CONVERT_PATH + pragma::asset::get_asset_root_directory(pragma::asset::Type::Model) + std::string {"/"} + outputPath.GetString();
	if(importAsMap) {
		std::unordered_set<std::string> materialMap;
		struct PropInfo {
			std::string name;
			std::string modelName;
			pragma::math::ScaledTransform pose;
		};
		std::vector<PropInfo> props;
		auto &meshGroups = mdl->GetMeshGroups();
		props.reserve(meshGroups.size());
		outputData.models.reserve(meshGroups.size());
		for(auto it = meshGroups.begin(); it != meshGroups.end(); ++it) {
			auto &meshGroup = *it;
			auto cpy = mdl->Copy(pragma::get_cgame());
			auto &cpyMeshGroups = cpy->GetMeshGroups();
			auto idx = it - meshGroups.begin();
			assert(idx >= 0 && idx < meshGroups.size());
			cpyMeshGroups = {meshGroups[idx]};

			auto &bodyGroups = cpy->GetBodyGroups();
			bodyGroups.clear();
			cpy->AddBodyGroup(meshGroups[idx]->GetName());

			cpy->RemoveUnusedMaterialReferences();
			cpy->Update();

			auto subMdlName = meshGroup->GetName();
			pragma::string::replace(subMdlName, " ", "_");
			pragma::string::replace(subMdlName, ".", "_");
			pragma::string::to_lower(subMdlName);
			cpy->Save(*pragma::get_cgame(), mdlWritePath + subMdlName, err);
			outputData.models.push_back((outputPath + subMdlName).GetString());

			auto &mats = cpy->GetMaterials();
			for(auto &mat : mats) {
				if(!mat)
					continue;
				materialMap.insert(mat->GetName());
			}

			auto itInstances = meshInstances.find(meshGroup.get());
			if(itInstances != meshInstances.end()) {
				for(auto &instanceInfo : itInstances->second) {
					props.push_back({});
					auto mdlName = outputPath + subMdlName;
					auto &propInfo = props.back();
					propInfo.modelName = mdlName.GetString();
					propInfo.name = instanceInfo.name;
					propInfo.pose = instanceInfo.pose;
				}
			}
		}

		auto worldData = pragma::asset::WorldData::Create(*pragma::get_client_state());
		auto &materials = worldData->GetMaterialTable();
		materials.reserve(materialMap.size());
		for(auto &mat : materialMap)
			materials.push_back(mat);

		uint32_t hashIdx = 0;
		auto createEntity = [&relFileName, &hashIdx](const pragma::math::ScaledTransform &pose, bool includeScale = true) -> std::shared_ptr<pragma::asset::EntityData> {
			auto baseHash = std::hash<std::string> {}(relFileName.GetString() + "_" + std::to_string(hashIdx++));
			auto ent = pragma::asset::EntityData::Create();
			ent->SetPose(pose);
			return ent;
		};

		for(auto i = decltype(props.size()) {0u}; i < props.size(); ++i) {
			auto &propInfo = props[i];

			auto ent = createEntity(propInfo.pose);
			ent->SetClassName("prop_dynamic");
			ent->SetKeyValue("model", propInfo.modelName);
			ent->SetKeyValue("name", propInfo.name);
			worldData->AddEntity(*ent);
		}

		for(auto &node : gltfMdl.nodes) {
			auto itExt = node.extensions.find("KHR_lights_punctual");
			if(itExt == node.extensions.end())
				continue;
			auto &extLight = itExt->second.Get("light");
			if(extLight.IsInt() == false)
				continue;
			auto lightSourceIndex = extLight.GetNumberAsInt();
			if(lightSourceIndex < 0 || lightSourceIndex >= gltfMdl.lights.size())
				continue;
			auto &light = gltfMdl.lights[lightSourceIndex];
			auto pose = getNodePose(node);
			auto color = light.color;
			color.resize(3);
			auto ent = createEntity(pose, false);
			// TODO: Some of these probably have to be converted
			ent->SetKeyValue("color", std::to_string(color[0]) + " " + std::to_string(color[1]) + " " + std::to_string(color[2]));
			ent->SetKeyValue("intensity", std::to_string(light.intensity));
			if(!node.name.empty())
				ent->SetKeyValue("name", node.name);
			if(light.type == "spot") {
				ent->SetClassName("env_light_spot");

				ent->SetKeyValue("radius", std::to_string(light.range));
				ent->SetKeyValue("outerCutoff", std::to_string(light.spot.outerConeAngle));

				auto blendFraction = pragma::BaseEnvLightSpotComponent::CalcBlendFraction(light.spot.outerConeAngle, light.spot.innerConeAngle);
				ent->SetKeyValue("blendFraction", std::to_string(blendFraction));

				worldData->AddEntity(*ent);
			}
			else if(light.type == "point") {
				ent->SetClassName("env_light_point");
				ent->SetKeyValue("radius", std::to_string(light.range));
			}
			else if(light.type == "directional")
				;
			else
				continue; // Unknown light type
			worldData->AddEntity(*ent);
		}

		for(auto &node : gltfMdl.nodes) {
			if(node.camera < 0 || node.camera >= gltfMdl.cameras.size())
				continue;
			auto &cam = gltfMdl.cameras[node.camera];
			if(cam.type != "perspective")
				continue; // orthographic currently not supported
			auto pose = getNodePose(node);
			auto ent = createEntity(pose, false);
			ent->SetClassName("env_camera");

			ent->SetKeyValue("fov", std::to_string(pragma::math::rad_to_deg(cam.perspective.yfov)));
			ent->SetKeyValue("farz", std::to_string(pragma::metres_to_units(cam.perspective.znear)));
			ent->SetKeyValue("nearz", std::to_string(pragma::metres_to_units(cam.perspective.zfar)));
			ent->SetKeyValue("aspectRatio", std::to_string(cam.perspective.aspectRatio));

			worldData->AddEntity(*ent);
		}

		auto mapWritePath = pragma::util::CONVERT_PATH + pragma::asset::get_asset_root_directory(pragma::asset::Type::Map) + std::string {"/"} + relFileName.GetString();

		auto udmData = udm::Data::Create();
		auto assetData = udmData->GetAssetData();
		std::string errMsg;
		auto res = worldData->Save(assetData, ufile::get_file_from_filename(mapWritePath), errMsg);
		if(!res) {
			err = "Failed to save map data '" + mapWritePath + "': " + errMsg;
			return {};
		}
		auto ext = pragma::asset::get_udm_format_extension(pragma::asset::Type::Map, true);
		assert(ext.has_value());
		mapWritePath += "." + *ext;
		pragma::fs::create_path(ufile::get_path_from_filename(mapWritePath));
		try {
			res = udmData->Save(mapWritePath);
		}
		catch(const udm::Exception &e) {
			err = "Failed to save map file '" + mapWritePath + "': " + std::string {e.what()};
		}
		if(!res && err.empty())
			err = "Failed to save map file '" + mapWritePath + "': Unknown error";
		outputData.mapName = relFileName.GetString();
		return outputData;
	}

	mdl->ApplyPostImportProcessing();
	mdl->Save(*pragma::get_cgame(), mdlWritePath + mdlName, err);
	outputData.model = mdl;
	return outputData;
}
std::shared_ptr<pragma::asset::Model> pragma::asset::import_model(ufile::IFile &f, std::string &outErrMsg, const util::Path &outputPath, bool importAsSingleModel)
{
	auto data = ::import_model(&f, "", outErrMsg, outputPath, !importAsSingleModel);
	if(!data)
		return nullptr;
	return data->model;
}
std::shared_ptr<pragma::asset::Model> pragma::asset::import_model(const std::string &fileName, std::string &outErrMsg, const util::Path &outputPath, bool importAsSingleModel)
{
	auto data = ::import_model(nullptr, fileName, outErrMsg, outputPath, !importAsSingleModel);
	if(!data)
		return nullptr;
	return data->model;
}
std::optional<pragma::asset::AssetImportResult> pragma::asset::import_gltf(ufile::IFile &f, std::string &outErrMsg, const util::Path &outputPath, bool importAsSingleModel)
{
	auto data = ::import_model(&f, "", outErrMsg, outputPath, !importAsSingleModel);
	if(!data)
		return {};
	AssetImportResult importInfo {};
	importInfo.models = std::move(data->models);
	importInfo.mapName = std::move(data->mapName);
	return importInfo;
}
std::optional<pragma::asset::AssetImportResult> pragma::asset::import_gltf(const std::string &fileName, std::string &outErrMsg, const util::Path &outputPath, bool importAsSingleModel)
{
	auto data = ::import_model(nullptr, fileName, outErrMsg, outputPath, !importAsSingleModel);
	if(!data)
		return {};
	AssetImportResult importInfo {};
	importInfo.models = std::move(data->models);
	importInfo.mapName = std::move(data->mapName);
	return importInfo;
}

bool pragma::asset::import_texture(const std::string &fileName, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg)
{
	auto tex = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager()).GetTextureManager().LoadAsset(fileName, util::AssetLoadFlags::DontCache);
	if(tex == nullptr) {
		outErrMsg = "Unable to load texture!";
		return false;
	}
	if(std::static_pointer_cast<material::Texture>(tex)->HasValidVkTexture() == false) {
		outErrMsg = "Invalid texture!";
		return false;
	}
	return import_texture(std::static_pointer_cast<material::Texture>(tex)->GetVkTexture()->GetImage(), texInfo, outputPath, outErrMsg);
}
bool pragma::asset::import_texture(std::unique_ptr<ufile::IFile> &&f, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg)
{
	auto path = f->GetFileName();
	if(!path.has_value())
		return false;
	std::string ext;
	if(ufile::get_extension(*path, &ext) == false)
		return false;
	auto &texManager = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager()).GetTextureManager();
	auto tex = texManager.LoadAsset("", std::move(f), ext, std::make_unique<material::TextureLoadInfo>(util::AssetLoadFlags::DontCache));
	if(tex == nullptr) {
		outErrMsg = "Unable to load texture!";
		return false;
	}
	if(std::static_pointer_cast<material::Texture>(tex)->HasValidVkTexture() == false) {
		outErrMsg = "Invalid texture!";
		return false;
	}
	return import_texture(std::static_pointer_cast<material::Texture>(tex)->GetVkTexture()->GetImage(), texInfo, outputPath, outErrMsg);
}
bool pragma::asset::import_texture(prosper::IImage &img, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg)
{
	std::string ext;
	auto texWriteInfo = get_texture_write_info(ModelExportInfo::ImageFormat::DDS, texInfo.normalMap, texInfo.srgb, image::TextureInfo::AlphaMode::Auto, ext);
	if(texInfo.greyScaleMap)
		texWriteInfo.outputFormat = image::TextureInfo::OutputFormat::GradientMap;
	else // TODO: Use BC1 if no alpha!
		texWriteInfo.outputFormat = image::TextureInfo::OutputFormat::BC3;

	if(texWriteInfo.containerFormat == image::TextureInfo::ContainerFormat::DDS) {
		auto anvFormat = img.GetFormat();
		switch(anvFormat) {
			// These formats require DDS10, which is not well supported, so we'll fall back to
			// a different compression format
		case prosper::Format::BC6H_SFloat_Block:
		case prosper::Format::BC6H_UFloat_Block:
		case prosper::Format::BC7_SRGB_Block:
		case prosper::Format::BC7_UNorm_Block:
			texWriteInfo.outputFormat = image::TextureInfo::OutputFormat::BC3;
			break;
		}
	}
	auto imgOutputPath = "materials/" + util::Path {outputPath};
	imgOutputPath.RemoveFileExtension();
	return get_cgame()->SaveImage(img, imgOutputPath.GetString(), texWriteInfo);
}

bool pragma::asset::export_map(const std::string &mapName, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<MapExportInfo> &mapExp)
{
	util::Path mapPath {mapName};
	mapPath.RemoveFileExtension(get_supported_extensions(Type::Map));
	fs::VFilePtr f = nullptr;
	auto openLocalMap = [&mapPath, &f]() {
		auto localMapPath = find_file(mapPath.GetString(), Type::Map);
		if(localMapPath.has_value()) {
			auto filePath = util::Path::CreateFile(std::string {get_asset_root_directory(Type::Map)} + '/' + *localMapPath);
			f = pragma::fs::open_file(filePath.GetString().c_str(), fs::FileMode::Read | fs::FileMode::Binary);
		}
	};
	openLocalMap();
	if(f == nullptr) {
		if(pragma::util::port_source2_map(get_client_state(), mapPath.GetString()) || pragma::util::port_hl2_map(get_client_state(), mapPath.GetString())) {
			f = pragma::fs::open_file(mapPath.GetString().c_str(), fs::FileMode::Read | fs::FileMode::Binary);
			if(f == nullptr) {
				// Sleep for a bit, then try again, in case the file hasn't been fully written yet
				util::sleep_for_seconds(1);
				openLocalMap();
			}
		}
	}
	if(f == nullptr) {
		outErrMsg = "Unable to open map file '" + mapPath.GetString() + "'!";
		return false;
	}
	if(exportInfo.verbose)
		Con::COUT << "Loading map data..." << Con::endl;

	auto worldData = WorldData::Create(*get_client_state());
	auto udmData = util::load_udm_asset(std::make_unique<fs::File>(f));
	f = nullptr;
	std::string err;
	if(udmData == nullptr || worldData->LoadFromAssetData(udmData->GetAssetData(), EntityData::Flags::None, err) == false)
		return false;

	if(exportInfo.verbose)
		Con::COUT << "Collecting world node models..." << Con::endl;

	GLTFWriter::SceneDesc sceneDesc {};
	if(mapExp.has_value()) {
		auto &cameras = mapExp->GetCameras();
		sceneDesc.cameras.reserve(cameras.size());
		for(auto &hCamC : cameras) {
			if(hCamC.expired())
				continue;
			auto &camC = *hCamC;
			auto &ent = camC.GetEntity();
			sceneDesc.cameras.push_back({});

			auto &camScene = sceneDesc.cameras.back();
			camScene.name = ent.GetName();
			camScene.type = GLTFWriter::Camera::Type::Perspective;
			camScene.aspectRatio = camC.GetAspectRatio();
			camScene.vFov = camC.GetFOV();
			camScene.zNear = camC.GetNearZ();
			camScene.zFar = camC.GetFarZ();
			camScene.pose = ent.GetPose();
		}

		auto &lightSources = mapExp->GetLightSources();
		sceneDesc.lightSources.reserve(lightSources.size());
		for(auto &hLightC : lightSources) {
			if(hLightC.expired())
				continue;
			auto &lightC = *hLightC;
			auto &ent = lightC.GetEntity();
			sceneDesc.lightSources.push_back({});

			auto &lightScene = sceneDesc.lightSources.back();
			lightScene.name = ent.GetName();
			lightScene.pose = ent.GetPose();

			auto colorC = ent.GetComponent<CColorComponent>();
			if(colorC.valid())
				lightScene.color = colorC->GetColor();

			auto radiusC = ent.GetComponent<CRadiusComponent>();
			if(radiusC.valid())
				lightScene.range = units_to_metres(radiusC->GetRadius());

			auto spotC = ent.GetComponent<CLightSpotComponent>();
			if(spotC.valid()) {
				lightScene.type = GLTFWriter::LightSource::Type::Spot;
				lightScene.blendFraction = spotC->GetBlendFraction();
				lightScene.outerConeAngle = spotC->GetOuterConeAngle();
			}

			auto pointC = ent.GetComponent<CLightPointComponent>();
			if(pointC.valid())
				lightScene.type = GLTFWriter::LightSource::Type::Point;

			auto dirC = ent.GetComponent<CLightDirectionalComponent>();
			if(dirC.valid())
				lightScene.type = GLTFWriter::LightSource::Type::Directional;

			switch(lightScene.type) {
			case GLTFWriter::LightSource::Type::Spot:
			case GLTFWriter::LightSource::Type::Point:
				// TODO: This should be Candela, not Lumen, but Lumen produces closer results for Blender
				lightScene.luminousIntensity = lightC.GetLightIntensityLumen();
				break;
			case GLTFWriter::LightSource::Type::Directional:
				lightScene.illuminance = lightC.GetLightIntensity();
				break;
			}
		}
	}

	for(auto &ent : worldData->GetEntities()) {
		if(ent->IsWorld() || ent->GetClassName() == "prop_static" || ent->GetClassName() == "prop_physics" || ent->GetClassName() == "prop_dynamic") {
			auto strMdl = ent->GetKeyValue("model");
			if(strMdl.has_value() == false)
				continue;
			if(exportInfo.verbose)
				Con::COUT << "Loading world node model '" << *strMdl << "'..." << Con::endl;
			auto mdl = get_cgame()->LoadModel(*strMdl);
			if(mdl == nullptr) {
				Con::CWAR << "Unable to load model '" << *strMdl << "'! Model will not be included in level export!" << Con::endl;
				continue;
			}
			if(sceneDesc.modelCollection.size() == sceneDesc.modelCollection.capacity())
				sceneDesc.modelCollection.reserve(sceneDesc.modelCollection.size() * 1.5 + 100);
			sceneDesc.modelCollection.push_back({*mdl});
			auto &mdlDesc = sceneDesc.modelCollection.back();
			mdlDesc.pose = ent->GetEffectivePose();
		}
		else if(ent->GetClassName() == "env_light_spot" || ent->GetClassName() == "env_light_point" || ent->GetClassName() == "env_light_environment") {
			if(mapExp.has_value() && mapExp->includeMapLightSources == false)
				continue;
			sceneDesc.lightSources.push_back({});
			auto &ls = sceneDesc.lightSources.back();

			ls.name = ent->GetKeyValue("name", ent->GetClassName() + '_' + std::to_string(ent->GetMapIndex()));
			ls.pose = ent->GetEffectivePose();

			auto color = ent->GetKeyValue("color");
			if(color.has_value() == false)
				color = ent->GetKeyValue("lightcolor");
			if(color.has_value())
				ls.color = Color {*color};

			auto radius = ent->GetKeyValue("radius");
			if(radius.has_value() == false)
				radius = ent->GetKeyValue("distance");
			if(radius.has_value())
				ls.range = util::to_float(*radius);

			std::optional<float> outerCutoffAngle {};
			if(ent->GetClassName() == "env_light_spot") {
				ls.type = GLTFWriter::LightSource::Type::Spot;

				auto blendFraction = ent->GetKeyValue("blendfraction");
				if(blendFraction.has_value())
					ls.blendFraction = util::to_float(*blendFraction);

				auto outerCutoff = ent->GetKeyValue("outercutoff");
				if(outerCutoff.has_value()) {
					ls.outerConeAngle = util::to_float(*outerCutoff);
					outerCutoffAngle = ls.outerConeAngle;
				}
			}
			else if(ent->GetClassName() == "env_light_point")
				ls.type = GLTFWriter::LightSource::Type::Point;
			else if(ent->GetClassName() == "env_light_environment")
				ls.type = GLTFWriter::LightSource::Type::Directional;

			auto intensity = ent->GetKeyValue("light_intensity");
			if(intensity.has_value()) {
				auto intensityType = BaseEnvLightComponent::LightIntensityType::Candela;
				auto vIntensityType = ent->GetKeyValue("light_intensity_type");
				if(vIntensityType.has_value())
					intensityType = static_cast<BaseEnvLightComponent::LightIntensityType>(util::to_int(*vIntensityType));

				auto flIntensity = util::to_float(*intensity);
				switch(ls.type) {
				case GLTFWriter::LightSource::Type::Spot:
				case GLTFWriter::LightSource::Type::Point:
					// TODO: This should be Candela, not Lumen, but Lumen produces closer results for Blender
					ls.luminousIntensity = BaseEnvLightComponent::GetLightIntensityLumen(flIntensity, intensityType, outerCutoffAngle);
					break;
				case GLTFWriter::LightSource::Type::Directional:
					ls.illuminance = flIntensity;
					break;
				}
			}
		}
	}

	if(sceneDesc.modelCollection.empty()) {
		outErrMsg = "No models to export found!";
		return false;
	}

	// HACK: If the model was just ported, we need to make sure the material and textures are in order by invoking the
	// resource watcher (in case they have been changed)
	// TODO: This doesn't belong here!
	get_client_state()->GetResourceWatcher().Poll();

	if(exportInfo.verbose)
		Con::COUT << "Exporting scene with " << sceneDesc.modelCollection.size() << " models and " << sceneDesc.lightSources.size() << " light sources..." << Con::endl;

	auto exportPath = "maps/" + mapName + '/';
	auto mdlName = exportPath + mapName;

	auto mapExportInfo = exportInfo;
	mapExportInfo.exportSkinnedMeshData = false;
	mapExportInfo.generateAo = false;
	auto success = GLTFWriter::Export(sceneDesc, mdlName, mapExportInfo, outErrMsg);
	if(success == false)
		return false;
	if(exportInfo.verbose)
		Con::COUT << "Exporting lightmap atlas..." << Con::endl;
	success = export_texture("maps/" + mapName + "/lightmap_atlas", mapExportInfo.imageFormat, outErrMsg, image::TextureInfo::AlphaMode::None, true /* extended DDS */, &exportPath);
	if(success == false) {
		if(exportInfo.verbose)
			Con::CWAR << "Unable to export lightmap atlas: " << outErrMsg << Con::endl;
	}
	return true;
}

bool pragma::asset::export_model(Model &mdl, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &modelName, std::string *optOutPath) { return GLTFWriter::Export(mdl, exportInfo, outErrMsg, modelName, optOutPath); }
bool pragma::asset::export_animation(Model &mdl, const std::string &animName, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &modelName) { return GLTFWriter::Export(mdl, animName, exportInfo, outErrMsg, modelName); }
bool pragma::asset::export_texture(image::ImageBuffer &imgBuf, ModelExportInfo::ImageFormat imageFormat, const std::string &outputPath, std::string &outErrMsg, bool normalMap, bool srgb, image::TextureInfo::AlphaMode alphaMode, std::string *optOutOutputPath)
{
	std::string inOutPath = std::string {EXPORT_PATH} + outputPath;
	auto success = save_image(imgBuf, imageFormat, inOutPath, normalMap, srgb, alphaMode);
	if(optOutOutputPath)
		*optOutOutputPath = inOutPath;
	return success;
}
bool pragma::asset::export_texture(const std::string &texturePath, ModelExportInfo::ImageFormat imageFormat, std::string &outErrMsg, image::TextureInfo::AlphaMode alphaMode, bool enableExtendedDDS, std::string *optExportPath, std::string *optOutOutputPath,
  const std::optional<std::string> &optFileNameOverride)
{
	auto &texManager = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager()).GetTextureManager();
	auto pTexture = texManager.LoadAsset(texturePath);
	if(pTexture == nullptr || pTexture->HasValidVkTexture() == false) {
		outErrMsg = "Unable to load texture '" + texturePath + "'!";
		return false;
	}
	auto texture = std::static_pointer_cast<material::Texture>(pTexture);
	auto &vkImg = texture->GetVkTexture()->GetImage();

	auto imgPath = optFileNameOverride.has_value() ? *optFileNameOverride : ufile::get_file_from_filename(texturePath);
	ufile::remove_extension_from_filename(imgPath);
	if(optExportPath)
		imgPath = (util::Path::CreatePath(*optExportPath) + imgPath).GetString();
	auto imgOutputPath = std::string {EXPORT_PATH} + imgPath;

	auto exportSuccess = false;
	if(imageFormat == ModelExportInfo::ImageFormat::DDS || imageFormat == ModelExportInfo::ImageFormat::KTX) {
		std::string ext;
		auto texWriteInfo = get_texture_write_info(imageFormat, texture->HasFlag(material::Texture::Flags::NormalMap), texture->HasFlag(material::Texture::Flags::SRGB), alphaMode, ext);
		imgOutputPath += '.' + ext;
		if(texWriteInfo.containerFormat == image::TextureInfo::ContainerFormat::DDS && enableExtendedDDS == false) {
			auto anvFormat = vkImg.GetFormat();
			switch(anvFormat) {
				// These formats require DDS10, which is not well supported, so we'll fall back to
				// a different compression format
			case prosper::Format::BC6H_SFloat_Block:
			case prosper::Format::BC6H_UFloat_Block:
			case prosper::Format::BC7_SRGB_Block:
			case prosper::Format::BC7_UNorm_Block:
				texWriteInfo.outputFormat = image::TextureInfo::OutputFormat::BC3;
				break;
			}
		}
		exportSuccess = get_cgame()->SaveImage(vkImg, imgOutputPath, texWriteInfo);
	}
	else {
		std::vector<std::vector<std::shared_ptr<image::ImageBuffer>>> imgBuffers;
		if(util::to_image_buffer(vkImg, {}, imgBuffers) == false) {
			outErrMsg = "Unable to convert texture '" + texturePath + "' to image buffer!";
			return false;
		}
		auto &imgBuf = imgBuffers.front().front();
		exportSuccess = save_image(*imgBuf, imageFormat, imgOutputPath, texture->HasFlag(material::Texture::Flags::NormalMap), texture->HasFlag(material::Texture::Flags::SRGB), alphaMode);
	}
	if(exportSuccess == false) {
		outErrMsg = "Unable to export texture '" + texturePath + "'!";
		return false;
	}
	if(optOutOutputPath)
		*optOutOutputPath = imgOutputPath;
	return true;
}
std::optional<pragma::asset::MaterialTexturePaths> pragma::asset::export_material(material::Material &mat, ModelExportInfo::ImageFormat imageFormat, std::string &outErrMsg, std::string *optExportPath, bool normalizeTextureNames)
{
	auto name = ufile::get_file_from_filename(mat.GetName());
	ufile::remove_extension_from_filename(name);

	auto exportPath = optExportPath ? *optExportPath : name;

	auto fSaveTexture = [imageFormat, &exportPath, normalizeTextureNames](TextureInfo *texInfo, bool normalMap, bool alpha, std::string &imgOutputPath, const std::string &normalizedName) -> bool {
		if(texInfo == nullptr)
			return false;
		std::string errMsg;
		return export_texture(texInfo->name, imageFormat, errMsg, alpha ? image::TextureInfo::AlphaMode::Transparency : image::TextureInfo::AlphaMode::None, false, &exportPath, &imgOutputPath, normalizeTextureNames ? normalizedName : std::optional<std::string> {});
	};

	auto alphaMode = mat.GetAlphaMode();

	MaterialTexturePaths texturePaths {};

	std::string imgOutputPath;
	if(fSaveTexture(mat.GetAlbedoMap(), false, alphaMode != AlphaMode::Opaque, imgOutputPath, name))
		texturePaths.insert(std::make_pair(material::ematerial::ALBEDO_MAP_IDENTIFIER, imgOutputPath));
	if(fSaveTexture(mat.GetNormalMap(), true, false, imgOutputPath, name + "_normal"))
		texturePaths.insert(std::make_pair(material::ematerial::NORMAL_MAP_IDENTIFIER, imgOutputPath));
	if(fSaveTexture(mat.GetRMAMap(), true, false, imgOutputPath, name + "_rma"))
		texturePaths.insert(std::make_pair(material::ematerial::RMA_MAP_IDENTIFIER, imgOutputPath));

	if(fSaveTexture(mat.GetGlowMap(), false, false, imgOutputPath, name + "_emission"))
		texturePaths.insert(std::make_pair(material::ematerial::EMISSION_MAP_IDENTIFIER, imgOutputPath));
	return texturePaths;
}

class ModelAOWorker : public pragma::util::ParallelWorker<pragma::asset::ModelAOWorkerResult> {
  public:
	ModelAOWorker(const std::vector<pragma::util::ParallelJob<pragma::image::ImageLayerSet>> &matAoJobs) : m_matAoJobs {matAoJobs}
	{
		AddThread([this]() {
			auto numJobs = m_matAoJobs.size();
			auto lastProgress = -1.f;
			auto numSuccessful = 0;
			while(m_matAoJobs.empty() == false) {
				if(IsCancelled() == false) {
					auto &t = m_matAoJobs.front();
					t.Start();
					while(t.IsComplete() == false) {
						t.Poll();
						auto jobProgress = t.GetProgress();
						auto totalProgress = jobProgress;
						totalProgress /= static_cast<float>(numJobs);
						if(totalProgress != lastProgress) {
							lastProgress = totalProgress;
							UpdateProgress(totalProgress);
						}
						if(jobProgress < 1.f)
							pragma::util::sleep_for_seconds(1);
						if(IsCancelled())
							t.Cancel();
					}
					t.Poll(); // We need to poll one more time after completion to make sure completion handler is called
					if(t.IsSuccessful())
						++numSuccessful; // Technically the completion handler could still fail, but we'll just ignore that case for now
				}
				m_matAoJobs.erase(m_matAoJobs.begin());
			}
			SetStatus((numSuccessful > 0) ? pragma::util::JobStatus::Successful : pragma::util::JobStatus::Failed);
		});
	}
	virtual pragma::asset::ModelAOWorkerResult GetResult() override { return m_result; }
  private:
	template<typename TJob, typename... TARGS>
	friend pragma::util::ParallelJob<typename TJob::RESULT_TYPE> pragma::util::create_parallel_job(TARGS &&...args);
	std::vector<pragma::util::ParallelJob<pragma::image::ImageLayerSet>> m_matAoJobs {};
	pragma::asset::ModelAOWorkerResult m_result {};
};
std::optional<pragma::util::ParallelJob<pragma::asset::ModelAOWorkerResult>> pragma::asset::generate_ambient_occlusion(Model &mdl, std::string &outErrMsg, bool forceRebuild, uint32_t aoResolution, uint32_t aoSamples, rendering::cycles::SceneInfo::DeviceType aoDevice)
{
	std::vector<util::ParallelJob<image::ImageLayerSet>> aoJobs {};
	std::unordered_set<std::string> builtRMAs {};
	auto &materials = mdl.GetMaterials();
	aoJobs.reserve(materials.size());
	for(auto &mat : materials) {
		if(!mat)
			continue;
		auto *rmaMap = mat->GetRMAMap();
		if(rmaMap == nullptr)
			continue;
		if(builtRMAs.find(rmaMap->name) != builtRMAs.end())
			continue; // AO has already been built (maybe by a different skin material)
		builtRMAs.insert(rmaMap->name);
		util::ParallelJob<image::ImageLayerSet> aoJob {};
		auto eResult = generate_ambient_occlusion(mdl, *mat.get(), aoJob, outErrMsg, forceRebuild, aoResolution, aoSamples, aoDevice);
		if(eResult != AOResult::AOJobReady)
			continue;
		aoJobs.push_back(aoJob);
	}
	if(aoJobs.empty())
		return {};
	return util::create_parallel_job<ModelAOWorker>(aoJobs);
}

template<class T>
static bool save_ambient_occlusion(pragma::material::Material &mat, std::string rmaPath, T &img, std::string &errMsg)
{
	auto *shaderComposeRMA = static_cast<pragma::ShaderComposeRMA *>(pragma::get_cengine()->GetShader("compose_rma").get());
	if(shaderComposeRMA == nullptr)
		return false;
	ufile::remove_extension_from_filename(rmaPath);

	auto outPath = pragma::util::Path {rmaPath};
	auto originalRmaPath = outPath.GetString();
	auto requiresSave = false;
	if(outPath.GetFront() == "pbr") {
		// We don't want to overwrite the default pbr materials, so we'll use a different rma path
		outPath = mat.GetName();
		outPath.RemoveFileExtension();
		rmaPath = outPath.GetString() + "_rma";
		mat.SetTextureProperty(pragma::material::ematerial::RMA_MAP_IDENTIFIER, rmaPath);
		requiresSave = true;
	}

	if(shaderComposeRMA->InsertAmbientOcclusion(pragma::get_cengine()->GetRenderContext(), originalRmaPath, img, &rmaPath) == false) {
		errMsg = "Unable to insert ambient occlusion data into RMA map!";
		return false;
	}

	mat.ClearProperty("rma_info/requires_ao_update");
	mat.ClearProperty("ao_map");

	if(requiresSave) {
		mat.UpdateTextures();
		std::string err;
		mat.Save(err);
	}
	return true;
}

pragma::asset::AOResult pragma::asset::generate_ambient_occlusion(Model &mdl, material::Material &mat, util::ParallelJob<image::ImageLayerSet> &outJob, std::string &outErrMsg, bool forceRebuild, uint32_t aoResolution, uint32_t aoSamples,
  rendering::cycles::SceneInfo::DeviceType aoDevice)
{
	// TODO: There really is no good way to determine whether the material has a ambient occlusion map or not.
	// Use a compute shader to determine if it's all white or black?
	// On the other hand a rma texture should be unique to a model, so does it really matter?

	auto requiresAOUpdate = false;
	mat.GetProperty("rma_info/requires_ao_update", &requiresAOUpdate);
	if(forceRebuild == false && requiresAOUpdate == false)
		return AOResult::NoAOGenerationRequired;
	auto *rmaTexInfo = mat.GetRMAMap();
	if(rmaTexInfo == nullptr || std::static_pointer_cast<material::Texture>(rmaTexInfo->texture) == nullptr) {
		outErrMsg = "Material has no RMA texture assigned!";
		return AOResult::FailedToCreateAOJob; // This is not a valid pbr material?
	}
	auto rmaTex = std::static_pointer_cast<material::Texture>(rmaTexInfo->texture);
	auto rmaPath = rmaTex->GetName();
	auto *shaderComposeRMA = static_cast<ShaderComposeRMA *>(get_cengine()->GetShader("compose_rma").get());
	if(shaderComposeRMA == nullptr) {
		outErrMsg = "Unable to load RMA shader!";
		return AOResult::FailedToCreateAOJob;
	}
	auto &materials = mdl.GetMaterials();
	auto it = std::find_if(materials.begin(), materials.end(), [&mat](const material::MaterialHandle &hMat) { return &mat == hMat.get(); });
	if(it == materials.end()) {
		outErrMsg = "Material not found in model!";
		return AOResult::FailedToCreateAOJob;
	}
	auto matIdx = (it - materials.begin());

	rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.denoise = true;
	sceneInfo.hdrOutput = false;
	sceneInfo.width = aoResolution;
	sceneInfo.height = aoResolution;
	sceneInfo.samples = aoSamples;
	sceneInfo.device = aoDevice;

	//std::shared_ptr<image::ImageBuffer> aoImg = nullptr;
	outJob = pragma::rendering::cycles::bake_ambient_occlusion(*get_client_state(), sceneInfo, mdl, matIdx);
	if(outJob.IsValid() == false) {
		outErrMsg = "Unable to create job for ao generation!";
		return AOResult::FailedToCreateAOJob;
	}

	auto *texInfoAo = mat.GetTextureInfo("ao_map");
	if(texInfoAo && texInfoAo->texture && std::static_pointer_cast<material::Texture>(texInfoAo->texture)->HasValidVkTexture()) {
		// Material already has a separate ambient occlusion map, just use that one
		auto &img = std::static_pointer_cast<material::Texture>(texInfoAo->texture)->GetVkTexture()->GetImage();
		std::string errMsg;
		auto result = save_ambient_occlusion<prosper::IImage>(mat, rmaPath, img, errMsg);
		if(result)
			return AOResult::NoAOGenerationRequired;
		// Failed; Just try to generate?
	}

	auto hMat = mat.GetHandle();
	outJob.SetCompletionHandler([rmaPath, hMat](util::ParallelWorker<image::ImageLayerSet> &worker) mutable {
		auto *shaderComposeRMA = static_cast<ShaderComposeRMA *>(get_cengine()->GetShader("compose_rma").get());
		if(worker.IsSuccessful() == false)
			return;
		if(!hMat) {
			worker.SetStatus(util::JobStatus::Failed, "Material is not valid!");
			return;
		}
		if(shaderComposeRMA == nullptr) {
			worker.SetStatus(util::JobStatus::Failed, "Shader is not valid!");
			return;
		}
		auto aoImg = worker.GetResult().images.begin()->second;
		std::string errMsg;
		auto result = save_ambient_occlusion<image::ImageBuffer>(*hMat.get(), rmaPath, *aoImg, errMsg);
		if(result == false)
			worker.SetStatus(util::JobStatus::Failed, errMsg);
	});
	return AOResult::AOJobReady;
}

bool pragma::asset::export_texture_as_vtf(const std::string &fileName, const std::function<const uint8_t *(uint32_t, uint32_t)> &fGetImgData, uint32_t width, uint32_t height, uint32_t szPerPixel, uint32_t numLayers, uint32_t numMipmaps, bool cubemap, const VtfInfo &texInfo,
  const std::function<void(const std::string &)> &errorHandler, bool absoluteFileName)
{
	auto dllHandle = pragma::util::initialize_external_archive_manager(get_client_state());
	if(!dllHandle)
		return false;
	auto *fExportVtf = dllHandle->FindSymbolAddress<bool (*)(const std::string &, const std::function<const uint8_t *(uint32_t, uint32_t)> &, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, bool, const VtfInfo &, const std::function<void(const std::string &)> &, bool)>("export_vtf");
	if(fExportVtf == nullptr)
		return false;
	return fExportVtf(fileName, fGetImgData, width, height, szPerPixel, numLayers, numMipmaps, cubemap, texInfo, errorHandler, absoluteFileName);
}

std::optional<prosper::Format> pragma::asset::vtf_format_to_prosper(VtfInfo::Format format)
{
	switch(format) {
	case VtfInfo::Format::Bc1:
		return prosper::Format::BC1_RGB_UNorm_Block;
	case VtfInfo::Format::Bc1a:
		return prosper::Format::BC1_RGBA_UNorm_Block;
	case VtfInfo::Format::Bc2:
		return prosper::Format::BC2_UNorm_Block;
	case VtfInfo::Format::Bc3:
		return prosper::Format::BC3_UNorm_Block;
	case VtfInfo::Format::R8G8B8A8_UNorm:
		return prosper::Format::R8G8B8A8_UNorm;
	case VtfInfo::Format::B8G8R8A8_UNorm:
		// vkImgData.swizzle = {prosper::ComponentSwizzle::B,prosper::ComponentSwizzle::G,prosper::ComponentSwizzle::R,prosper::ComponentSwizzle::A};
		return prosper::Format::B8G8R8A8_UNorm;
	case VtfInfo::Format::R8G8_UNorm:
		return prosper::Format::R8G8_UNorm;
	case VtfInfo::Format::R16G16B16A16_SFloat:
		return prosper::Format::R16G16B16A16_SFloat;
	case VtfInfo::Format::R32G32B32A32_SFloat:
		return prosper::Format::R32G32B32A32_SFloat;
	case VtfInfo::Format::A8B8G8R8_UNorm_Pack32:
		// vkImgData.swizzle = {prosper::ComponentSwizzle::A,prosper::ComponentSwizzle::B,prosper::ComponentSwizzle::G,prosper::ComponentSwizzle::R};
		return prosper::Format::A8B8G8R8_UNorm_Pack32;
	}
	static_assert(math::to_integral(VtfInfo::Format::Count) == 10, "Update this implementation when new format types have been added!");
	return {};
}
std::optional<pragma::asset::VtfInfo::Format> pragma::asset::prosper_format_to_vtf(prosper::Format format)
{
	switch(format) {
	case prosper::Format::BC1_RGB_UNorm_Block:
		return VtfInfo::Format::Bc1;
	case prosper::Format::BC1_RGBA_UNorm_Block:
		return VtfInfo::Format::Bc1a;
	case prosper::Format::BC2_UNorm_Block:
		return VtfInfo::Format::Bc2;
	case prosper::Format::BC3_UNorm_Block:
		return VtfInfo::Format::Bc3;
	case prosper::Format::R8G8B8A8_UNorm:
		return VtfInfo::Format::R8G8B8A8_UNorm;
	case prosper::Format::B8G8R8A8_UNorm:
		// vkImgData.swizzle = {prosper::ComponentSwizzle::B,prosper::ComponentSwizzle::G,prosper::ComponentSwizzle::R,prosper::ComponentSwizzle::A};
		return VtfInfo::Format::B8G8R8A8_UNorm;
	case prosper::Format::R8G8_UNorm:
		return VtfInfo::Format::R8G8_UNorm;
	case prosper::Format::R16G16B16A16_SFloat:
		return VtfInfo::Format::R16G16B16A16_SFloat;
	case prosper::Format::R32G32B32A32_SFloat:
		return VtfInfo::Format::R32G32B32A32_SFloat;
	case prosper::Format::A8B8G8R8_UNorm_Pack32:
		// vkImgData.swizzle = {prosper::ComponentSwizzle::A,prosper::ComponentSwizzle::B,prosper::ComponentSwizzle::G,prosper::ComponentSwizzle::R};
		return VtfInfo::Format::A8B8G8R8_UNorm_Pack32;
	}
	static_assert(math::to_integral(VtfInfo::Format::Count) == 10, "Update this implementation when new format types have been added!");
	return {};
}

bool pragma::asset::export_texture_as_vtf(const std::string &fileName, const prosper::IImage &img, const VtfInfo &texInfo, const std::function<void(const std::string &)> &errorHandler, bool absoluteFileName)
{
	auto inputFormat = prosper_format_to_vtf(img.GetFormat());
	if(inputFormat.has_value() == false)
		return false; // TODO: Convert into a compatible format
	auto fGetImgData = prosper::util::image_to_data(const_cast<prosper::IImage &>(img), img.GetFormat());
	if(fGetImgData == nullptr)
		return false;
	auto width = img.GetWidth();
	auto height = img.GetHeight();
	auto numLayers = img.GetLayerCount();
	auto numMipmaps = img.GetMipmapCount();
	auto cubemap = img.IsCubemap();
	auto format = img.GetFormat();
	auto sizePerPixel = prosper::util::is_compressed_format(format) ? prosper::util::get_block_size(format) : prosper::util::get_byte_size(format);
	std::function<void(void)> deleter = nullptr;
	auto nTexInfo = texInfo;
	nTexInfo.inputFormat = *inputFormat;
	auto result = export_texture_as_vtf(fileName, [&fGetImgData, &deleter](uint32_t layerId, uint32_t mipmapIdx) -> const uint8_t * { return fGetImgData(layerId, mipmapIdx, deleter); }, width, height, sizePerPixel, numLayers, numMipmaps, cubemap, nTexInfo, errorHandler, absoluteFileName);
	if(deleter)
		deleter();
	return result;
}
