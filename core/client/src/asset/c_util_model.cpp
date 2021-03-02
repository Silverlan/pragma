/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/clientstate/clientstate.h"
#include "pragma/asset/c_util_model.hpp"
#include "pragma/util/util_image.hpp"
#include "pragma/rendering/shaders/util/c_shader_compose_rma.hpp"
#include "pragma/rendering/shaders/util/c_shader_specular_glossiness_to_metalness_roughness.hpp"
#include "c_gltf_writer.hpp"
#include <pragma/entities/entity_component_system_t.hpp>
#include <pragma/model/animation/vertex_animation.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/alpha_mode.hpp>
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_parallel_job.hpp>
#include <pragma/asset_types/world.hpp>
#include <pragma/engine_version.h>
#include <image/prosper_sampler.hpp>
#include <util_image.hpp>
#include <cmaterialmanager.h>

extern DLLCLIENT CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

void pragma::asset::MapExportInfo::AddCamera(CCameraComponent &cam) {m_cameras.push_back(cam.GetHandle<CCameraComponent>());}
void pragma::asset::MapExportInfo::AddLightSource(CLightComponent &light) {m_lightSources.push_back(light.GetHandle<CLightComponent>());}
void pragma::asset::ModelExportInfo::SetAnimationList(const std::vector<std::string> &animations)
{
	exportAnimations = true;
	m_animations = animations;
}
std::vector<std::string> *pragma::asset::ModelExportInfo::GetAnimationList() {return m_animations.has_value() ? &*m_animations : nullptr;}

