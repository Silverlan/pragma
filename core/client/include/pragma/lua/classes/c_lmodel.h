#ifndef __C_LMODEL_H__
#define __C_LMODEL_H__

#include "pragma/clientdefinitions.h"
#include "pragma/lua/c_ldefinitions.h"
#include "pragma/rendering/raytracing/cycles.hpp"
#include <pragma/util/util_game.hpp>

class CModel;
namespace Lua
{
	namespace Model
	{
		namespace Client
		{
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
				bool exportImages = true;
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
			};

			DLLCLIENT void AddMaterial(lua_State *l,::Model &mdl,uint32_t textureGroup,const std::string &name);
			DLLCLIENT void SetMaterial(lua_State *l,::Model &mdl,uint32_t texIdx,const std::string &name);
			DLLCLIENT void GetVertexAnimationBuffer(lua_State *l,::Model &mdl);
			DLLCLIENT void Export(lua_State *l,::Model &mdl,const ModelExportInfo &exportInfo);
		};
	};
};

#endif