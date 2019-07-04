#ifndef __S_POLYMESH_H__
#define __S_POLYMESH_H__
#include "pragma/serverdefinitions.h"
#include "pragma/model/polymesh.h"

class DLLSERVER SPolyMesh
	: public PolyMesh
{
	friend Con::c_cout& operator<<(Con::c_cout&,const SPolyMesh&);
private:
public:
	static void GenerateBrushMeshes(pragma::physics::IEnvironment &env,std::vector<std::shared_ptr<BrushMesh>> &outBrushMeshes,const std::vector<std::shared_ptr<SPolyMesh>> &meshes);
};

inline Con::c_cout &operator<<(Con::c_cout &os,const SPolyMesh& mesh)
{
	os<<static_cast<PolyMesh>(mesh);
	return os;
}


inline void SPolyMesh::GenerateBrushMeshes(pragma::physics::IEnvironment &env,std::vector<std::shared_ptr<BrushMesh>> &outBrushMeshes,const std::vector<std::shared_ptr<SPolyMesh>> &meshes)
{
	return PolyMesh::GenerateBrushMeshes<Side,SPolyMesh,BrushMesh>(env,outBrushMeshes,meshes);
}

#endif