#include <tiny_gltf.h>
struct GLTFBufferData
{
	tinygltf::Accessor &accessor;
	tinygltf::BufferView &bufferView;
	tinygltf::Buffer &buffer;
	template<typename T>
		T GetValue(uint32_t offset,const T &default={}) const
	{
		offset += bufferView.byteOffset +accessor.byteOffset;
		if(offset +sizeof(T) >= buffer.data.size())
			return default;
		return *reinterpret_cast<T*>(buffer.data.data() +offset);
	}
	template<typename T>
		T GetIndexedValue(uint32_t index,const T &default={}) const
	{
		auto stride = bufferView.byteStride;
		if(stride == 0)
			stride = sizeof(T);
		return GetValue(index *stride,default);
	}
	template<uint32_t C>
		std::array<int64_t,C> GetIntArray(uint32_t index) const
	{
		auto stride = bufferView.byteStride;
		auto componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
		if(stride == 0)
			stride = componentSize *C;
		auto offset = index *stride;
		std::array<int64_t,C> result {};
		for(auto i=decltype(C){0u};i<C;++i)
		{
			int64_t value = 0;
			switch(accessor.componentType)
			{
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
		std::array<float,C> GetFloatArray(uint32_t index) const
	{
		auto stride = bufferView.byteStride;
		auto componentSize = tinygltf::GetComponentSizeInBytes(accessor.componentType);
		if(stride == 0)
			stride = componentSize *C;
		auto offset = index *stride;
		std::array<float,C> result {};
		for(auto i=decltype(C){0u};i<C;++i)
		{
			float value = 0.f;
			switch(accessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_FLOAT:
				value = GetValue<float>(offset);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
				value = GetValue<uint8_t>(offset) /static_cast<float>(std::numeric_limits<uint8_t>::max());
				break;
			case TINYGLTF_COMPONENT_TYPE_BYTE:
				value = umath::max(GetValue<int8_t>(offset) /static_cast<float>(std::numeric_limits<int8_t>::max()),-1.f);
				break;
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
				value = GetValue<uint16_t>(offset) /static_cast<float>(std::numeric_limits<uint16_t>::max());
				break;
			case TINYGLTF_COMPONENT_TYPE_SHORT:
				value = umath::max(GetValue<int16_t>(offset) /static_cast<float>(std::numeric_limits<int16_t>::max()),-1.f);
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
		return Vector3{v[0],v[1],v[2]};
	}
	Quat GetQuat(uint32_t offset) const
	{
		auto v = GetFloatArray<4>(offset);
		return Quat{v[3],v[0],v[1],v[2]};
	}
	Vector4 GetVector4(uint32_t offset) const
	{
		auto v = GetFloatArray<4>(offset);
		return Vector4{v[0],v[1],v[2],v[3]};
	}
};

static uimg::TextureInfo get_texture_write_info(
	pragma::asset::ModelExportInfo::ImageFormat imageFormat,bool normalMap,bool srgb,uimg::TextureInfo::AlphaMode alphaMode,std::string &outExt
)
{
	uimg::TextureInfo texWriteInfo {};
	switch(imageFormat)
	{
	case pragma::asset::ModelExportInfo::ImageFormat::DDS:
		texWriteInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
		outExt = "dds";
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::KTX:
		texWriteInfo.containerFormat = uimg::TextureInfo::ContainerFormat::KTX;
		outExt = "ktx";
		break;
	}
	texWriteInfo.flags = uimg::TextureInfo::Flags::GenerateMipmaps;
	texWriteInfo.alphaMode = alphaMode;
	// umath::set_flag(texWriteInfo.flags,uimg::TextureInfo::Flags::SRGB,srgb);
	texWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::KeepInputImageFormat;
	texWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::KeepInputImageFormat;
	if(normalMap)
		texWriteInfo.SetNormalMap();
	return texWriteInfo;
}

static bool save_image(
	uimg::ImageBuffer &imgBuf,pragma::asset::ModelExportInfo::ImageFormat imageFormat,std::string &inOutImgOutputPath,bool normalMap,bool srgb,uimg::TextureInfo::AlphaMode alphaMode
)
{
	imgBuf.Convert(uimg::ImageBuffer::Format::RGBA8); // TODO

	if(imageFormat == pragma::asset::ModelExportInfo::ImageFormat::DDS || imageFormat == pragma::asset::ModelExportInfo::ImageFormat::KTX)
	{
		std::string ext;
		auto texWriteInfo = get_texture_write_info(imageFormat,normalMap,srgb,alphaMode,ext);
		texWriteInfo.inputFormat = uimg::TextureInfo::InputFormat::R8G8B8A8_UInt;
		inOutImgOutputPath += '.' +ext;
		return c_game->SaveImage(imgBuf,inOutImgOutputPath,texWriteInfo);
	}

	auto saveFormat = uimg::ImageFormat::PNG;
	switch(imageFormat)
	{
	case pragma::asset::ModelExportInfo::ImageFormat::PNG:
		saveFormat = uimg::ImageFormat::PNG;
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::BMP:
		saveFormat = uimg::ImageFormat::BMP;
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::TGA:
		saveFormat = uimg::ImageFormat::TGA;
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::JPG:
		saveFormat = uimg::ImageFormat::JPG;
		break;
	case pragma::asset::ModelExportInfo::ImageFormat::HDR:
		saveFormat = uimg::ImageFormat::HDR;
		break;
	}
	auto ext = uimg::get_file_extension(saveFormat);
	inOutImgOutputPath += '.' +ext;

	if(alphaMode == uimg::TextureInfo::AlphaMode::None)
		imgBuf.Convert(uimg::ImageBuffer::ToRGBFormat(imgBuf.GetFormat()));
	FileManager::CreatePath(ufile::get_path_from_filename(inOutImgOutputPath).c_str());
	auto fImg = FileManager::OpenFile<VFilePtrReal>(inOutImgOutputPath.c_str(),"wb");
	if(fImg == nullptr)
		return false;
	return uimg::save_image(fImg,imgBuf,saveFormat);
};

struct GLTFInputData
{
	std::string path;
	std::vector<std::shared_ptr<prosper::Texture>> textures {};
};

static bool load_image(
	tinygltf::Image *image,const int imageIdx,std::string *outErr,std::string *outWarn,int reqWidth,int reqHeight,const unsigned char *bytes,int size, void *userData
)
{
	if(image->mimeType.empty() == false)
	{
		// Embedded image; Let tinygltf handle it
		return tinygltf::LoadImageData(image,imageIdx,outErr,outWarn,reqWidth,reqHeight,bytes,size,nullptr);
	}
	auto &inputData = *static_cast<GLTFInputData*>(userData);
	auto imgPath = inputData.path +image->uri;
	auto f = FileManager::OpenSystemFile(imgPath.c_str(),"rb");
	if(f == nullptr)
		return false;
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly | TextureLoadFlags::DontCache;
	std::shared_ptr<void> texture = nullptr;
	auto &texManager = static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager();
	if(texManager.Load(c_engine->GetRenderContext(),image->uri,f,loadInfo,&texture) == false)
		return false;
	if(std::static_pointer_cast<Texture>(texture)->HasValidVkTexture() == false)
		return false;
	if(imageIdx >= inputData.textures.size())
		inputData.textures.resize(imageIdx +1);
	inputData.textures.at(imageIdx) = std::static_pointer_cast<Texture>(texture)->GetVkTexture();
	return true;
}

static std::shared_ptr<Model> import_model(VFilePtr optFile,const std::string &optFileName,std::string &outErrMsg,const util::Path &outputPath)
{
	auto verbose = true; // TODO
	auto scale = static_cast<float>(util::pragma::metres_to_units(1.f));

	std::string fileName = optFileName;
	std::string absPathToFile;
	if(optFile)
	{
		auto fptrReal = std::static_pointer_cast<VFilePtrInternalReal>(optFile);
		if(fptrReal)
		{
			fileName = fptrReal->GetPath();
			absPathToFile = fileName;
			fileName = ufile::get_file_from_filename(fileName);
		}
		else
		{
			fileName = "unknown";
			absPathToFile = FileManager::GetProgramPath() +'/' +fileName;
		}
	}
	else
		absPathToFile = FileManager::GetProgramPath() +'/' +fileName;
	auto absPath = ufile::get_path_from_filename(absPathToFile);

	auto mdlName = ufile::get_file_from_filename(fileName);
	ufile::remove_extension_from_filename(mdlName);
	auto matPath = "materials/models/" +mdlName +'/';

	tinygltf::TinyGLTF reader {};

	GLTFInputData inputData {};
	inputData.path = absPath;

	reader.SetImageLoader(load_image,&inputData);

	std::string err;
	std::string warn;

	if(verbose)
		Con::cout<<"Loading file '"<<absPathToFile<<"'..."<<Con::endl;

	tinygltf::Model gltfMdl {};
	auto result = false;
	if(optFile)
	{
		std::vector<uint8_t> fileData {};
		auto sz = optFile->GetSize();
		fileData.resize(sz);
		optFile->Read(fileData.data(),fileData.size());
		auto binary = (sz >= 4) ? ustring::compare(reinterpret_cast<char*>(fileData.data()),"glTF",true,4) : false;
		result = binary ? reader.LoadBinaryFromMemory(&gltfMdl,&err,&warn,fileData.data(),fileData.size(),absPath) :
			reader.LoadASCIIFromString(&gltfMdl,&err,&warn,reinterpret_cast<char*>(fileData.data()),fileData.size(),absPath);
	}
	else
	{
		std::string ext;
		ufile::get_extension(fileName,&ext);

		auto binary = ustring::compare(ext,"glb",false);
		result = binary ? reader.LoadBinaryFromFile(&gltfMdl,&err,&warn,absPathToFile) : reader.LoadASCIIFromFile(&gltfMdl,&err,&warn,absPathToFile);
	}
	if(verbose)
	{
		if(result)
			Con::cout<<"Successfully loaded file '"<<absPathToFile<<"'! Creating model..."<<Con::endl;
		else if(err.empty() == false)
			Con::cwar<<"WARNING: Unable to load file '"<<absPathToFile<<"': "<<err<<Con::endl;
		else
			Con::cwar<<"WARNING: Unable to load file '"<<absPathToFile<<"': "<<warn<<Con::endl;
	}
	if(result == false)
	{
		if(err.empty() == false)
			outErrMsg = err;
		else
			outErrMsg = warn;
		return nullptr;
	}

	auto TransformPos = [scale](const Vector3 &v) -> Vector3 {
		return v *scale;
	};

	auto mdl = std::shared_ptr<Model>{c_game->CreateModel(false)};
	mdl->GetBaseMeshes() = {0u};

	// Materials
	mdl->CreateTextureGroup();
	// TODO: Load materials/textures lazily
	auto numImages = gltfMdl.images.size();
	if(numImages >= inputData.textures.size())
		inputData.textures.resize(numImages);
	for(auto i=decltype(inputData.textures.size()){0u};i<inputData.textures.size();++i)
	{
		auto &tex = inputData.textures.at(i);

		auto &gltfImg = gltfMdl.images.at(i);
		if(gltfImg.image.empty() == false)
		{
			// It's an embedded image
			auto *data = gltfImg.image.data();

			std::shared_ptr<uimg::ImageBuffer> imgBuf = nullptr;
			auto numPixels = gltfImg.width *gltfImg.height;
			auto bytesPerChannel = gltfImg.bits /8;
			auto format = uimg::ImageBuffer::Format::RGBA8;
			if(bytesPerChannel == 2)
				format = uimg::ImageBuffer::Format::RGBA16;
			else if(bytesPerChannel == 4)
				format = uimg::ImageBuffer::Format::RGBA32;
			auto numChannels = gltfImg.component;
			if(numChannels < 4)
			{
				imgBuf = uimg::ImageBuffer::Create(gltfImg.width,gltfImg.height,format);
				auto *convertedData = static_cast<uint8_t*>(imgBuf->GetData());
				for(auto i=decltype(numPixels){0u};i<numPixels;++i)
				{
					auto *pIn = data +i *bytesPerChannel *numChannels;
					auto *pOut = convertedData +i *bytesPerChannel *4;
					memcpy(pOut,pIn,bytesPerChannel *numChannels);
				}
			}
			else
				imgBuf = uimg::ImageBuffer::Create(data,gltfImg.width,gltfImg.height,format);

			auto img = c_engine->GetRenderContext().CreateImage(*imgBuf);
			prosper::util::TextureCreateInfo texCreateInfo {};
			prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
			prosper::util::SamplerCreateInfo samplerCreateInfo {};
			tex = c_engine->GetRenderContext().CreateTexture(texCreateInfo,*img,imgViewCreateInfo,samplerCreateInfo);
		}
	}
	uint32_t matIdx = 0;
	for(auto &gltfMat : gltfMdl.materials)
	{
		auto name = gltfMat.name;
		if(name.empty())
			name = "material_" +std::to_string(matIdx);

		enum class AlphaMode : uint8_t
		{
			Opaque = 0,
			Mask,
			Blend
		};
		auto alphaMode = AlphaMode::Opaque;
		if(gltfMat.alphaMode == "OPAQUE")
			alphaMode = AlphaMode::Opaque;
		else if(gltfMat.alphaMode == "MASK")
			alphaMode = AlphaMode::Mask;
		else if(gltfMat.alphaMode == "BLEND")
			alphaMode = AlphaMode::Blend;

		auto fGetTextureInfo = [](bool isGreyScale,bool isNormalMap,AlphaMode alphaMode=AlphaMode::Opaque) -> uimg::TextureInfo {
			uimg::TextureInfo texInfo {};
			texInfo.containerFormat = uimg::TextureInfo::ContainerFormat::DDS;
			texInfo.flags |= uimg::TextureInfo::Flags::GenerateMipmaps;
			if(isGreyScale)
				texInfo.outputFormat = uimg::TextureInfo::OutputFormat::GradientMap;
			else
			{
				switch(alphaMode)
				{
				case AlphaMode::Opaque:
					texInfo.outputFormat = uimg::TextureInfo::OutputFormat::BC1;
					break;
				case AlphaMode::Mask:
					texInfo.outputFormat = uimg::TextureInfo::OutputFormat::BC1a;
					break;
				case AlphaMode::Blend:
					texInfo.outputFormat = uimg::TextureInfo::OutputFormat::BC3;
					break;
				}
			}
			if(isNormalMap)
				texInfo.SetNormalMap();
			return texInfo;
		};

		auto fGetTexture = [&gltfMdl,&inputData](int32_t index) -> std::shared_ptr<prosper::Texture> {
			if(index == -1)
				return nullptr;
			return inputData.textures.at(gltfMdl.textures.at(index).source);
		};

		auto fGetImage = [&fGetTexture,&inputData](int32_t index) -> std::shared_ptr<prosper::IImage> {
			auto tex = fGetTexture(index);
			if(tex == nullptr)
				return nullptr;
			return tex->GetImage().shared_from_this();
		};

		auto matName = matPath +name;
		util::Path matPathRelative {matName};
		matPathRelative.PopFront();

		auto *mat = static_cast<CMaterial*>(client->CreateMaterial(matPathRelative.GetString(),"pbr"));
		auto &dataBlock = mat->GetDataBlock();
		dataBlock->AddValue("int","alpha_mode",std::to_string(umath::to_integral(alphaMode)));
		dataBlock->AddValue("float","alpha_cutoff",std::to_string(gltfMat.alphaCutoff));

		std::string textureRootPath = "addons/converted/";

		auto fWriteImage = [mat,&fGetTextureInfo,&textureRootPath](
			const std::string &matIdentifier,const std::string &texName,prosper::IImage &img,bool greyScale,bool normalMap,AlphaMode alphaMode=AlphaMode::Opaque
			) {
				auto texInfo = fGetTextureInfo(greyScale,normalMap,alphaMode);
				c_game->SaveImage(img,textureRootPath +texName,texInfo);

				util::Path albedoPath {texName};
				albedoPath.PopFront();
				mat->SetTexture(matIdentifier,albedoPath.GetString());
		};

		auto isHandled = false;
		for(auto &pair : gltfMat.extensions)
		{
			if(pair.first == "KHR_materials_pbrSpecularGlossiness")
			{
				if(pair.second.IsObject())
				{
					pragma::ShaderSpecularGlossinessToMetalnessRoughness::PushConstants pushConstants {};
					if(pair.second.Has("diffuseFactor"))
					{
						auto &vDiffuseFactor = pair.second.Get("diffuseFactor");
						if(vDiffuseFactor.IsArray())
						{
							for(uint8_t i=0;i<4;++i)
								pushConstants.diffuseFactor[i] = vDiffuseFactor.Get(i).GetNumberAsDouble();
						}
					}
					if(pair.second.Has("specularFactor"))
					{
						auto &vSpecularFactor = pair.second.Get("specularFactor");
						if(vSpecularFactor.IsArray())
						{
							for(uint8_t i=0;i<3;++i)
								pushConstants.specularFactor[i] = vSpecularFactor.Get(i).GetNumberAsDouble();
						}
					}
					if(pair.second.Has("glossinessFactor"))
						pushConstants.specularFactor.a = pair.second.Get("glossinessFactor").GetNumberAsDouble();

					prosper::Texture *diffuseTex = nullptr;
					if(pair.second.Has("diffuseTexture"))
					{
						auto diffuseTexture = pair.second.Get("diffuseTexture");
						if(diffuseTexture.Has("index"))
						{
							auto diffuseIndex = static_cast<int32_t>(diffuseTexture.Get("index").GetNumberAsInt());
							diffuseTex = fGetTexture(diffuseIndex).get();
						}
					}

					prosper::Texture *specularGlossinessTex = nullptr;
					if(pair.second.Has("specularGlossinessTexture"))
					{
						auto specularGlossinessTexture = pair.second.Get("specularGlossinessTexture");
						if(specularGlossinessTexture.Has("index"))
						{
							auto specularGlossinessIndex = static_cast<int32_t>(specularGlossinessTexture.Get("index").GetNumberAsInt());
							specularGlossinessTex = fGetTexture(specularGlossinessIndex).get();
						}
					}

					auto occlusionTex = fGetTexture(gltfMat.occlusionTexture.index);
					if(occlusionTex == nullptr)
					{
						auto rmaInfo = dataBlock->AddBlock("rma_info");
						if(rmaInfo)
							rmaInfo->AddValue("bool","requires_ao_update","1");
					}

					auto *shader = static_cast<pragma::ShaderSpecularGlossinessToMetalnessRoughness*>(c_engine->GetShader("specular_glossiness_to_metalness_roughness").get());
					if(shader)
					{
						auto metallicRoughnessSet = shader->ConvertToMetalnessRoughness(
							c_engine->GetRenderContext(),diffuseTex,specularGlossinessTex,pushConstants,occlusionTex.get()
						);
						if(metallicRoughnessSet.has_value())
						{
							fWriteImage(Material::ALBEDO_MAP_IDENTIFIER,matName +"_albedo",*metallicRoughnessSet->albedoMap,false /* greyScale */,false /* normalMap */,alphaMode);
							fWriteImage(Material::RMA_MAP_IDENTIFIER,matName +"_rma",*metallicRoughnessSet->rmaMap,false /* greyScale */,false /* normalMap */);
						}
					}

					isHandled = true;
				}
				break;
			}
		}

		if(isHandled == false)
		{
			auto &baseColorTexture = gltfMat.pbrMetallicRoughness.baseColorTexture;
			auto baseColorImage = fGetImage(baseColorTexture.index);
			if(baseColorImage)
				fWriteImage(Material::ALBEDO_MAP_IDENTIFIER,matName +"_albedo",*baseColorImage,false /* greyScale */,false /* normalMap */,alphaMode);
			else
				mat->SetTexture(Material::ALBEDO_MAP_IDENTIFIER,"white");

			auto &baseColorFactor = gltfMat.pbrMetallicRoughness.baseColorFactor;
			if(baseColorFactor != std::vector<double>{1.0,1.0,1.0,1.0})
			{
				dataBlock->AddValue("color","color",
					std::to_string(baseColorFactor.at(0) *255.f) +' ' +
					std::to_string(baseColorFactor.at(1) *255.f) +' ' +
					std::to_string(baseColorFactor.at(2) *255.f) +' ' +
					std::to_string(baseColorFactor.at(3) *255.f)
				);
			}

			auto metallicRoughnessImg = fGetImage(gltfMat.pbrMetallicRoughness.metallicRoughnessTexture.index);
			if(metallicRoughnessImg)
			{
				auto rmaName = matName +"_rma";
				fWriteImage(Material::RMA_MAP_IDENTIFIER,rmaName,*metallicRoughnessImg,false /* greyScale */,false /* normalMap */);

				auto occlusionImg = fGetImage(gltfMat.occlusionTexture.index);
				if(occlusionImg)
				{
					// Separate ao texture; Merge it with rma texture
					auto *shaderComposeRMA = static_cast<pragma::ShaderComposeRMA*>(c_engine->GetShader("compose_rma").get());
					if(shaderComposeRMA)
						shaderComposeRMA->InsertAmbientOcclusion(c_engine->GetRenderContext(),rmaName,*occlusionImg);
				}
				else
				{
					auto rmaInfo = dataBlock->AddBlock("rma_info");
					if(rmaInfo)
						rmaInfo->AddValue("bool","requires_ao_update","1");
				}
			}

			dataBlock->AddValue("float","roughness_factor",std::to_string(gltfMat.pbrMetallicRoughness.roughnessFactor));
			dataBlock->AddValue("float","metalness_factor",std::to_string(gltfMat.pbrMetallicRoughness.metallicFactor));
		}

		if(gltfMat.normalTexture.index != -1)
		{
			auto &tex = inputData.textures.at(gltfMdl.textures.at(gltfMat.normalTexture.index).source);
			if(tex)
				fWriteImage(Material::NORMAL_MAP_IDENTIFIER,matName +"_normal",tex->GetImage(),false /* greyScale */,true /* normalMap */);
		}

		if(gltfMat.emissiveTexture.index != -1)
		{
			auto &tex = inputData.textures.at(gltfMdl.textures.at(gltfMat.emissiveTexture.index).source);
			if(tex)
				fWriteImage(Material::EMISSION_MAP_IDENTIFIER,matName +"_emission",tex->GetImage(),false /* greyScale */,false /* normalMap */);
		}
		auto &emissiveFactor = gltfMat.emissiveFactor;
		if(emissiveFactor != std::vector<double>{1.0,1.0,1.0,1.0})
		{
			dataBlock->AddValue("vector","emission_factor",
				std::to_string(emissiveFactor.at(0)) +' ' +
				std::to_string(emissiveFactor.at(1)) +' ' +
				std::to_string(emissiveFactor.at(2))
			);
		}

		mat->UpdateTextures();
		mat->Save(matPathRelative.GetString(),"addons/converted/");

		mdl->AddMaterial(0,mat);

		++matIdx;
	}

	auto fGetBufferData = [&gltfMdl](int accessorIdx) -> GLTFBufferData {
		auto &accessor = gltfMdl.accessors.at(accessorIdx);
		auto &bufView = gltfMdl.bufferViews.at(accessor.bufferView);
		auto &buf = gltfMdl.buffers.at(bufView.buffer);
		return GLTFBufferData{accessor,bufView,buf};
	};

	auto &gltfMeshes = gltfMdl.meshes;
	auto meshGroup = mdl->AddMeshGroup("reference");
	uint32_t absUnnamedFcIdx = 0;
	for(auto &gltfMesh : gltfMeshes)
	{
		auto mesh = c_game->CreateModelMesh();
		for(auto &primitive : gltfMesh.primitives)
		{
			auto itPos = primitive.attributes.find("POSITION");
			if(itPos == primitive.attributes.end())
				continue;
			auto &idxAccessor = gltfMdl.accessors.at(primitive.indices);
			auto &idxBufView = gltfMdl.bufferViews.at(idxAccessor.bufferView);
			auto &idxBuf = gltfMdl.buffers.at(idxBufView.buffer);
			
			auto *srcIndexData = idxBuf.data.data() +idxBufView.byteOffset +idxAccessor.byteOffset;

			auto subMesh = c_game->CreateModelSubMesh();
			subMesh->SetSkinTextureIndex(primitive.material);
			auto &indices = subMesh->GetTriangles();
			auto numIndices = idxAccessor.count;
			indices.resize(numIndices);
			switch(idxAccessor.componentType)
			{
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
			{
				memcpy(indices.data(),srcIndexData,indices.size() *sizeof(indices.front()));
				break;
			}
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
			{
				for(auto i=decltype(numIndices){0u};i<numIndices;++i)
					indices.at(i) = srcIndexData[i];
				break;
			}
			case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
			{
				auto *uiData = reinterpret_cast<const uint32_t*>(srcIndexData);
				for(auto i=decltype(numIndices){0u};i<numIndices;++i)
					indices.at(i) = uiData[i]; // TODO: If index is out of range (uint16), split the mesh into sub-meshes!
				break;
			}
			}

			auto fGetVertexBufferData = [&primitive,&gltfMdl,&fGetBufferData](const std::string &identifier) -> std::optional<GLTFBufferData> {
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
			while(fGetVertexBufferData("JOINTS_" +std::to_string(iWeightChannel++)).has_value())
				Con::cwar<<"WARNING: Model has more than 4 bone weights, this is not supported!"<<Con::endl;

			auto &verts = subMesh->GetVertices();
			auto numVerts = posBufData->accessor.count;
			verts.resize(numVerts);

			std::vector<Vector2> *lightmapUvs = nullptr;
			if(texCoordBufData1.has_value())
			{
				lightmapUvs = &subMesh->AddUVSet("lightmap");
				lightmapUvs->resize(numVerts);
			}

			for(auto i=decltype(numVerts){0u};i<numVerts;++i)
			{
				auto &v = verts.at(i);
				v.position = TransformPos(posBufData->GetIndexedValue<Vector3>(i));
				if(normBufData.has_value())
					v.normal = normBufData->GetIndexedValue<Vector3>(i);
				if(texCoordBufData.has_value())
					v.uv = texCoordBufData->GetIndexedValue<Vector2>(i);
				if(lightmapUvs)
					(*lightmapUvs).at(i) = texCoordBufData1->GetIndexedValue<Vector2>(i);
			}

			if(jointsBufData.has_value() && weightsBufData.has_value())
			{
				auto &vertWeights = subMesh->GetVertexWeights();
				vertWeights.resize(numVerts);

				for(auto i=decltype(numVerts){0u};i<numVerts;++i)
				{
					auto &vw = vertWeights.at(i);

					auto weights = weightsBufData->GetFloatArray<4>(i);
					auto boneIds = jointsBufData->GetIntArray<4>(i);
					for(uint8_t j=0;j<4;++j)
					{
						vw.weights[j] = weights[j];
						vw.boneIds[j] = boneIds[j];
					}
				}
				// JOINTS_1  -> +4
			}

			for(auto i=decltype(idxAccessor.count){0u};i<idxAccessor.count;++i)
			{

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
			for(auto &target : primitive.targets)
			{
				util::ScopeGuard sg {[&targetIdx]() {++targetIdx;}};
				auto itPos = target.find("POSITION");
				auto itNormal = target.find("NORMAL");
				// auto itTangent = target.find("TANGENT");
				if(itPos == target.end())
					continue;
				auto &posAccessor = gltfMdl.accessors.at(itPos->second);
				auto &posBufView = gltfMdl.bufferViews.at(posAccessor.bufferView);
				auto &posBuf = gltfMdl.buffers.at(posBufView.buffer);
				auto posBufData = GLTFBufferData{posAccessor,posBufView,posBuf};

				std::unique_ptr<GLTFBufferData> normBufData {};
				if(itNormal != target.end())
				{
					auto &normAccessor = gltfMdl.accessors.at(itNormal->second);
					auto &normBufView = gltfMdl.bufferViews.at(normAccessor.bufferView);
					auto &normBuf = gltfMdl.buffers.at(normBufView.buffer);
					normBufData = std::unique_ptr<GLTFBufferData>{new GLTFBufferData{normAccessor,normBufView,normBuf}};
				}

				std::string morphTargetName;

				if(gltfMesh.extras.Has("targetNames"))
					morphTargetName = gltfMesh.extras.Get("targetNames").Get(targetIdx).Get<std::string>();
				else
					morphTargetName = std::to_string(absUnnamedFcIdx +targetIdx);

				if(mdl->GetFlexController(morphTargetName) == nullptr)
				{
					auto defaultWeight = (targetIdx < gltfMesh.weights.size()) ? gltfMesh.weights.at(targetIdx) : 0.f;
					auto &fc = mdl->AddFlexController(morphTargetName);
					fc.min = 0.f;
					fc.max = 1.f;
					// TODO: Apply default
				}
				uint32_t fcId = 0;
				mdl->GetFlexControllerId(morphTargetName,fcId);

				if(mdl->GetFlex(morphTargetName) == false)
				{
					auto &flex = mdl->AddFlex(morphTargetName);
					auto va = mdl->AddVertexAnimation(morphTargetName);
					flex.SetVertexAnimation(*va);

					auto &operations = flex.GetOperations();
					operations.push_back({});
					auto &op = flex.GetOperations().back();
					op.type = Flex::Operation::Type::Fetch;
					op.d.index = fcId;
				}
				uint32_t flexId;
				mdl->GetFlexId(morphTargetName,flexId);
				auto &va = *mdl->GetFlex(flexId)->GetVertexAnimation();

				assert(posAccessor.count == numVerts);
				auto mva = va.AddMeshFrame(*mesh,*subMesh);
				mva->SetVertexCount(numVerts);
				if(normBufData)
					mva->SetFlagEnabled(MeshVertexFrame::Flags::HasNormals);
				for(auto i=decltype(posAccessor.count){0u};i<posAccessor.count;++i)
				{
					auto pos = TransformPos(posBufData.GetIndexedValue<Vector3>(i));
					mva->SetVertexPosition(i,pos);
					if(normBufData)
					{
						auto n = normBufData->GetIndexedValue<Vector3>(i);
						mva->SetVertexNormal(i,n);
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
	}

	std::unordered_map<tinygltf::Node*,uint32_t> nodeToBoneIndex;
	if(gltfMdl.skins.empty() == false)
	{
		auto &skeleton = mdl->GetSkeleton();
		auto &skin = gltfMdl.skins.front();
		skeleton.GetBones().reserve(skin.joints.size());
		std::unordered_map<int,Bone*> nodeIdxToBone;
		for(auto i=decltype(skin.joints.size()){0u};i<skin.joints.size();++i)
		{
			auto nodeIdx = skin.joints[i];
			auto &node = gltfMdl.nodes[nodeIdx];
			nodeToBoneIndex[&node] = i;

			auto *bone = new Bone{};
			bone->name = node.name;
			skeleton.AddBone(bone);
			nodeIdxToBone[nodeIdx] = bone;
		}

		auto bufferData = fGetBufferData(skin.inverseBindMatrices);

		// Build hierarchy
		std::vector<umath::ScaledTransform> bindPoses {};
		bindPoses.reserve(skin.joints.size());
		for(auto i=decltype(skin.joints.size()){0u};i<skin.joints.size();++i)
		{
			auto nodeIdx = skin.joints[i];
			auto &node = gltfMdl.nodes[nodeIdx];
			auto &bone = *nodeIdxToBone[nodeIdx];
			for(auto childIdx : node.children)
			{
				auto it = nodeIdxToBone.find(childIdx);
				if(it == nodeIdxToBone.end())
					continue; // Not a bone?
				auto &child = *it->second;
				bone.children.insert(std::make_pair(child.ID,child.shared_from_this()));
				child.parent = bone.shared_from_this();
			}

			auto invBindMatrix = bufferData.GetIndexedValue<Mat4>(i);
			umath::ScaledTransform bindPose {invBindMatrix};
			bindPose = bindPose.GetInverse();
			bindPose.SetOrigin(TransformPos(bindPose.GetOrigin()));
			bindPoses.push_back(bindPose);
		}

		// Build reference pose
		auto &reference = mdl->GetReference();
		reference.SetBoneCount(skin.joints.size());
		for(auto i=decltype(skin.joints.size()){0u};i<skin.joints.size();++i)
		{
			auto nodeIdx = skin.joints[i];
			auto &node = gltfMdl.nodes[nodeIdx];
			umath::ScaledTransform pose {};
			if(!node.translation.empty())
				pose.SetOrigin(TransformPos(Vector3{static_cast<float>(node.translation[0]),static_cast<float>(node.translation[1]),static_cast<float>(node.translation[2])}));
			if(!node.rotation.empty())
				pose.SetRotation(Quat{static_cast<float>(node.rotation[3]),static_cast<float>(node.rotation[0]),static_cast<float>(node.rotation[1]),static_cast<float>(node.rotation[2])});
			if(!node.scale.empty())
				pose.SetScale(Vector3{static_cast<float>(node.scale[0]),static_cast<float>(node.scale[1]),static_cast<float>(node.scale[2])});
			reference.SetBonePose(i,pose);
		}

		for(auto &bone : skeleton.GetBones())
		{
			if(bone->parent.expired() == false)
				continue;
			skeleton.GetRootBones().insert(std::make_pair(bone->ID,bone));
		}
		
		auto refAnim = Animation::Create();
		refAnim->ReserveBoneIds(skin.joints.size());
		for(auto i=decltype(skin.joints.size()){0u};i<skin.joints.size();++i)
			refAnim->AddBoneId(i);

		auto frame = Frame::Create(reference);
		refAnim->AddFrame(frame);
		reference.Globalize(skeleton);
		mdl->AddAnimation("reference",refAnim);
	}

	auto &skeleton = mdl->GetSkeleton();
	auto numBones = skeleton.GetBoneCount();
	for(auto &gltfAnim : gltfMdl.animations)
	{
		auto &animName = gltfAnim.name;
		auto anim = Animation::Create();
		float fps = 24.f;
		auto fGetFrame = [&](uint32_t frameIndex) -> Frame& {
			auto &frames = anim->GetFrames();
			if(frameIndex < frames.size())
				return *frames[frameIndex];
			frames.reserve(frameIndex +1);
			for(auto i=frames.size();i<frameIndex +1;++i)
			{
				auto frame = Frame::Create(numBones);
				frames.push_back(frame);
			}
			return *frames.back();
		};
		for(auto &channel : gltfAnim.channels)
		{
			auto &sampler = gltfAnim.samplers[channel.sampler];
			auto &node = gltfMdl.nodes[channel.target_node];

			auto it = nodeToBoneIndex.find(&node);
			if(it == nodeToBoneIndex.end())
				continue; // Not a bone
			auto boneId = it->second;

			auto bufTimes = fGetBufferData(sampler.input);
			auto bufValues = fGetBufferData(sampler.output);

			enum class Channel : uint8_t
			{
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
			struct Value
			{
				Value(const Vector3 &translation)
					: translation{translation}
				{}
				Value(const Quat &rotation)
					: rotation{rotation}
				{}
				Value(float weight)
					: weight{weight}
				{}
				Value() {}
				Value(const Value &other)
				{
					memcpy(this,&other,sizeof(*this));
				}
				union
				{
					Vector3 translation;
					Quat rotation;
					Vector3 scale;
					float weight;
				};
			};
			std::vector<Value> values;
			times.reserve(n);
			values.reserve(n);
			for(auto i=decltype(n){0u};i<n;++i)
			{
				auto t = bufTimes.GetIndexedValue<float>(i);
				times.push_back(t);
				switch(eChannel)
				{
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

			auto fGetInterpolatedValue = [&times,&values,eChannel](float tTgt) -> Value {
				auto it = std::find_if(times.begin(),times.end(),[tTgt](float t) {
					return t >= tTgt;
				});
				auto itNext = it;
				if(it == times.end())
				{
					--it;
					itNext = it;
				}
				else
				{
					++itNext;
					if(itNext == times.end())
						itNext = it;
				}

				auto t0 = *it;
				auto t1 = *itNext;
				auto interp = (t1 > t0) ? umath::clamp((tTgt -t0) /(t1 -t0),0.f,1.f) : 0.f;
				auto &v0 = values[it -times.begin()];
				auto &v1 = values[itNext -times.begin()];
				Value interpValue {};
				switch(eChannel)
				{
				case Channel::Translation:
				{
					interpValue.translation = uvec::lerp(v0.translation,v1.translation,interp);
					break;
				}
				case Channel::Rotation:
				{
					interpValue.rotation = uquat::slerp(v0.rotation,v1.rotation,interp);
					break;
				}
				case Channel::Scale:
				{
					interpValue.scale = uvec::lerp(v0.scale,v1.scale,interp);
					break;
				}
				case Channel::Weights:
				{
					interpValue.weight = umath::lerp(v0.weight,v1.weight,interp);
					break;
				}
				}
				return interpValue;
			};

			auto numFrames = umath::max(umath::ceil(times.back() *24),1);
			for(auto i=decltype(numFrames){0u};i<numFrames;++i)
			{
				auto &frame = fGetFrame(i);
				auto t = static_cast<float>(i) /fps;
				switch(eChannel)
				{
				case Channel::Translation:
				{
					frame.SetBonePosition(boneId,fGetInterpolatedValue(t).translation);
					break;
				}
				case Channel::Rotation:
				{
					frame.SetBoneOrientation(boneId,fGetInterpolatedValue(t).rotation);
					break;
				}
				case Channel::Scale:
				{
					frame.SetBoneScale(boneId,fGetInterpolatedValue(t).scale);
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
		for(auto i=decltype(numBones){0u};i<numBones;++i)
			anim->AddBoneId(i);
		mdl->AddAnimation(animName,anim);
	}

	if(numBones > umath::to_integral(GameLimits::MaxBones))
		Con::cwar<<"Model has "<<numBones<<", but engine only supports "<<umath::to_integral(GameLimits::MaxBones)<<", this may cause rendering glitches!"<<Con::endl;
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
							Con::cwar<<"Bone weight id "<<id<<" out of range ("<<numBones<<")!"<<Con::endl;
					}
				}
			}
		}
	}
#endif

	mdl->Update(ModelUpdateFlags::All);
	mdl->SaveLegacy(c_game,outputPath.GetString() +mdlName,"addons/converted/");
	return mdl;
}
std::shared_ptr<Model> pragma::asset::import_model(VFilePtr f,std::string &outErrMsg,const util::Path &outputPath)
{
	return ::import_model(f,"",outErrMsg,outputPath);
}
std::shared_ptr<Model> pragma::asset::import_model(const std::string &fileName,std::string &outErrMsg,const util::Path &outputPath)
{
	return ::import_model(nullptr,fileName,outErrMsg,outputPath);
}

bool pragma::asset::import_texture(const std::string &fileName,const TextureImportInfo &texInfo,const std::string &outputPath,std::string &outErrMsg)
{
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly | TextureLoadFlags::DontCache;
	std::shared_ptr<void> tex;
	if(static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(c_engine->GetRenderContext(),fileName,loadInfo,&tex) == false)
	{
		outErrMsg = "Unable to load texture!";
		return false;
	}
	if(std::static_pointer_cast<Texture>(tex)->HasValidVkTexture() == false)
	{
		outErrMsg = "Invalid texture!";
		return false;
	}
	return import_texture(std::static_pointer_cast<Texture>(tex)->GetVkTexture()->GetImage(),texInfo,outputPath,outErrMsg);
}
bool pragma::asset::import_texture(VFilePtr f,const TextureImportInfo &texInfo,const std::string &outputPath,std::string &outErrMsg)
{
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly | TextureLoadFlags::DontCache;
	std::shared_ptr<void> tex;
	if(static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager().Load(c_engine->GetRenderContext(),"",f,loadInfo,&tex) == false)
	{
		outErrMsg = "Unable to load texture!";
		return false;
	}
	if(std::static_pointer_cast<Texture>(tex)->HasValidVkTexture() == false)
	{
		outErrMsg = "Invalid texture!";
		return false;
	}
	return import_texture(std::static_pointer_cast<Texture>(tex)->GetVkTexture()->GetImage(),texInfo,outputPath,outErrMsg);
}
bool pragma::asset::import_texture(prosper::IImage &img,const TextureImportInfo &texInfo,const std::string &outputPath,std::string &outErrMsg)
{
	std::string ext;
	auto texWriteInfo = get_texture_write_info(pragma::asset::ModelExportInfo::ImageFormat::DDS,texInfo.normalMap,texInfo.srgb,uimg::TextureInfo::AlphaMode::Auto,ext);
	if(texInfo.greyScaleMap)
		texWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::GradientMap;
	else // TODO: Use BC1 if no alpha!
		texWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::BC3;

	if(texWriteInfo.containerFormat == uimg::TextureInfo::ContainerFormat::DDS)
	{
		auto anvFormat = img.GetFormat();
		switch(anvFormat)
		{
			// These formats require DDS10, which is not well supported, so we'll fall back to
			// a different compression format
		case prosper::Format::BC6H_SFloat_Block:
		case prosper::Format::BC6H_UFloat_Block:
		case prosper::Format::BC7_SRGB_Block:
		case prosper::Format::BC7_UNorm_Block:
			texWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::BC3;
			break;
		}
	}
	auto imgOutputPath = "materials/" +util::Path{outputPath};
	imgOutputPath.RemoveFileExtension();
	return c_game->SaveImage(img,imgOutputPath.GetString(),texWriteInfo);
}

bool pragma::asset::export_map(const std::string &mapName,const ModelExportInfo &exportInfo,std::string &outErrMsg,const std::optional<MapExportInfo> &mapExp)
{
	::util::Path mapPath {mapName};
	mapPath.RemoveFileExtension();
	mapPath = "maps/" +mapPath +".wld";
	auto f = FileManager::OpenFile(mapPath.GetString().c_str(),"rb");
	if(f == nullptr)
	{
		if(util::port_source2_map(client,mapPath.GetString()) || util::port_hl2_map(client,mapPath.GetString()))
		{
			f = FileManager::OpenFile(mapPath.GetString().c_str(),"rb");
			if(f == nullptr)
			{
				// Sleep for a bit, then try again, in case the file hasn't been fully written yet
				std::this_thread::sleep_for(std::chrono::seconds{1});
				f = FileManager::OpenFile(mapPath.GetString().c_str(),"rb");
			}
		}
	}
	if(f == nullptr)
	{
		outErrMsg = "Unable to open map file '" +mapPath.GetString() +"'!";
		return false;
	}
	if(exportInfo.verbose)
		Con::cout<<"Loading map data..."<<Con::endl;
	auto worldData = pragma::asset::WorldData::Create(*client);
	if(worldData->Read(f,pragma::asset::EntityData::Flags::None,&outErrMsg) == false)
		return false;
	f = nullptr;

	if(exportInfo.verbose)
		Con::cout<<"Collecting world node models..."<<Con::endl;

	pragma::asset::GLTFWriter::SceneDesc sceneDesc {};
	if(mapExp.has_value())
	{
		auto &cameras = mapExp->GetCameras();
		sceneDesc.cameras.reserve(cameras.size());
		for(auto &hCamC : cameras)
		{
			if(hCamC.expired())
				continue;
			auto &camC = *hCamC;
			auto &ent = camC.GetEntity();
			sceneDesc.cameras.push_back({});

			auto &camScene = sceneDesc.cameras.back();
			camScene.name = ent.GetName();
			camScene.type = pragma::asset::GLTFWriter::Camera::Type::Perspective;
			camScene.aspectRatio = camC.GetAspectRatio();
			camScene.vFov = camC.GetFOV();
			camScene.zNear = camC.GetNearZ();
			camScene.zFar = camC.GetFarZ();
			camScene.pose = ent.GetPose();
		}

		auto &lightSources = mapExp->GetLightSources();
		sceneDesc.lightSources.reserve(lightSources.size());
		for(auto &hLightC : lightSources)
		{
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
				lightScene.range = util::pragma::units_to_metres(radiusC->GetRadius());
			
			auto spotC = ent.GetComponent<CLightSpotComponent>();
			if(spotC.valid())
			{
				lightScene.type = pragma::asset::GLTFWriter::LightSource::Type::Spot;
				lightScene.innerConeAngle = spotC->GetInnerCutoffAngle();
				lightScene.outerConeAngle = spotC->GetOuterCutoffAngle();
			}

			auto pointC = ent.GetComponent<CLightPointComponent>();
			if(pointC.valid())
				lightScene.type = pragma::asset::GLTFWriter::LightSource::Type::Point;

			auto dirC = ent.GetComponent<CLightDirectionalComponent>();
			if(dirC.valid())
				lightScene.type = pragma::asset::GLTFWriter::LightSource::Type::Directional;

			switch(lightScene.type)
			{
			case pragma::asset::GLTFWriter::LightSource::Type::Spot:
			case pragma::asset::GLTFWriter::LightSource::Type::Point:
				// TODO: This should be Candela, not Lumen, but Lumen produces closer results for Blender
				lightScene.luminousIntensity = lightC.GetLightIntensityLumen();
				break;
			case pragma::asset::GLTFWriter::LightSource::Type::Directional:
				lightScene.illuminance = lightC.GetLightIntensity();
				break;
			}
		}
	}

	for(auto &ent : worldData->GetEntities())
	{
		if(ent->IsWorld() || ent->GetClassName() == "prop_static" || ent->GetClassName() == "prop_physics" || ent->GetClassName() == "prop_dynamic")
		{
			auto strMdl = ent->GetKeyValue("model");
			if(strMdl.has_value() == false)
				continue;
			if(exportInfo.verbose)
				Con::cout<<"Loading world node model '"<<*strMdl<<"'..."<<Con::endl;
			auto mdl = c_game->LoadModel(*strMdl);
			if(mdl == nullptr)
			{
				Con::cwar<<"WARNING: Unable to load model '"<<*strMdl<<"'! Model will not be included in level export!"<<Con::endl;
				continue;
			}
			if(sceneDesc.modelCollection.size() == sceneDesc.modelCollection.capacity())
				sceneDesc.modelCollection.reserve(sceneDesc.modelCollection.size() *1.5 +100);
			sceneDesc.modelCollection.push_back({*mdl});
			auto &mdlDesc = sceneDesc.modelCollection.back();
			mdlDesc.pose = ent->GetPose();
		}
		else if(ent->GetClassName() == "env_light_spot" || ent->GetClassName() == "env_light_point" || ent->GetClassName() == "env_light_environment")
		{
			if(mapExp.has_value() && mapExp->includeMapLightSources == false)
				continue;
			sceneDesc.lightSources.push_back({});
			auto &ls = sceneDesc.lightSources.back();

			ls.name = ent->GetKeyValue("name",ent->GetClassName() +'_' +std::to_string(ent->GetMapIndex()));
			ls.pose = ent->GetPose();

			auto color = ent->GetKeyValue("color");
			if(color.has_value() == false)
				color = ent->GetKeyValue("lightcolor");
			if(color.has_value())
				ls.color = Color{*color};
			
			auto radius = ent->GetKeyValue("radius");
			if(radius.has_value() == false)
				radius = ent->GetKeyValue("distance");
			if(radius.has_value())
				ls.range = util::to_float(*radius);

			std::optional<float> outerCutoffAngle {};
			if(ent->GetClassName() == "env_light_spot")
			{
				ls.type = pragma::asset::GLTFWriter::LightSource::Type::Spot;

				auto innerCutoff = ent->GetKeyValue("innercutoff");
				if(innerCutoff.has_value())
					ls.innerConeAngle = util::to_float(*innerCutoff);

				auto outerCutoff = ent->GetKeyValue("outercutoff");
				if(outerCutoff.has_value())
				{
					ls.outerConeAngle = util::to_float(*outerCutoff);
					outerCutoffAngle = ls.outerConeAngle;
				}
			}
			else if(ent->GetClassName() == "env_light_point")
				ls.type = pragma::asset::GLTFWriter::LightSource::Type::Point;
			else if(ent->GetClassName() == "env_light_environment")
				ls.type = pragma::asset::GLTFWriter::LightSource::Type::Directional;			

			auto intensity = ent->GetKeyValue("light_intensity");
			if(intensity.has_value())
			{
				auto intensityType = pragma::BaseEnvLightComponent::LightIntensityType::Candela;
				auto vIntensityType = ent->GetKeyValue("light_intensity_type");
				if(vIntensityType.has_value())
					intensityType = static_cast<pragma::BaseEnvLightComponent::LightIntensityType>(util::to_int(*vIntensityType));

				auto flIntensity = util::to_float(*intensity);
				switch(ls.type)
				{
				case pragma::asset::GLTFWriter::LightSource::Type::Spot:
				case pragma::asset::GLTFWriter::LightSource::Type::Point:
					// TODO: This should be Candela, not Lumen, but Lumen produces closer results for Blender
					ls.luminousIntensity = BaseEnvLightComponent::GetLightIntensityLumen(flIntensity,intensityType,outerCutoffAngle);
					break;
				case pragma::asset::GLTFWriter::LightSource::Type::Directional:
					ls.illuminance = flIntensity;
					break;
				}
			}
		}
	}

	if(sceneDesc.modelCollection.empty())
	{
		outErrMsg = "No models to export found!";
		return false;
	}

	// HACK: If the model was just ported, we need to make sure the material and textures are in order by invoking the
	// resource watcher (in case they have been changed)
	// TODO: This doesn't belong here!
	client->GetResourceWatcher().Poll();

	if(exportInfo.verbose)
		Con::cout<<"Exporting scene with "<<sceneDesc.modelCollection.size()<<" models and "<<sceneDesc.lightSources.size()<<" light sources..."<<Con::endl;

	auto exportPath = "maps/" +mapName +'/';
	auto mdlName = exportPath +mapName;

	auto mapExportInfo = exportInfo;
	mapExportInfo.exportSkinnedMeshData = false;
	mapExportInfo.generateAo = false;
	auto success = GLTFWriter::Export(sceneDesc,mdlName,mapExportInfo,outErrMsg);
	if(success == false)
		return false;
	if(exportInfo.verbose)
		Con::cout<<"Exporting lightmap atlas..."<<Con::endl;
	success = export_texture("maps/" +mapName +"/lightmap_atlas",mapExportInfo.imageFormat,outErrMsg,uimg::TextureInfo::AlphaMode::None,true /* extended DDS */,&exportPath);
	if(success == false)
	{
		if(exportInfo.verbose)
			Con::cout<<"WARNING: Unable to export lightmap atlas: "<<outErrMsg<<Con::endl;
	}
	return true;
}

bool pragma::asset::export_model(::Model &mdl,const ModelExportInfo &exportInfo,std::string &outErrMsg,const std::optional<std::string> &modelName)
{
	return GLTFWriter::Export(mdl,exportInfo,outErrMsg,modelName);
}
bool pragma::asset::export_animation(Model &mdl,const std::string &animName,const ModelExportInfo &exportInfo,std::string &outErrMsg,const std::optional<std::string> &modelName)
{
	return GLTFWriter::Export(mdl,animName,exportInfo,outErrMsg,modelName);
}
bool pragma::asset::export_texture(
	uimg::ImageBuffer &imgBuf,ModelExportInfo::ImageFormat imageFormat,const std::string &outputPath,std::string &outErrMsg,
	bool normalMap,bool srgb,uimg::TextureInfo::AlphaMode alphaMode,std::string *optOutOutputPath
)
{
	std::string inOutPath = EXPORT_PATH +outputPath;
	auto success = save_image(imgBuf,imageFormat,inOutPath,normalMap,srgb,alphaMode);
	if(optOutOutputPath)
		*optOutOutputPath = inOutPath;
	return success;
}
bool pragma::asset::export_texture(
	const std::string &texturePath,ModelExportInfo::ImageFormat imageFormat,std::string &outErrMsg,
	uimg::TextureInfo::AlphaMode alphaMode,bool enableExtendedDDS,std::string *optExportPath,std::string *optOutOutputPath,
	const std::optional<std::string> &optFileNameOverride
)
{
	auto &texManager = static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager();
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly;
	std::shared_ptr<void> pTexture = nullptr;
	if(texManager.Load(c_engine->GetRenderContext(),texturePath,loadInfo,&pTexture) == false || std::static_pointer_cast<Texture>(pTexture)->HasValidVkTexture() == false)
	{
		outErrMsg = "Unable to load texture '" +texturePath +"'!";
		return false;
	}
	auto texture = std::static_pointer_cast<Texture>(pTexture);
	auto &vkImg = texture->GetVkTexture()->GetImage();

	auto imgPath = optFileNameOverride.has_value() ? *optFileNameOverride : ufile::get_file_from_filename(texturePath);
	ufile::remove_extension_from_filename(imgPath);
	if(optExportPath)
		imgPath = (util::Path::CreatePath(*optExportPath) +imgPath).GetString();
	auto imgOutputPath = EXPORT_PATH +imgPath;

	auto exportSuccess = false;
	if(imageFormat == ModelExportInfo::ImageFormat::DDS || imageFormat == ModelExportInfo::ImageFormat::KTX)
	{
		std::string ext;
		auto texWriteInfo = get_texture_write_info(imageFormat,texture->HasFlag(Texture::Flags::NormalMap),texture->HasFlag(Texture::Flags::SRGB),alphaMode,ext);
		imgOutputPath += '.' +ext;
		if(texWriteInfo.containerFormat == uimg::TextureInfo::ContainerFormat::DDS && enableExtendedDDS == false)
		{
			auto anvFormat = vkImg.GetFormat();
			switch(anvFormat)
			{
				// These formats require DDS10, which is not well supported, so we'll fall back to
				// a different compression format
			case prosper::Format::BC6H_SFloat_Block:
			case prosper::Format::BC6H_UFloat_Block:
			case prosper::Format::BC7_SRGB_Block:
			case prosper::Format::BC7_UNorm_Block:
				texWriteInfo.outputFormat = uimg::TextureInfo::OutputFormat::BC3;
				break;
			}
		}
		exportSuccess = c_game->SaveImage(vkImg,imgOutputPath,texWriteInfo);
	}
	else
	{
		std::vector<std::vector<std::shared_ptr<uimg::ImageBuffer>>> imgBuffers;
		if(::util::to_image_buffer(vkImg,imgBuffers) == false)
		{
			outErrMsg = "Unable to convert texture '" +texturePath +"' to image buffer!";
			return false;
		}
		auto &imgBuf = imgBuffers.front().front();
		exportSuccess = save_image(*imgBuf,imageFormat,imgOutputPath,texture->HasFlag(Texture::Flags::NormalMap),texture->HasFlag(Texture::Flags::SRGB),alphaMode);
	}
	if(exportSuccess == false)
	{
		outErrMsg = "Unable to export texture '" +texturePath +"'!";
		return false;
	}
	if(optOutOutputPath)
		*optOutOutputPath = imgOutputPath;
	return true;
}
std::optional<pragma::asset::MaterialTexturePaths> pragma::asset::export_material(
	Material &mat,ModelExportInfo::ImageFormat imageFormat,std::string &outErrMsg,std::string *optExportPath,
	bool normalizeTextureNames
)
{
	auto name = ufile::get_file_from_filename(mat.GetName());
	ufile::remove_extension_from_filename(name);

	auto exportPath = optExportPath ? *optExportPath : name;

	auto fSaveTexture = [imageFormat,&exportPath,normalizeTextureNames]
	(TextureInfo *texInfo,bool normalMap,bool alpha,std::string &imgOutputPath,const std::string &normalizedName) -> bool {
		if(texInfo == nullptr)
			return false;
		std::string errMsg;
		return export_texture(
			texInfo->name,imageFormat,errMsg,alpha ? uimg::TextureInfo::AlphaMode::Transparency : uimg::TextureInfo::AlphaMode::None,false,
			&exportPath,&imgOutputPath,normalizeTextureNames ? normalizedName : std::optional<std::string>{}
		);
	};

	auto &data = mat.GetDataBlock();
	auto alphaMode = mat.GetAlphaMode();
	
	pragma::asset::MaterialTexturePaths texturePaths {};

	std::string imgOutputPath;
	if(fSaveTexture(mat.GetAlbedoMap(),false,alphaMode != AlphaMode::Opaque,imgOutputPath,name))
		texturePaths.insert(std::make_pair(Material::ALBEDO_MAP_IDENTIFIER,imgOutputPath));
	if(fSaveTexture(mat.GetNormalMap(),true,false,imgOutputPath,name +"_normal"))
		texturePaths.insert(std::make_pair(Material::NORMAL_MAP_IDENTIFIER,imgOutputPath));
	if(fSaveTexture(mat.GetRMAMap(),true,false,imgOutputPath,name +"_rma"))
		texturePaths.insert(std::make_pair(Material::RMA_MAP_IDENTIFIER,imgOutputPath));

	if(fSaveTexture(mat.GetGlowMap(),false,false,imgOutputPath,name +"_emission"))
		texturePaths.insert(std::make_pair(Material::EMISSION_MAP_IDENTIFIER,imgOutputPath));
	return texturePaths;
}

class ModelAOWorker
	: public util::ParallelWorker<pragma::asset::ModelAOWorkerResult>
{
public:
	ModelAOWorker(const std::vector<util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>>> &matAoJobs)
		: m_matAoJobs{matAoJobs}
	{
		AddThread([this]() {
			auto numJobs = m_matAoJobs.size();
			auto lastProgress = -1.f;
			auto numSuccessful = 0;
			while(m_matAoJobs.empty() == false)
			{
				if(IsCancelled() == false)
				{
					auto &t = m_matAoJobs.front();
					t.Start();
					while(t.IsComplete() == false)
					{
						t.Poll();
						auto jobProgress = t.GetProgress();
						auto totalProgress = jobProgress;
						totalProgress /= static_cast<float>(numJobs);
						if(totalProgress != lastProgress)
						{
							lastProgress = totalProgress;
							UpdateProgress(totalProgress);
						}
						if(jobProgress < 1.f)
							std::this_thread::sleep_for(std::chrono::seconds(1));
						if(IsCancelled())
							t.Cancel();
					}
					t.Poll(); // We need to poll one more time after completion to make sure completion handler is called
					if(t.IsSuccessful())
						++numSuccessful; // Technically the completion handler could still fail, but we'll just ignore that case for now
				}
				m_matAoJobs.erase(m_matAoJobs.begin());
			}
			SetStatus((numSuccessful > 0) ? util::JobStatus::Successful : util::JobStatus::Failed);
		});
	}
	virtual pragma::asset::ModelAOWorkerResult GetResult() override {return m_result;}
private:
	template<typename TJob,typename... TARGS>
		friend util::ParallelJob<typename TJob::RESULT_TYPE> util::create_parallel_job(TARGS&& ...args);
	std::vector<util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>>> m_matAoJobs {};
	pragma::asset::ModelAOWorkerResult m_result {};
};
std::optional<util::ParallelJob<pragma::asset::ModelAOWorkerResult>> pragma::asset::generate_ambient_occlusion(
	Model &mdl,std::string &outErrMsg,bool forceRebuild,uint32_t aoResolution,
	uint32_t aoSamples,pragma::rendering::cycles::SceneInfo::DeviceType aoDevice
)
{
	std::vector<util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>>> aoJobs {};
	std::unordered_set<std::string> builtRMAs {};
	auto &materials = mdl.GetMaterials();
	aoJobs.reserve(materials.size());
	for(auto &mat : materials)
	{
		if(mat.IsValid() == false)
			continue;
		auto *rmaMap = mat->GetRMAMap();
		if(rmaMap == nullptr)
			continue;
		if(builtRMAs.find(rmaMap->name) != builtRMAs.end())
			continue; // AO has already been built (maybe by a different skin material)
		builtRMAs.insert(rmaMap->name);
		util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> aoJob {};
		auto eResult = generate_ambient_occlusion(mdl,*mat.get(),aoJob,outErrMsg,forceRebuild,aoResolution,aoSamples,aoDevice);
		if(eResult != AOResult::AOJobReady)
			continue;
		aoJobs.push_back(aoJob);
	}
	if(aoJobs.empty())
		return {};
	return util::create_parallel_job<ModelAOWorker>(aoJobs);
}

template<class T>
	static bool save_ambient_occlusion(Material &mat,std::string rmaPath,T &img,std::string &errMsg)
{
	auto *shaderComposeRMA = static_cast<pragma::ShaderComposeRMA*>(c_engine->GetShader("compose_rma").get());
	if(shaderComposeRMA == nullptr)
		return false;
	ufile::remove_extension_from_filename(rmaPath);

	auto outPath = util::Path{rmaPath};
	auto originalRmaPath = outPath.GetString();
	auto requiresSave = false;
	if(outPath.GetFront() == "pbr")
	{
		// We don't want to overwrite the default pbr materials, so we'll use a different rma path
		outPath = mat.GetName();
		outPath.RemoveFileExtension();
		rmaPath = outPath.GetString() +"_rma";
		mat.GetDataBlock()->AddValue("texture",Material::RMA_MAP_IDENTIFIER,rmaPath);
		requiresSave = true;
	}

	if(shaderComposeRMA->InsertAmbientOcclusion(c_engine->GetRenderContext(),originalRmaPath,img,&rmaPath) == false)
	{
		errMsg = "Unable to insert ambient occlusion data into RMA map!";
		return false;
	}

	auto &dataBlock = mat.GetDataBlock();
	auto rmaInfo = dataBlock->GetBlock("rma_info");
	if(rmaInfo)
	{
		rmaInfo->RemoveValue("requires_ao_update");
		if(rmaInfo->IsEmpty())
			dataBlock->RemoveValue("rma_info");
	}
	dataBlock->RemoveValue("ao_map");

	if(requiresSave)
	{
		mat.UpdateTextures();
		mat.Save();
	}
	return true;
}

pragma::asset::AOResult pragma::asset::generate_ambient_occlusion(
	Model &mdl,Material &mat,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &outJob,std::string &outErrMsg,bool forceRebuild,uint32_t aoResolution,
	uint32_t aoSamples,pragma::rendering::cycles::SceneInfo::DeviceType aoDevice
)
{
	// TODO: There really is no good way to determine whether the material has a ambient occlusion map or not.
	// Use a compute shader to determine if it's all white or black?
	// On the other hand a rma texture should be unique to a model, so does it really matter?

	auto rmaInfo = mat.GetDataBlock()->GetBlock("rma_info");
	if(forceRebuild == false && (rmaInfo == nullptr || rmaInfo->GetBool("requires_ao_update") == false))
		return AOResult::NoAOGenerationRequired;
	auto *rmaTexInfo = mat.GetRMAMap();
	if(rmaTexInfo == nullptr || std::static_pointer_cast<Texture>(rmaTexInfo->texture) == nullptr)
	{
		outErrMsg = "Material has no RMA texture assigned!";
		return AOResult::FailedToCreateAOJob; // This is not a valid pbr material?
	}
	auto rmaTex = std::static_pointer_cast<Texture>(rmaTexInfo->texture);
	auto rmaPath = rmaTex->GetName();
	auto *shaderComposeRMA = static_cast<pragma::ShaderComposeRMA*>(c_engine->GetShader("compose_rma").get());
	if(shaderComposeRMA == nullptr)
	{
		outErrMsg = "Unable to load RMA shader!";
		return AOResult::FailedToCreateAOJob;
	}
	auto &materials = mdl.GetMaterials();
	auto it = std::find_if(materials.begin(),materials.end(),[&mat](const MaterialHandle &hMat) {
		return &mat == hMat.get();
	});
	if(it == materials.end())
	{
		outErrMsg = "Material not found in model!";
		return AOResult::FailedToCreateAOJob;
	}
	auto matIdx = (it -materials.begin());

	pragma::rendering::cycles::SceneInfo sceneInfo {};
	sceneInfo.denoise = true;
	sceneInfo.hdrOutput = false;
	sceneInfo.width = aoResolution;
	sceneInfo.height = aoResolution;
	sceneInfo.samples = aoSamples;
	sceneInfo.device = aoDevice;

	//std::shared_ptr<uimg::ImageBuffer> aoImg = nullptr;
	outJob = pragma::rendering::cycles::bake_ambient_occlusion(*client,sceneInfo,mdl,matIdx);
	if(outJob.IsValid() == false)
	{
		outErrMsg = "Unable to create job for ao generation!";
		return AOResult::FailedToCreateAOJob;
	}

	auto *texInfoAo = mat.GetTextureInfo("ao_map");
	if(texInfoAo && texInfoAo->texture && std::static_pointer_cast<Texture>(texInfoAo->texture)->HasValidVkTexture())
	{
		// Material already has a separate ambient occlusion map, just use that one
		auto &img = std::static_pointer_cast<Texture>(texInfoAo->texture)->GetVkTexture()->GetImage();
		std::string errMsg;
		auto result = save_ambient_occlusion<prosper::IImage>(mat,rmaPath,img,errMsg);
		if(result)
			return AOResult::NoAOGenerationRequired;
		// Failed; Just try to generate?
	}

	auto hMat = mat.GetHandle();
	outJob.SetCompletionHandler([rmaPath,hMat](::util::ParallelWorker<std::shared_ptr<uimg::ImageBuffer>> &worker) mutable {
		auto *shaderComposeRMA = static_cast<pragma::ShaderComposeRMA*>(c_engine->GetShader("compose_rma").get());
		if(worker.IsSuccessful() == false)
			return;
		if(hMat.IsValid() == false)
		{
			worker.SetStatus(util::JobStatus::Failed,"Material is not valid!");
			return;
		}
		if(shaderComposeRMA == nullptr)
		{
			worker.SetStatus(util::JobStatus::Failed,"Shader is not valid!");
			return;
		}
		auto aoImg = worker.GetResult();
		std::string errMsg;
		auto result = save_ambient_occlusion<uimg::ImageBuffer>(*hMat.get(),rmaPath,*aoImg,errMsg);
		if(result == false)
			worker.SetStatus(util::JobStatus::Failed,errMsg);
	});
	return AOResult::AOJobReady;
}
