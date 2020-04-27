/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
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
#include <pragma/model/animation/vertex_animation.hpp>
#include <sharedutils/util_file.h>
#include <sharedutils/util_path.hpp>
#include <sharedutils/util_parallel_job.hpp>
#include <pragma/asset_types/world.hpp>
#include <pragma/engine_version.h>
#include <image/prosper_sampler.hpp>
#include <util_image.hpp>
#include <cmaterialmanager.h>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT ClientState *client;
extern DLLCLIENT CGame *c_game;

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
	T GetValue(uint32_t vertexIndex,const T &default={}) const
	{
		auto stride = bufferView.byteStride;
		if(stride == 0)
			stride = sizeof(T);
		auto offset = bufferView.byteOffset +vertexIndex *stride;
		if(offset +sizeof(T) >= buffer.data.size())
			return default;
		return *reinterpret_cast<T*>(buffer.data.data() +offset);
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
	if(texManager.Load(*c_engine,image->uri,f,loadInfo,&texture) == false)
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
	auto scale = static_cast<float>(util::metres_to_units(1.f));

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

			auto img = c_engine->CreateImage(*imgBuf);
			prosper::util::TextureCreateInfo texCreateInfo {};
			prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
			prosper::util::SamplerCreateInfo samplerCreateInfo {};
			tex = c_engine->CreateTexture(texCreateInfo,*img,imgViewCreateInfo,samplerCreateInfo);
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
							*c_engine,diffuseTex,specularGlossinessTex,pushConstants,occlusionTex.get()
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
						shaderComposeRMA->InsertAmbientOcclusion(*c_engine,rmaName,*occlusionImg);
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
			dataBlock->AddValue("color","emission_factor",
				std::to_string(emissiveFactor.at(0) *255.f) +' ' +
				std::to_string(emissiveFactor.at(1) *255.f) +' ' +
				std::to_string(emissiveFactor.at(2) *255.f)
			);
		}

		mat->UpdateTextures();
		mat->Save(matPathRelative.GetString(),"addons/converted/");

		mdl->AddMaterial(0,mat);

		++matIdx;
	}

	auto &gltfMeshes = gltfMdl.meshes;
	auto meshGroup = mdl->AddMeshGroup("reference");
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

			auto *srcIndexData = idxBuf.data.data() +idxBufView.byteOffset;

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

			auto fGetBufferData = [&primitive,&gltfMdl](const std::string &identifier) -> std::optional<GLTFBufferData> {
				auto it = primitive.attributes.find(identifier);
				if(it == primitive.attributes.end())
					return {};
				auto &accessor = gltfMdl.accessors.at(it->second);
				auto &bufView = gltfMdl.bufferViews.at(accessor.bufferView);
				auto &buf = gltfMdl.buffers.at(bufView.buffer);
				return GLTFBufferData{accessor,bufView,buf};
			};
			auto &posBufData = fGetBufferData("POSITION");
			auto &normBufData = fGetBufferData("NORMAL");
			auto &texCoordBufData = fGetBufferData("TEXCOORD_0");
			auto &texCoordBufData1 = fGetBufferData("TEXCOORD_1");

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
				v.position = TransformPos(posBufData->GetValue<Vector3>(i));
				if(normBufData.has_value())
					v.normal = normBufData->GetValue<Vector3>(i);
				if(texCoordBufData.has_value())
					v.uv = texCoordBufData->GetValue<Vector2>(i);
				if(lightmapUvs)
					(*lightmapUvs).at(i) = texCoordBufData1->GetValue<Vector2>(i);
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


			for(auto i=decltype(idxAccessor.count){0u};i<idxAccessor.count;++i)
			{

				//idxAccessor.componentType == ;
				//idxAccessor.type
				//TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT,TINYGLTF_TYPE_SCALAR

				//	The indices data type. Valid values correspond to WebGL enums: 5121 (UNSIGNED_BYTE), 5123 (UNSIGNED_SHORT), 5125 (UNSIGNED_INT).
			}
			//idxBuf.
			//primitive.indices
			//primitive.mode
			mesh->AddSubMesh(subMesh);
		}
		meshGroup->AddMesh(mesh);
	}
	mdl->Update(ModelUpdateFlags::All);
	mdl->Save(c_game,outputPath.GetString() +mdlName,"addons/converted/");
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

bool pragma::asset::export_map(const std::string &mapName,const ModelExportInfo &exportInfo,std::string &outErrMsg)
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
	uimg::TextureInfo::AlphaMode alphaMode,bool enableExtendedDDS,std::string *optExportPath,std::string *optOutOutputPath
)
{
	auto &texManager = static_cast<CMaterialManager&>(client->GetMaterialManager()).GetTextureManager();
	TextureManager::LoadInfo loadInfo {};
	loadInfo.flags = TextureLoadFlags::LoadInstantly;
	std::shared_ptr<void> pTexture = nullptr;
	if(texManager.Load(*c_engine,texturePath,loadInfo,&pTexture) == false || std::static_pointer_cast<Texture>(pTexture)->HasValidVkTexture() == false)
	{
		outErrMsg = "Unable to load texture '" +texturePath +"'!";
		return false;
	}
	auto texture = std::static_pointer_cast<Texture>(pTexture);
	auto &vkImg = texture->GetVkTexture()->GetImage();

	auto imgPath = ufile::get_file_from_filename(texturePath);
	ufile::remove_extension_from_filename(imgPath);
	if(optExportPath)
		imgPath = util::Path{*optExportPath}.GetString() +imgPath;
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
	Material &mat,ModelExportInfo::ImageFormat imageFormat,std::string &outErrMsg,std::string *optExportPath
)
{
	auto name = ufile::get_file_from_filename(mat.GetName());
	ufile::remove_extension_from_filename(name);

	auto exportPath = optExportPath ? *optExportPath : name;

	auto fSaveTexture = [imageFormat,&exportPath]
	(TextureInfo *texInfo,bool normalMap,bool alpha,std::string &imgOutputPath) -> bool {
		if(texInfo == nullptr)
			return false;
		std::string errMsg;
		return export_texture(
			texInfo->name,imageFormat,errMsg,alpha ? uimg::TextureInfo::AlphaMode::Transparency : uimg::TextureInfo::AlphaMode::None,false,
			&exportPath,&imgOutputPath
		);
	};

	auto &data = mat.GetDataBlock();
	auto translucent = data->GetBool("translucent");
	
	pragma::asset::MaterialTexturePaths texturePaths {};

	std::string imgOutputPath;
	if(fSaveTexture(mat.GetAlbedoMap(),false,translucent,imgOutputPath))
		texturePaths.insert(std::make_pair(Material::ALBEDO_MAP_IDENTIFIER,imgOutputPath));
	if(fSaveTexture(mat.GetNormalMap(),true,false,imgOutputPath))
		texturePaths.insert(std::make_pair(Material::NORMAL_MAP_IDENTIFIER,imgOutputPath));
	if(fSaveTexture(mat.GetRMAMap(),true,false,imgOutputPath))
		texturePaths.insert(std::make_pair(Material::RMA_MAP_IDENTIFIER,imgOutputPath));

	if(fSaveTexture(mat.GetGlowMap(),false,false,imgOutputPath))
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

	if(shaderComposeRMA->InsertAmbientOcclusion(*c_engine,originalRmaPath,img,&rmaPath) == false)
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
