#ifndef __C_NORMALMESH_H__
#define __C_NORMALMESH_H__

#include "pragma/clientdefinitions.h"
#include <mathutil/glmutil.h>
#include <vector>
#include <pragma/model/vertex.h>

class DLLCLIENT NormalMesh
{
protected:
	void ComputeTangentBasis(std::vector<Vertex> &verts,std::vector<uint16_t> &triangles);
public:
	NormalMesh();
};

#endif