#ifndef __C_PRAGMA_ASSET_UTIL_MODEL_HPP__
#define __C_PRAGMA_ASSET_UTIL_MODEL_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/raytracing/cycles.hpp"
#include <pragma/util/util_game.hpp>
#include <pragma/asset/util_asset.hpp>
#include <util_texture_info.hpp>
#include <sharedutils/util_path.hpp>

class Model;
namespace pragma::asset
{
	static std::string EXPORT_PATH = "export/";
	struct DLLCLIENT ModelExportInfo
	{
		enum class ImageFormat : uint8_t
		{
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
		ImageFormat imageFormat = ImageFormat::DDS;
		float scale = ::util::units_to_metres(1.f);

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

	DLLCLIENT std::shared_ptr<Model> import_model(VFilePtr f,std::string &outErrMsg,const util::Path &outputPath={});
	DLLCLIENT std::shared_ptr<Model> import_model(const std::string &fileName,std::string &outErrMsg,const util::Path &outputPath={});

	DLLCLIENT bool export_model(Model &model,const ModelExportInfo &exportInfo,std::string &outErrMsg,const std::string *optModelName=nullptr);
	DLLCLIENT bool export_animation(Model &model,const std::string &animName,const ModelExportInfo &exportInfo,std::string &outErrMsg,const std::string *optModelName=nullptr);
	DLLCLIENT bool export_map(const std::string &mapName,const ModelExportInfo &exportInfo,std::string &outErrMsg);
	DLLCLIENT bool export_texture(
		uimg::ImageBuffer &imgBuf,ModelExportInfo::ImageFormat imageFormat,const std::string &outputPath,std::string &outErrMsg,
		bool normalMap=false,bool srgb=false,uimg::TextureInfo::AlphaMode alphaMode=uimg::TextureInfo::AlphaMode::Auto,
		std::string *optOutOutputPath=nullptr
	);
	DLLCLIENT bool export_texture(
		const std::string &texturePath,ModelExportInfo::ImageFormat imageFormat,std::string &outErrMsg,
		uimg::TextureInfo::AlphaMode alphaMode=uimg::TextureInfo::AlphaMode::Auto,bool enableExtendedDDS=false,
		std::string *optExportPath=nullptr,std::string *optOutOutputPath=nullptr
	);
	using MaterialTexturePaths = std::unordered_map<std::string,std::string>;
	DLLCLIENT std::optional<MaterialTexturePaths> export_material(Material &mat,ModelExportInfo::ImageFormat imageFormat,std::string &outErrMsg,std::string *optExportPath=nullptr);

	struct DLLCLIENT ModelAOWorkerResult
	{
		bool success = false;
		std::string errMsg;
	};
	DLLCLIENT std::optional<util::ParallelJob<ModelAOWorkerResult>> generate_ambient_occlusion(
		Model &mdl,std::string &outErrMsg,bool forceRebuild=false,uint32_t aoResolution=512,
		uint32_t aoSamples=40,pragma::rendering::cycles::SceneInfo::DeviceType aoDevice=pragma::rendering::cycles::SceneInfo::DeviceType::CPU
	);
	enum class AOResult : uint8_t
	{
		NoAOGenerationRequired = 0,
		FailedToCreateAOJob,
		AOJobReady
	};
	DLLCLIENT AOResult generate_ambient_occlusion(
		Model &mdl,Material &mat,util::ParallelJob<std::shared_ptr<uimg::ImageBuffer>> &outJob,std::string &outErrMsg,bool forceRebuild=false,uint32_t aoResolution=512,
		uint32_t aoSamples=40,pragma::rendering::cycles::SceneInfo::DeviceType aoDevice=pragma::rendering::cycles::SceneInfo::DeviceType::CPU
	);
};

#endif
