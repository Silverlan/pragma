#ifndef __PRAGMA_BSP_CONVERTER_HPP__
#define __PRAGMA_BSP_CONVERTER_HPP__

#include "pragma/networkdefinitions.h"
#include <pragma/game/game_resources.hpp>
#include <memory>
#include <functional>
#include <string>
#include <unordered_set>
#include <util_fgd.hpp>
#include <util_bsp.hpp>
#include <pragma/util/util_bsp_tree.hpp>
#include <mathutil/color.h>
#include <mathutil/uvec.h>
#include <pragma/math/surfacematerial.h>
#include <material.h>

class Game;
class CollisionMesh;
class BrushMesh;
namespace bsp {class File; struct dDisp;};
namespace util {class BSPTree;};
namespace vmf {class PolyMesh;};
namespace util::fgd {struct Data;};
namespace pragma::asset {class EntityData; class WorldData;};
namespace pragma::asset::vbsp
{
	class BSPTree
		: public util::BSPTree
	{
	public:
		static std::shared_ptr<BSPTree> Create(bsp::File &bsp);
	protected:
		static void InitializeNode(Node &node,::bsp::File &bsp,const std::array<int16_t,3u> &min,const std::array<int16_t,3u> &max,uint16_t firstFace,uint16_t faceCount);
		std::shared_ptr<Node> CreateNode(::bsp::File &bsp,int32_t nodeIndex);
		std::shared_ptr<Node> CreateLeaf(::bsp::File &bsp,int32_t nodeIndex);
		BSPTree();
	};

	struct BSPEntityData
	{
		std::unordered_set<uint32_t> brushIndices {};
		std::vector<uint32_t> modelIndices {};
		std::vector<std::shared_ptr<vmf::PolyMesh>> polyMeshes {};
		std::vector<int32_t> faceIndices {};
	};

	struct TexInfo
	{
		std::array<Vector4,2> lightMapVecs;
		std::array<Vector4,2> textureVecs;
		int32_t materialIndex;
	};
	struct FaceLightMapInfo
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
	struct DispInfo
	{
		struct VertInfo
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

	struct LightmapData
	{
		struct DispSamplePosition
		{
			uint32_t triangleIndex;
			Vector3 barycentricCoordinates;
		};
		struct Rect
		{
			uint16_t x;
			uint16_t y;
			uint16_t w;
			uint16_t h;
		};
		uint32_t borderSize = 1u;
		Vector2i atlasSize = {0,0};
		std::vector<FaceLightMapInfo> faceInfos = {};
		std::vector<uint8_t> luxelData = {}; // Vector of bsp::ColorRGBExp32
		std::vector<uint8_t> dispLightmapSamplePositions = {};
		std::vector<Rect> lightmapAtlas {};
	};

	struct BSPInputData
	{
		BSPInputData();
		std::shared_ptr<LightmapData> lightmapData = nullptr;
		std::vector<int32_t> surfEdges {};
		std::vector<uint16_t> leafFaces {};
		std::vector<bsp::dedge_t> edges {};
		std::vector<Vector3> verts {};
		std::vector<TexInfo> texInfo {};
		std::vector<DispInfo> displacementInfo = {};
		std::shared_ptr<util::BSPTree> bspTree;
	};

	enum class ContentsFlags : uint64_t
	{
		Empty = 0,

		Solid = 0x1,
		Window = 0x2,
		Aux = 0x4,
		Grate = 0x8,
		Slime = 0x10,
		Water = 0x20,
		BlockLOS = 0x40,
		Opaque = 0x80,

		TestFogVolume = 0x100,
		Unused = 0x200,

		Unused6 = 0x400,

		Team1 = 0x800,
		Team2 = 0x1000,

		IgnoreNodrawOpaque = 0x2000,
		Moveable = 0x4000,
		AreaPortal = 0x8000,
		PlayerClip = 0x10000,
		MonsterClip = 0x20000,

		Current0 = 0x40000,
		Current90 = 0x80000,
		Current180 = 0x100000,
		Current270 = 0x200000,
		CurrentUp = 0x400000,
		CurrentDown = 0x800000,

		Origin = 0x1000000,

