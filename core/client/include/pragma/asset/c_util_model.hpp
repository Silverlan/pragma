/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PRAGMA_ASSET_UTIL_MODEL_HPP__
#define __C_PRAGMA_ASSET_UTIL_MODEL_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/raytracing/cycles.hpp"
#include <pragma/util/util_game.hpp>
#include <pragma/asset/util_asset.hpp>
#include <pragma/types.hpp>
#include <util_texture_info.hpp>
#include <sharedutils/util_path.hpp>

class Model;
namespace prosper {
	class IImage;
	enum class Format : uint32_t;
};
namespace ufile {
	struct IFile;
};
namespace pragma::asset {
	static std::string EXPORT_PATH = "export/";
	struct DLLCLIENT ModelExportInfo {
		enum class ImageFormat : uint8_t {
			PNG = 0,
			BMP,
			TGA,
			JPG,
			HDR,

			DDS,
			KTX
		};
		bool exportAnimations = true;
		bool exportSkinnedMeshData = true;
		bool exportMorphTargets = true;
		bool exportImages = true;
		bool embedAnimations = true;
		bool fullExport = false;
		bool normalizeTextureNames = false;
		ImageFormat imageFormat = ImageFormat::DDS;
		float scale = ::pragma::units_to_metres(1.f);

		bool generateAo = false;
		uint32_t aoResolution = 512;
		uint32_t aoSamples = 40;
		pragma::rendering::cycles::SceneInfo::DeviceType aoDevice = pragma::rendering::cycles::SceneInfo::DeviceType::CPU;

		bool mergeMeshesByMaterial = true;
		bool enableExtendedDDS = false;
		bool saveAsBinary = false;
		bool verbose = true;

		void SetAnimationList(const std::vector<std::string> &animations);
		std::vector<std::string> *GetAnimationList();
	  private:
		std::optional<std::vector<std::string>> m_animations {};
	};
	struct DLLCLIENT MapExportInfo {
		bool includeMapLightSources = true;
		void AddCamera(CCameraComponent &cam);
		const std::vector<pragma::ComponentHandle<CCameraComponent>> &GetCameras() const { return m_cameras; }

		void AddLightSource(CLightComponent &light);
		const std::vector<pragma::ComponentHandle<CLightComponent>> &GetLightSources() const { return m_lightSources; }
	  private:
		std::vector<ComponentHandle<CCameraComponent>> m_cameras {};
		std::vector<ComponentHandle<CLightComponent>> m_lightSources {};
	};
	struct DLLCLIENT TextureImportInfo {
		bool srgb = false;
		bool normalMap = false;
		bool greyScaleMap = false;
	};

	DLLCLIENT std::shared_ptr<Model> import_model(ufile::IFile &f, std::string &outErrMsg, const util::Path &outputPath = {}, bool importAsSingleModel = true);
	DLLCLIENT std::shared_ptr<Model> import_model(const std::string &fileName, std::string &outErrMsg, const util::Path &outputPath = {}, bool importAsSingleModel = true);

	struct DLLCLIENT GltfImportInfo {
		std::vector<std::string> models;
		std::string mapName;
	};
	DLLCLIENT std::optional<GltfImportInfo> import_gltf(ufile::IFile &f, std::string &outErrMsg, const util::Path &outputPath = {}, bool importAsSingleModel = false);
	DLLCLIENT std::optional<GltfImportInfo> import_gltf(const std::string &fileName, std::string &outErrMsg, const util::Path &outputPath = {}, bool importAsSingleModel = false);

	DLLCLIENT bool import_texture(const std::string &fileName, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg);
	DLLCLIENT bool import_texture(std::unique_ptr<ufile::IFile> &&f, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg);
	DLLCLIENT bool import_texture(prosper::IImage &img, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg);

	DLLCLIENT bool export_model(Model &model, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &modelName = {}, std::string *optOutPath = nullptr);
	DLLCLIENT bool export_animation(Model &model, const std::string &animName, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &modelName = {});
	DLLCLIENT bool export_map(const std::string &mapName, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<MapExportInfo> &mapExportInfo = {});
	DLLCLIENT bool export_texture(uimg::ImageBuffer &imgBuf, ModelExportInfo::ImageFormat imageFormat, const std::string &outputPath, std::string &outErrMsg, bool normalMap = false, bool srgb = false, uimg::TextureInfo::AlphaMode alphaMode = uimg::TextureInfo::AlphaMode::Auto,
	  std::string *optOutOutputPath = nullptr);
	DLLCLIENT bool export_texture(const std::string &texturePath, ModelExportInfo::ImageFormat imageFormat, std::string &outErrMsg, uimg::TextureInfo::AlphaMode alphaMode = uimg::TextureInfo::AlphaMode::Auto, bool enableExtendedDDS = false, std::string *optExportPath = nullptr,
	  std::string *optOutOutputPath = nullptr, const std::optional<std::string> &optFileNameOverride = {});
	using MaterialTexturePaths = std::unordered_map<std::string, std::string>;
	DLLCLIENT std::optional<MaterialTexturePaths> export_material(Material &mat, ModelExportInfo::ImageFormat imageFormat, std::string &outErrMsg, std::string *optExportPath = nullptr, bool normalizeTextureNames = false);

	struct DLLCLIENT ModelAOWorkerResult {
		bool success = false;
		std::string errMsg;
	};
	DLLCLIENT std::optional<util::ParallelJob<ModelAOWorkerResult>> generate_ambient_occlusion(Model &mdl, std::string &outErrMsg, bool forceRebuild = false, uint32_t aoResolution = 512, uint32_t aoSamples = 40,
	  pragma::rendering::cycles::SceneInfo::DeviceType aoDevice = pragma::rendering::cycles::SceneInfo::DeviceType::CPU);
	enum class AOResult : uint8_t { NoAOGenerationRequired = 0, FailedToCreateAOJob, AOJobReady };
	DLLCLIENT AOResult generate_ambient_occlusion(Model &mdl, Material &mat, util::ParallelJob<uimg::ImageLayerSet> &outJob, std::string &outErrMsg, bool forceRebuild = false, uint32_t aoResolution = 512, uint32_t aoSamples = 40,
	  pragma::rendering::cycles::SceneInfo::DeviceType aoDevice = pragma::rendering::cycles::SceneInfo::DeviceType::CPU);

	DLLCLIENT std::optional<prosper::Format> vtf_format_to_prosper(VtfInfo::Format format);
	DLLCLIENT std::optional<VtfInfo::Format> prosper_format_to_vtf(prosper::Format format);
	DLLCLIENT bool export_texture_as_vtf(const std::string &fileName, const std::function<const uint8_t *(uint32_t, uint32_t)> &fGetImgData, uint32_t width, uint32_t height, uint32_t szPerPixel, uint32_t numLayers, uint32_t numMipmaps, bool cubemap, const VtfInfo &texInfo,
	  const std::function<void(const std::string &)> &errorHandler, bool absoluteFileName);
	DLLCLIENT bool export_texture_as_vtf(const std::string &fileName, const prosper::IImage &img, const VtfInfo &texInfo, const std::function<void(const std::string &)> &errorHandler, bool absoluteFileName);
};

#endif
