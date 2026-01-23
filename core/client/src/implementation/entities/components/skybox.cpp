// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/console/helper.hpp"

module pragma.client;

import :entities.components.skybox;
import :client_state;
import :engine;
import :game;
import :rendering.shaders;

using namespace pragma;

void CSkyboxComponent::Initialize()
{
	BaseSkyboxComponent::Initialize();
	auto &ent = static_cast<ecs::CBaseEntity &>(GetEntity());
	auto pRenderComponent = ent.GetComponent<CRenderComponent>();
	if(pRenderComponent.valid()) {
		pRenderComponent->SetSceneRenderPass(rendering::SceneRenderPass::Sky);
		pRenderComponent->SetCastShadows(false);
		pRenderComponent->SetDepthPassEnabled(false);
	}

	BindEventUnhandled(cModelComponent::EVENT_ON_MODEL_CHANGED, [this](std::reference_wrapper<ComponentEvent> evData) {
		if(m_cbOnModelMaterialsLoaded.IsValid())
			m_cbOnModelMaterialsLoaded.Remove();
		auto &evMdl = static_cast<CEOnModelChanged &>(evData.get());
		if(evMdl.model == nullptr)
			return;
		m_cbOnModelMaterialsLoaded = evMdl.model->CallOnMaterialsLoaded([this]() { ValidateMaterials(); });
	});
	SetSkyAngles({});
}
void CSkyboxComponent::ReceiveData(NetPacket &packet) { m_skyAngles = packet->Read<EulerAngles>(); }
Bool CSkyboxComponent::ReceiveNetEvent(NetEventId eventId, NetPacket &packet)
{
	if(eventId == m_netEvSetSkyAngles)
		SetSkyAngles(packet->Read<EulerAngles>());
	else
		return CBaseNetComponent::ReceiveNetEvent(eventId, packet);
	return true;
}
void CSkyboxComponent::OnRemove()
{
	BaseSkyboxComponent::OnRemove();
	if(m_cbOnModelMaterialsLoaded.IsValid())
		m_cbOnModelMaterialsLoaded.Remove();
}
void CSkyboxComponent::InitializeLuaObject(lua::State *l) { return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l); }
void CSkyboxComponent::SetSkyAngles(const EulerAngles &ang)
{
	m_skyAngles = ang;
	Vector3 axis;
	float angle;
	uquat::to_axis_angle(uquat::create(ang), axis, angle);
	m_renderSkyAngles = {axis, angle};
}
const EulerAngles &CSkyboxComponent::GetSkyAngles() const { return m_skyAngles; }
const Vector4 &CSkyboxComponent::GetRenderSkyAngles() const { return m_renderSkyAngles; }
bool CSkyboxComponent::CreateCubemapFromIndividualTextures(const std::string &materialPath, const std::string &postfix) const
{
	// Check if this skybox is made of individual textures
	// (e.g. if it came from the source engine)
	auto matName = materialPath;
	ufile::remove_extension_from_filename(matName, pragma::asset::get_supported_extensions(asset::Type::Material, asset::FormatType::All));

	auto containerFormat = image::TextureInfo::ContainerFormat::DDS;
	auto ext = "dds";
	if(fs::exists("materials/" + matName + "." + ext) && pragma::asset::exists(matName, asset::Type::Material))
		return true; // Skybox texture already exists; There's nothing for us to do

	// Load all sides as textures
	const std::array<std::string, 6> sidePostfixes {"ft", "bk", "up", "dn", "rt", "lf"};
	constexpr uint32_t numLayers = 6u;
	std::array<std::shared_ptr<prosper::IImage>, numLayers> cubemapImages {};
	auto bAllValid = true;
	uint32_t largestWidth = 0u;
	uint32_t largestHeight = 0u;
	for(auto i = decltype(sidePostfixes.size()) {0u}; i < sidePostfixes.size(); ++i) {
		auto &sidePostFix = sidePostfixes.at(i);
		auto *matSide = get_client_state()->LoadMaterial(matName + postfix + sidePostFix, nullptr, false, true);
		if(matSide == nullptr)
			return false;
		auto *diffuseMapSide = matSide->GetDiffuseMap();
		if(diffuseMapSide == nullptr || diffuseMapSide->texture == nullptr)
			return false;
		auto texture = std::static_pointer_cast<material::Texture>(diffuseMapSide->texture);
		if(texture->HasValidVkTexture() == false || texture->IsError())
			return false;
		auto &img = texture->GetVkTexture()->GetImage();
		cubemapImages.at(i) = img.shared_from_this();

		auto extents = img.GetExtents();
		largestWidth = math::max(largestWidth, extents.width);
		largestHeight = math::max(largestHeight, extents.height);
	}
	Con::COUT << "Found individual skybox textures for skybox '" << materialPath << "'! Generating cubemap texture..." << Con::endl;

	// Merge textures into cubemap image

	// Note: We need to transfer the individual images from their compressed format to RGBA8UNORM, as well as
	// from optimal tiling to linear tiling. This can't be done in one step, because some GPUs do not support
	// images with linear tiling and mipmaps, so we need to create a new image which will contain the RGBA8 data and
	// optimal tiling, and then copy that data into a host-readable buffer.
	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.flags = prosper::util::ImageCreateInfo::Flags::Cubemap | prosper::util::ImageCreateInfo::Flags::FullMipmapChain;
	imgCreateInfo.format = prosper::Format::R8G8B8A8_UNorm;
	imgCreateInfo.width = largestWidth;
	imgCreateInfo.height = largestHeight;
	imgCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::DeviceLocal;
	imgCreateInfo.postCreateLayout = prosper::ImageLayout::TransferDstOptimal;
	imgCreateInfo.tiling = prosper::ImageTiling::Optimal;
	imgCreateInfo.usage = prosper::ImageUsageFlags::TransferSrcBit;
	auto imgCubemap = get_cengine()->GetRenderContext().CreateImage(imgCreateInfo);
	auto numMipmaps = imgCubemap->GetMipmapCount();

	struct ImageBufferInfo {
		uint32_t layerIndex = 0;
		uint32_t mipmapIndex = 0;
		uint64_t bufferOffset = 0;
		uint64_t bufferSize = 0;

		uint32_t width = 0;
		uint32_t height = 0;
	};
	auto numBytesPerPixel = prosper::util::get_byte_size(imgCreateInfo.format);
	std::vector<ImageBufferInfo> imageBufferInfos {};
	imageBufferInfos.resize(numLayers * numMipmaps);
	uint32_t i = 0;
	uint64_t offset = 0;
	for(auto iLayer = decltype(numLayers) {0u}; iLayer < numLayers; ++iLayer) {
		for(auto iMipmap = decltype(numMipmaps) {0u}; iMipmap < numMipmaps; ++iMipmap) {
			uint32_t wMipmap, hMipmap;
			prosper::util::calculate_mipmap_size(largestWidth, largestHeight, &wMipmap, &hMipmap, iMipmap);
			auto &imgBufferInfo = imageBufferInfos.at(i++);
			imgBufferInfo.width = wMipmap;
			imgBufferInfo.height = hMipmap;
			imgBufferInfo.layerIndex = iLayer;
			imgBufferInfo.mipmapIndex = iMipmap;
			imgBufferInfo.bufferOffset = offset;
			imgBufferInfo.bufferSize = wMipmap * hMipmap * numBytesPerPixel;

			offset += imgBufferInfo.bufferSize;
		}
	}

	prosper::util::BufferCreateInfo bufCreateInfo {};
	bufCreateInfo.memoryFeatures = prosper::MemoryFeatureFlags::GPUToCPU;
	bufCreateInfo.size = offset;
	bufCreateInfo.usageFlags = prosper::BufferUsageFlags::TransferDstBit;
	auto buf = get_cengine()->GetRenderContext().CreateBuffer(bufCreateInfo);

	auto &setupCmd = get_cengine()->GetSetupCommandBuffer();
	for(auto &imgBufferInfo : imageBufferInfos) {
		auto &img = cubemapImages.at(imgBufferInfo.layerIndex);

		// Blit image into cubemap image
		prosper::util::ImageSubresourceRange range {};
		range.baseMipLevel = imgBufferInfo.mipmapIndex;
		setupCmd->RecordImageBarrier(*img, prosper::ImageLayout::ShaderReadOnlyOptimal, prosper::ImageLayout::TransferSrcOptimal, range);
		prosper::util::BlitInfo blitInfo {};
		blitInfo.dstSubresourceLayer.mipLevel = imgBufferInfo.mipmapIndex;
		setupCmd->RecordBlitImage(blitInfo, *img, *imgCubemap);
		setupCmd->RecordImageBarrier(*img, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::ShaderReadOnlyOptimal, range);

		// Copy image data to host-readable buffer
		setupCmd->RecordImageBarrier(*imgCubemap, prosper::ImageLayout::TransferDstOptimal, prosper::ImageLayout::TransferSrcOptimal, range);
		// Note: No buffer barrier required, since we're writing to non-intersecting sections of the buffer
		prosper::util::BufferImageCopyInfo copyInfo {};
		copyInfo.imageExtent = {imgBufferInfo.width, imgBufferInfo.height};
		copyInfo.mipLevel = imgBufferInfo.mipmapIndex;
		copyInfo.bufferOffset = imgBufferInfo.bufferOffset;
		setupCmd->RecordCopyImageToBuffer(copyInfo, *imgCubemap, prosper::ImageLayout::TransferSrcOptimal, *buf);
		setupCmd->RecordImageBarrier(*imgCubemap, prosper::ImageLayout::TransferSrcOptimal, prosper::ImageLayout::TransferDstOptimal, range);
	}
	get_cengine()->FlushSetupCommandBuffer();

	std::vector<std::vector<std::shared_ptr<image::ImageBuffer>>> cubemapBuffers {};
	cubemapBuffers.resize(numLayers);
	for(auto iLayer = decltype(numLayers) {0u}; iLayer < numLayers; ++iLayer) {
		auto &mipmapBuffers = cubemapBuffers.at(iLayer);
		mipmapBuffers.reserve(numMipmaps);
		for(auto iMipmap = decltype(numMipmaps) {0u}; iMipmap < numMipmaps; ++iMipmap) {
			auto &imgBufferInfo = imageBufferInfos.at(iLayer * numMipmaps + iMipmap);

			auto imgBuffer = image::ImageBuffer::Create(imgBufferInfo.width, imgBufferInfo.height, image::Format::RGBA8);
			buf->Read(imgBufferInfo.bufferOffset, imgBufferInfo.bufferSize, imgBuffer->GetData());
			mipmapBuffers.push_back(imgBuffer);
		}
	}

	std::vector<std::vector<const void *>> ptrCubemapBuffers {};
	ptrCubemapBuffers.resize(numLayers);
	for(auto iLayer = decltype(numLayers) {0u}; iLayer < numLayers; ++iLayer) {
		ptrCubemapBuffers.at(iLayer).resize(numMipmaps);
		for(auto iMipmap = decltype(numMipmaps) {0u}; iMipmap < numMipmaps; ++iMipmap)
			ptrCubemapBuffers.at(iLayer).at(iMipmap) = cubemapBuffers.at(iLayer).at(iMipmap)->GetData();
	}

	// Save the cubemap image on disk; It will automatically be reloaded
	image::TextureInfo imgWriteInfo {};
	imgWriteInfo.containerFormat = containerFormat;
	imgWriteInfo.inputFormat = image::TextureInfo::InputFormat::R8G8B8A8_UInt;
	imgWriteInfo.outputFormat = image::TextureInfo::OutputFormat::ColorMap;
	imgWriteInfo.wrapMode = image::TextureInfo::WrapMode::Clamp;
	auto fullPath = "addons/converted/materials/" + matName;
	auto szPerPixel = prosper::util::get_byte_size(prosper::Format::R8G8B8A8_UNorm);
	if(get_cgame()->SaveImage(ptrCubemapBuffers, largestWidth, largestHeight, szPerPixel, fullPath, imgWriteInfo, true)) {
		Con::COUT << "Skybox cubemap texture saved as '" << fullPath << "'! Generating material..." << Con::endl;
		auto mat = get_client_state()->CreateMaterial("skybox");
		mat->SetTextureProperty("skybox", matName);
		auto savePath = pragma::asset::relative_path_to_absolute_path(matName, asset::Type::Material, util::CONVERT_PATH);
		std::string err;
		if(mat->Save(savePath.GetString(), err, true)) {
			get_client_state()->LoadMaterial(matName, nullptr, true, true);
			Con::COUT << "Skybox material saved as '" << (matName + ".wmi") << "'" << Con::endl;
			return true;
		}
		else
			Con::CWAR << "Unable to save skybox material as '" << (matName + ".wmi") << "': " << err << "!" << Con::endl;
	}
	else
		Con::CWAR << "Unable to save skybox cubemap texture as '" << fullPath << "'!" << Con::endl;
	return false;
}
void CSkyboxComponent::ValidateMaterials()
{
	// TODO: Move this to Source Engine porting module?
	auto mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	auto mdlC = GetEntity().GetComponent<CModelComponent>();
	auto *mat = mdlC->GetRenderMaterial(0);
	if(!mat)
		return;
	auto &metaInfo = mdl->GetMetaInfo();
	auto &texturePaths = metaInfo.texturePaths;
	auto &texturePath = texturePaths.front();
	auto *texInfo = mat ? mat->GetTextureInfo("skybox") : nullptr;
	if(texInfo) {
		// Skybox is valid; Skip the material
		if(texInfo->texture) {
			auto &tex = *static_cast<material::Texture *>(texInfo->texture.get());
			auto &vkTex = tex.GetVkTexture();
			if(vkTex) {
				auto &img = vkTex->GetImage();
				if(img.IsCubemap() == false) {
					// Equirectangular skybox; We need to convert it to a skybox
					auto *shader = static_cast<ShaderEquirectangularToCubemap *>(get_cengine()->GetShader("equirectangular_to_cubemap").get());
					if(shader) {
						constexpr uint32_t resolution = 1024;
						auto tex = shader->EquirectangularTextureToCubemap(*vkTex, resolution);
						if(tex) {
							static_cast<material::CMaterial *>(mat)->SetTexture("skybox", *tex);
							static_cast<material::CMaterial *>(mat)->UpdateTextures();
						}
					}
				}
			}
		}
		return;
	}
	if(!metaInfo.textures.empty()) {
		// Attempt to use HDR textures, otherwise LDR
		auto texture = ufile::get_file_from_filename(metaInfo.textures.front());
		ufile::remove_extension_from_filename(texture, pragma::asset::get_supported_extensions(asset::Type::Material));
		if(CreateCubemapFromIndividualTextures(texturePath + texture + ".pmat", "_hdr") || CreateCubemapFromIndividualTextures(texturePath + texture + ".pmat"))
			mdl->LoadMaterials();
	}
}
void CSkyboxComponent::SetSkyMaterial(material::Material *mat)
{
	auto &ent = GetEntity();
	auto mdlC = ent.GetComponent<CModelComponent>();
	if(mdlC.expired())
		return;
	if(mat) {
		auto overrideC = ent.AddComponent<CMaterialOverrideComponent>();
		overrideC->SetMaterialOverride(0, static_cast<material::CMaterial &>(*mat));
	}
	else {
		auto overrideC = ent.GetComponent<CMaterialOverrideComponent>();
		if(overrideC.valid())
			overrideC->ClearMaterialOverride(0);
	}
	ValidateMaterials();
	mdlC->UpdateRenderMeshes();
}

