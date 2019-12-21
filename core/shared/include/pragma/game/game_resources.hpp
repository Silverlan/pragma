#ifndef __GAME_RESOURCES_HP__
#define __GAME_RESOURCES_HP__

#include "pragma/networkdefinitions.h"
#include "pragma/model/vertex.h"
#include <string>

class NetworkState;
namespace bsp {class File;};
namespace util
{
	static const std::string IMPORT_PATH = "addons\\imported\\";
	namespace impl
	{
		void *get_module_func(NetworkState *nw,const std::string &name);
	};
	DLLNETWORK void initialize_external_archive_manager(NetworkState *nw);
	DLLNETWORK void close_external_archive_manager();

	DLLNETWORK bool port_nif_model(NetworkState *nw,const std::string &path,std::string mdlName);
	DLLNETWORK bool port_hl2_smd(NetworkState &nw,Model &mdl,VFilePtr &f,const std::string &animName,bool isCollisionMesh,std::vector<std::string> &outTextures);
	DLLNETWORK bool port_hl2_model(NetworkState *nw,const std::string &path,std::string mdlName);
	DLLNETWORK bool port_hl2_particle(NetworkState *nw,const std::string &path);
	DLLNETWORK bool port_hl2_map(NetworkState *nw,const std::string &path);
	DLLNETWORK bool port_file(NetworkState *nw,const std::string &path);
	DLLNETWORK bool port_sound_script(NetworkState *nw,const std::string &path);

	namespace bsp
	{
		struct DLLNETWORK TexInfo
		{
			std::array<Vector4,2> lightMapVecs;
			std::array<Vector4,2> textureVecs;
			int32_t materialIndex;
		};
		struct DLLNETWORK FaceLightMapInfo
		{
			enum class Flags : uint8_t
			{
				None = 0u,
				Valid = 1u,
				Rotated = Valid<<1u
			};
			bool valid() const;
			int32_t x;
			int32_t y;
			std::array<int32_t,2> lightMapSize;
			std::array<int32_t,2> lightMapMins;
			Flags flags;
			size_t faceIndex;
			uint32_t luxelDataOffset;
			int16_t texInfoIndex;
			int16_t dispInfoIndex;
			uint32_t firstEdge;
			uint16_t numEdges;
			Vector3 planeNormal;
		};
		struct DLLNETWORK LightMapInfo
		{
			struct DLLNETWORK DispSamplePosition
			{
				uint32_t triangleIndex;
				Vector3 barycentricCoordinates;
			};
			struct DLLNETWORK Rect
			{
				uint16_t x;
				uint16_t y;
				uint16_t w;
				uint16_t h;
			};
			uint32_t borderSize = 1u;
			uint32_t atlasSize = 0u;
			std::vector<FaceLightMapInfo> faceInfos = {};
			std::vector<uint8_t> luxelData = {}; // Vector of bsp::ColorRGBExp32
			std::vector<uint8_t> dispLightmapSamplePositions = {};
			std::vector<Rect> lightmapAtlas {};
		};
		struct DLLNETWORK DispInfo
		{
			struct DLLNETWORK VertInfo
			{
				Vector3 vec;	// Vector field defining displacement volume.
				float dist;	// Displacement distances.
				float alpha;	// "per vertex" alpha values.
			};
			std::vector<VertInfo> vertices;
			int32_t dispVertStart;
			int32_t lightmapSamplePositionStart;
			int32_t power;
			Vector3 startPosition;
		};
		struct DLLNETWORK GeometryData
		{
			struct MeshInfo
			{
				std::vector<Vertex> vertices;
				std::vector<Vector2> lightMapUvs;
			};
			LightMapInfo lightMapData = {};
		};
		GeometryData load_bsp_geometry(NetworkState &nw,::bsp::File &bsp);
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(util::bsp::FaceLightMapInfo::Flags)

#endif
