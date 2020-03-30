#include <pragma/game/game.h>
#include <pragma/math/vector/wvvector3.h>
#include "pragma/model/brush/brushmesh.h"
#include "pragma/game/game_resources.hpp"
#include "pragma/util/util_bsp_tree.hpp"

DLLNETWORK void BuildDisplacementTriangles(std::vector<Vector3> &sideVerts,unsigned int start,
	Vector3 &nu,Vector3 &nv,float sw,float sh,float ou,float ov,float su,float sv,
	unsigned char power,std::vector<std::vector<Vector3>> &normals,std::vector<std::vector<Vector3>> &offsets,std::vector<std::vector<float>> &distances,unsigned char numAlpha,std::vector<std::vector<glm::vec2>> &alphas,
	std::vector<Vector3> &outVertices,std::vector<Vector3> &outNormals,std::vector<Vector2> &outUvs,std::vector<unsigned int> &outTriangles,std::vector<Vector2> *outAlphas=NULL);

DLLNETWORK void ToTriangles(const std::vector<Vector3> &vertices,std::vector<uint16_t> &outTriangles);
DLLNETWORK bool decimate_displacement_geometry(const std::vector<uint16_t> &indices,uint32_t power,std::vector<uint16_t> &outIndices,uint8_t decimateAmount=2);

namespace pragma
{
	namespace level
	{
		DLLNETWORK void load_map_brushes(
			Game &game,uint32_t version,VFilePtr f,BaseEntity *ent,std::vector<Material*> &materials,std::vector<SurfaceMaterial> &surfaceMaterials,const Vector3 &origin
		);
		DLLNETWORK void load_optimized_map_geometry(
			Game &game,uint32_t version,VFilePtr f,BaseEntity *ent,std::vector<Material*> &materials,
			std::vector<SurfaceMaterial> &surfaceMaterials
		);
	};
};