void CSkybox::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CSkyboxComponent>();
}

static void sky_override(NetworkState *, const console::ConVar &, std::string, std::string skyMat)
{
	if(get_cgame() == nullptr)
		return;
	material::CMaterial *matSky = nullptr;
	if(skyMat.empty() == false)
		matSky = static_cast<material::CMaterial *>(get_client_state()->LoadMaterial(skyMat, nullptr, false, true));
	ecs::EntityIterator entIt {*get_cgame()};
	entIt.AttachFilter<TEntityIteratorFilterComponent<CSkyboxComponent>>();
	entIt.AttachFilter<TEntityIteratorFilterComponent<CModelComponent>>();
	for(auto *ent : entIt)
		ent->GetComponent<CSkyboxComponent>()->SetSkyMaterial(matSky);
}
namespace {
	auto _ = pragma::console::client::register_variable_listener<std::string>("sky_override", &sky_override);
}

enum class ConversionMode : uint8_t { CubemapToEquirectangular = 0, EquirectangularToCubemap };

static void util_convert_cubemap_equirect(std::vector<std::string> &argv, ConversionMode conversionMode)
{
	if(argv.empty()) {
		Con::CWAR << "No image path has been specified!" << Con::endl;
		return;
	}
	auto &fileName = argv.front();
	auto assetFileName = pragma::asset::find_file(argv.front(), asset::Type::Texture);
	if(!assetFileName.has_value()) {
		Con::CWAR << "Failed to locate texture file for '" << *assetFileName << "'!" << Con::endl;
		return;
	}

	std::string absPath;
	if(!fs::find_local_path("materials/" + *assetFileName, absPath)) {
		Con::CWAR << "Failed to determine absolute path for texture file '" << *assetFileName << "'!" << Con::endl;
		return;
	}

	auto &matMan = static_cast<material::CMaterialManager &>(get_client_state()->GetMaterialManager());
	auto tex = matMan.GetTextureManager().LoadAsset(fileName, util::AssetLoadFlags::DontCache | util::AssetLoadFlags::IgnoreCache);
	if(!tex) {
		Con::CWAR << "Texture '" << fileName << "' could not be loaded!" << Con::endl;
		return;
	}

	auto &vkTex = tex->GetVkTexture();
	{
		//auto el = pragma::gui::WGUI::GetInstance().Create<WITexturedRect>();
		//el->SetTexture(*vkTex,0);
		//el->SetSize(512,512);
		//return;
		//image::TextureInfo texInfo {};
		//texInfo.containerFormat = image::TextureInfo::ContainerFormat::DDS;
		//prosper::util::save_texture("test.dds",vkTex->GetImage(),texInfo);
	}

	if(!vkTex) {
		Con::CWAR << "Invalid texture!" << Con::endl;
		return;
	}

	auto outputFileName = util::Path::CreateFile(absPath);
	outputFileName.RemoveFileExtension(pragma::asset::get_supported_extensions(asset::Type::Texture));
	if(conversionMode == ConversionMode::EquirectangularToCubemap) {
		auto *shader = static_cast<ShaderEquirectangularToCubemap *>(get_cengine()->GetShader("equirectangular_to_cubemap").get());
		if(!shader) {
			Con::CWAR << "Invalid shader!" << Con::endl;
			return;
		}
		auto texCubemap = shader->EquirectangularTextureToCubemap(*vkTex, 1'024);
		if(!texCubemap) {
			Con::CWAR << "Conversion failed!" << Con::endl;
			return;
		}
		image::TextureInfo texInfo {};
		texInfo.containerFormat = image::TextureInfo::ContainerFormat::DDS;
		outputFileName += "_cubemap.dds";
		auto res = prosper::util::save_texture(outputFileName.GetString(), texCubemap->GetImage(), texInfo);
		if(!res) {
			Con::CWAR << "Failed to save output texture '" << outputFileName.GetString() << "'!" << Con::endl;
			return;
		}
		Con::COUT << "Successfully saved output file '" << outputFileName.GetString() << "'!" << Con::endl;
		return;
	}

	auto *shader = static_cast<ShaderCubemapToEquirectangular *>(get_cengine()->GetShader("cubemap_to_equirectangular").get());
	if(!shader) {
		Con::CWAR << "Invalid shader!" << Con::endl;
		return;
	}
	auto texEqui = shader->CubemapToEquirectangularTexture(*vkTex);
	if(!texEqui) {
		Con::CWAR << "Conversion failed!" << Con::endl;
		return;
	}

	// TODO: We should save it as HDR, but that causes weird artifacts. Investigate! Maybe add support for exr?
	auto saveAsHdr = false;
	auto imgBuf = texEqui->GetImage().ToHostImageBuffer(image::Format::RGBA16, prosper::ImageLayout::ShaderReadOnlyOptimal);
	if(!imgBuf) {
		Con::CWAR << "Failed to generate host image buffer!" << Con::endl;
		return;
	}
	if(saveAsHdr)
		outputFileName += "_equirect.hdr";
	else
		outputFileName += "_equirect.png";
	auto f = fs::open_file(outputFileName.GetString(), fs::FileMode::Write | fs::FileMode::Binary);
	if(!f) {
		Con::CWAR << "Failed to open output file '" << outputFileName.GetString() << "'!" << Con::endl;
		return;
	}
	fs::File fp {f};
	auto res = image::save_image(fp, *imgBuf, saveAsHdr ? image::ImageFormat::HDR : image::ImageFormat::PNG);
	if(!res) {
		Con::CWAR << "Failed to save output file '" << outputFileName.GetString() << "'!" << Con::endl;
		return;
	}
	Con::COUT << "Successfully saved output file '" << outputFileName.GetString() << "'!" << Con::endl;
}
static void util_convert_cubemap_to_equirectangular_image(NetworkState *state, BasePlayerComponent *pl, std::vector<std::string> &argv) { util_convert_cubemap_equirect(argv, ConversionMode::CubemapToEquirectangular); }
static void util_convert_equirectangular_image_to_cubemap(NetworkState *state, BasePlayerComponent *pl, std::vector<std::string> &argv) { util_convert_cubemap_equirect(argv, ConversionMode::EquirectangularToCubemap); }

namespace {
	auto UVN = console::client::register_command("util_convert_cubemap_to_equirectangular_image", &util_convert_cubemap_to_equirectangular_image, console::ConVarFlags::None, "Converts a cubemap to a equirectangular image.");
	auto UVN = console::client::register_command("util_convert_equirectangular_image_to_cubemap", &util_convert_equirectangular_image_to_cubemap, console::ConVarFlags::None, "Converts a equirectangular image to a cubemap.");
}
