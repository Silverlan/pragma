#if 0
#ifndef __DMX_COMMON_HPP__
#define __DMX_COMMON_HPP__

#include <vector>
#include <string>
#include <mathutil/uvec.h>

namespace dmx
{
	// A "DAG" is a node in a Directed acyclic graph, constructing the scene hierarchy.
	// It appears here in order to transform its child objects.
	// It can take the place of a DmeMesh, DmeJoint or DmeAttachment at any time.
	class DmeDag
	{
		DmeTransform	transform;	// used by SFM to store the *current* position of the dag.
									// Studiomdl only reads this value if nothing is found in DmeModel::baseStates::DmeTransformList.
		bool			visible;
		std::vector<DmeDag>			children; // child nodes of the dag. never seen used (Compatibility with DmeModel?)
	
		// One of the following:
		std::string		name; // of a DmeJoint
		DmeMesh		shape;
		DmeAttachment	shape;
	};

	class DmeMesh : DmeDag
	{
		bool				visible;

		void				bindState;		// only seen empty
		DmeVertexData		currentState;	// pointer to default baseState		
		std::vector<DmeVertexData> baseStates;	// only ever seen with one value	
		std::vector<DmeVertexDeltaData> deltaStates;	// flex shapes	
		std::vector<DmeFaceSet> faceSets;

		std::vector<Vector2> deltaStateWeights;		// unknown
		std::vector<Vector2> deltaStateWeightsLagged;	// unknown
	};

	class DmeJoint : DmeDag // a bone
	{
		DmeTransform	transform;	// used by SFM to store the *current* position of the bone.
									// Studiomdl only reads this value if nothing is found in DmeModel::baseStates::DmeTransformList.
		void			shape; // only seen empty
		bool			visible;
		std::vector<DmeJoint>		children;
		bool			lockInfluenceWeights;
	};

	class DmeTransformList
	{
		DmeTransform transforms[]; // contains one element for each bone and mesh. Elements are matched according to name.
	};
	
	class DmeTransform
	{
		Vector3 position;
		Quat orientation;
	};
};

#endif

#endif