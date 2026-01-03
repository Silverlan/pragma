// SPDX-FileCopyrightText: (c) 2020 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:assets.import_export;

export import :entities.components.camera;
export import :entities.components.lights.light;
export import :rendering.cycles;
export import pragma.shared;

export namespace pragma::asset {
	constexpr std::string_view EXPORT_PATH = "export/";
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
		float scale = units_to_metres(1.f);

		bool generateAo = false;
		uint32_t aoResolution = 512;
		uint32_t aoSamples = 40;
		rendering::cycles::SceneInfo::DeviceType aoDevice = rendering::cycles::SceneInfo::DeviceType::CPU;

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
		const std::vector<ComponentHandle<CCameraComponent>> &GetCameras() const { return m_cameras; }

		void AddLightSource(CLightComponent &light);
		const std::vector<ComponentHandle<CLightComponent>> &GetLightSources() const { return m_lightSources; }
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

	struct DLLCLIENT AssetImportResult {
		std::vector<std::string> models;
		std::vector<std::shared_ptr<Model>> modelObjects;
		std::string mapName;
	};
	DLLCLIENT std::optional<AssetImportResult> import_gltf(ufile::IFile &f, std::string &outErrMsg, const util::Path &outputPath = {}, bool importAsSingleModel = false);
	DLLCLIENT std::optional<AssetImportResult> import_gltf(const std::string &fileName, std::string &outErrMsg, const util::Path &outputPath = {}, bool importAsSingleModel = false);

	DLLCLIENT std::optional<AssetImportResult> import_fbx(ufile::IFile &f, std::string &outErrMsg, const util::Path &outputPath = {});
	DLLCLIENT std::optional<AssetImportResult> import_fbx(const std::string &fileName, std::string &outErrMsg, const util::Path &outputPath = {});

	DLLCLIENT bool import_texture(const std::string &fileName, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg);
	DLLCLIENT bool import_texture(std::unique_ptr<ufile::IFile> &&f, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg);
	DLLCLIENT bool import_texture(prosper::IImage &img, const TextureImportInfo &texInfo, const std::string &outputPath, std::string &outErrMsg);

	DLLCLIENT void assign_texture(material::CMaterial &mat, const std::string &textureRootPath, const std::string &matIdentifier, const std::string &texName, prosper::IImage &img, bool greyScale, bool normalMap, AlphaMode alphaMode = AlphaMode::Opaque);

	DLLCLIENT bool export_model(Model &model, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &modelName = {}, std::string *optOutPath = nullptr);
	DLLCLIENT bool export_animation(Model &model, const std::string &animName, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<std::string> &modelName = {});
	DLLCLIENT bool export_map(const std::string &mapName, const ModelExportInfo &exportInfo, std::string &outErrMsg, const std::optional<MapExportInfo> &mapExportInfo = {});
	DLLCLIENT bool export_texture(image::ImageBuffer &imgBuf, ModelExportInfo::ImageFormat imageFormat, const std::string &outputPath, std::string &outErrMsg, bool normalMap = false, bool srgb = false, image::TextureInfo::AlphaMode alphaMode = image::TextureInfo::AlphaMode::Auto,
	  std::string *optOutOutputPath = nullptr);
	DLLCLIENT bool export_texture(const std::string &texturePath, ModelExportInfo::ImageFormat imageFormat, std::string &outErrMsg, image::TextureInfo::AlphaMode alphaMode = image::TextureInfo::AlphaMode::Auto, bool enableExtendedDDS = false, std::string *optExportPath = nullptr,
	  std::string *optOutOutputPath = nullptr, const std::optional<std::string> &optFileNameOverride = {});
	using MaterialTexturePaths = std::unordered_map<std::string, std::string>;
	DLLCLIENT std::optional<MaterialTexturePaths> export_material(material::Material &mat, ModelExportInfo::ImageFormat imageFormat, std::string &outErrMsg, std::string *optExportPath = nullptr, bool normalizeTextureNames = false);

	struct DLLCLIENT ModelAOWorkerResult {
		bool success = false;
		std::string errMsg;
	};
	DLLCLIENT std::optional<util::ParallelJob<ModelAOWorkerResult>> generate_ambient_occlusion(Model &mdl, std::string &outErrMsg, bool forceRebuild = false, uint32_t aoResolution = 512, uint32_t aoSamples = 40,
	  rendering::cycles::SceneInfo::DeviceType aoDevice = rendering::cycles::SceneInfo::DeviceType::CPU);
	enum class AOResult : uint8_t { NoAOGenerationRequired = 0, FailedToCreateAOJob, AOJobReady };
	DLLCLIENT AOResult generate_ambient_occlusion(Model &mdl, material::Material &mat, util::ParallelJob<image::ImageLayerSet> &outJob, std::string &outErrMsg, bool forceRebuild = false, uint32_t aoResolution = 512, uint32_t aoSamples = 40,
	  rendering::cycles::SceneInfo::DeviceType aoDevice = rendering::cycles::SceneInfo::DeviceType::CPU);

	DLLCLIENT std::optional<prosper::Format> vtf_format_to_prosper(VtfInfo::Format format);
	DLLCLIENT std::optional<VtfInfo::Format> prosper_format_to_vtf(prosper::Format format);
	DLLCLIENT bool export_texture_as_vtf(const std::string &fileName, const std::function<const uint8_t *(uint32_t, uint32_t)> &fGetImgData, uint32_t width, uint32_t height, uint32_t szPerPixel, uint32_t numLayers, uint32_t numMipmaps, bool cubemap, const VtfInfo &texInfo,
	  const std::function<void(const std::string &)> &errorHandler, bool absoluteFileName);
	DLLCLIENT bool export_texture_as_vtf(const std::string &fileName, const prosper::IImage &img, const VtfInfo &texInfo, const std::function<void(const std::string &)> &errorHandler, bool absoluteFileName);

	DLLCLIENT image::TextureInfo get_texture_info(bool isGreyScale, bool isNormalMap, AlphaMode alphaMode = AlphaMode::Opaque);
};