		Monster = 0x2000000,
		Debris = 0x4000000,
		Detail = 0x8000000,
		Translucent = 0x10000000,
		Ladder = 0x20000000,
		Hitbox = 0x40000000
	};
	enum class SurfFlags : uint32_t
	{
		Light = 0x0001,
		Sky2D = 0x0002,
		Sky = 0x0004,
		Warp = 0x0008,
		Trans = 0x0010,
		NoPortal = 0x0020,
		Trigger = 0x0040,
		Nodraw = 0x0080,

		Hint = 0x0100,

		Skip = 0x0200,
		NoLight = 0x0400,
		BumpLight = 0x0800,
		NoShadows = 0x1000,
		NoDecals = 0x2000,
		NoChop = 0x400,
		Hitbox = 0x8000
	};

	class BSPConverter
	{
	public:
		static const Color WLD_DEFAULT_AMBIENT_COLOR;

		enum class MeshType : uint8_t
		{
			PolyMesh = 0u,
			Displacement,
			FaceMesh
		};

		static std::shared_ptr<BSPConverter> Open(Game &game,const std::string &path);
		static void BuildDisplacement(bsp::File &bsp,NetworkState *nw,const bsp::dDisp &disp,std::vector<std::shared_ptr<vmf::PolyMesh>> &outMeshes);
		static Vector3 BSPVertexToPragma(const Vector3 &inPos);
		static Vector3 BSPTextureVecToPragma(const Vector3 &vec);

		std::string GetMapName() const;
		bool StartConversion();
		void SetMessageLogger(const std::function<void(const std::string&)> &msgLogger);
	private:
		static Vector2 CalcDispSurfCoords(const std::array<Vector2,4> &texCoords,uint32_t power,uint32_t x,uint32_t y);
		static std::vector<std::shared_ptr<CollisionMesh>> GenerateCollisionMeshes(Game &game,std::vector<std::shared_ptr<BrushMesh>> &meshes,const std::vector<SurfaceMaterial> &surfaceMaterials);

		static void BuildBrushMesh(bsp::File &bsp,NetworkState *nw,const std::unordered_set<uint32_t> &brushIds,std::vector<std::shared_ptr<vmf::PolyMesh>> &outMeshes);
		static void FindModelBrushes(bsp::File &bsp,const bsp::dmodel_t &mdl,std::unordered_set<uint32_t> &brushIds);

		BSPConverter(NetworkState &nw,Game &game,std::unique_ptr<bsp::File> &&bsp,const std::string &path);

		static std::shared_ptr<LightmapData> LoadLightmapData(NetworkState &nw,::bsp::File &bsp);

		void LoadFGDData();
		void ExtractBSPFiles();
		BSPEntityData *FindBSPEntityData(pragma::asset::EntityData &entData);
		void ConvertEntityData();
		void ParseEntityGeometryData(
			pragma::asset::EntityData &entData,const std::unordered_set<uint32_t> &materialRemovalTable,
			int32_t skyTexIdx,int32_t skyTex2dIdx
		);
		bool GenerateLightMapAtlas(LightmapData &lightmapInfo,const std::string &mapName);
		void ConvertWorldGeometry();
		std::shared_ptr<Model> GenerateModel(EntityData &entData,LightmapData &lightmapInfo,const std::vector<MaterialHandle> &materials);
		std::vector<std::shared_ptr<CollisionMesh>> GeneratePhysics(asset::EntityData &entData,const std::vector<std::shared_ptr<vmf::PolyMesh>> &polyMeshes,const std::vector<MaterialHandle> &materials);

		std::string m_path;
		std::unique_ptr<bsp::File> m_bsp = nullptr;

		std::function<void(const std::string&)> m_messageLogger = nullptr;
		std::unordered_set<std::string> m_msgCache;
		std::unordered_map<pragma::asset::EntityData*,std::shared_ptr<BSPEntityData>> m_bspEntityData {};

		std::vector<util::fgd::Data> m_fgdData = {};
		std::shared_ptr<pragma::asset::WorldData> m_outputWorldData = nullptr;
		Game &m_game;
		NetworkState &m_nw;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::asset::vbsp::FaceLightMapInfo::Flags)

#endif
