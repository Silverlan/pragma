#if 0
#ifndef __DMX_MESH_HPP__
#define __DMX_MESH_HPP__

#include <vector>
#include <mathutil/uvec.h>
#include "dmx_common.hpp"

namespace dmx
{
	class DmeModel // one per file
	{
		bool				visible;
		std::vector<DmeDag> children;
		std::vector<DmeJoint> jointList;
		std::vector<DmeTransformList> baseStates;	// defines bone and mesh positions; only ever seen with one value	
	};

	class DmeModelRoot
	{
		// model and skeleton should point to the same object
		DmeModel	model;
		DmeModel	skeleton;

		DmeCombinationOperator	combinationOperator; // flex controllers; optional
	};

	class DmeAttachment : DmeDag // an attachment
	{
		bool		visible;
		bool		isRigid; // apparently obsolete?
		bool		isWorldAligned; // does not move with parent bone, but is still positioned relative to it
	};

	class DmeVertexData // mesh data
	{
		std::vector<CUtlString> vertexFormat;		// { positions, normals, textureCoordinates, [jointWeights, jointIndices, balance] }
		int			jointCount;			// most bones any one vert is weighted to; max 3 (studiomdl errors on compile otherwise)
		bool		flipVCoordinates;	// left-handed to right-handed?

		// The first array contains one entry per vertex.
		// The second "Indices" array contains one entry one entry per vertex /per face/.
	
		std::vector<Vector3> positions;
		std::vector<int> positionsIndices;

		std::vector<Vector3> normals;
		std::vector<int> normalsIndices;

		std::vector<Vector2> textureCoordinates;
		std::vector<int> textureCoordinatesIndices;

		// Flex controller stereo split; optional
		std::vector<float> balance;			// 0 = 100% right, 1 = 100% left.
		std::vector<int> balanceIndices;
	
		// Weightmapping; optional. The size of BOTH arrays is equal to ( sizeof(positions) * jointCount )
		std::vector<float> jointWeights;		// weight
		std::vector<int> jointIndices;	// index in DmeModel::jointList (v15+) or DmeModel::jointTransforms (v1-14)
	};

	class DmeFaceSet // defines a set of faces with a given material
	{
		DmeMaterial	material; // the material these faces are drawn with
		std::vector<int> faces; // the indices of the vertices that make up each face, delimited by -1. Quads and *convex* n-gons allowed.
	};

	class DmeMaterial // a material
	{
		CUtlString	mtlName; // path relative to \game\materials\, no extension
	};

	class DmeVertexDeltaData // a shape key
	{
		std::vector<CUtlString>	vertexFormat; // positions, normals, [wrinkle]
		bool		flipVCoordinates; // unknown
		bool		corrected; // unknown

		std::vector<Vector3>		positions; // offset: (shape position) - (mesh position)
		std::vector<int>			positionsIndices; // index in DmeMesh::currentState::positions

		std::vector<Vector3>		normals; // offset: (shape normal) - (mesh normal). For corrective shapes "base" is the mesh plus target shapes.
		std::vector<int>			normalsIndices; // index in DmeMesh::currentState::normals

		std::vector<float>		wrinkle; // wrinkle scale. +1 means full compress, -1 means full stretch.
		std::vector<int>			wrinkleIndices; // index in DmeMesh::currentState::textureCoordinates
	};
};

#endif
#endif