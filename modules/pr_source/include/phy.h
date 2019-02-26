#ifndef __PHY_H__
#define __PHY_H__

#include <string>
#include <vector>
#include <mathutil/uvec.h>
#include <pragma/model/vertex.h>
#include <array>
#include <unordered_map>
#include <fsys/filesystem.h>
#include "mdl_shared.h"

#define MAX_NUM_LODS 8

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{ 
		namespace phy
		{
			typedef struct phyheader_s
			{
				int32_t size;
				int32_t id;
				int32_t solidCount;
				int32_t checkSum; // checksum of source .mdl file
			} phyheader_t;

			struct compactsurfaceheader_t
			{
				int32_t	size;
				int32_t	vphysicsID;
				int16_t version;
				int16_t modelType;
				int32_t	surfaceSize;
				Vector3	dragAxisAreas;
				int32_t	axisMapSize;
			};

			struct trianglefaceheader_t
			{
				int32_t m_offsetTovertices;
				std::array<int32_t,2> dummy;
				int32_t m_countFaces;
			};

			struct triangleface_t
			{
				uint8_t id;
				std::array<uint8_t,3> _dummy;
				uint8_t v1;
				std::array<uint8_t,3> _dummy2;
				uint8_t v2;
				std::array<uint8_t,3> _dummy3;
				uint8_t v3;
				std::array<uint8_t,3> _dummy4;
			};

			struct legacysurfaceheader_t
			{
				Vector3 m_vecMassCenter;
				Vector3 m_vecRotationInertia;
				float m_flUpperLimitRadius;
				int32_t m_volumeFull;
				std::array<int32_t,4> dummy;
			};

			struct VectorStrange
			{
				Vector3 v;
				int32_t unknown;
			};

			struct PhyFaceSection
			{
				int32_t boneIdx;
				std::vector<std::array<uint16_t,3>> faces;
			};

			struct PhyKeyValues
			{
				std::string name;
				std::string parent;
				std::string surfaceProp;
				float mass = 0.f;
				float damping = 0.f;
				float rotDamping = 0.f;
				float drag = 0.f;
				float inertia = 0.f;
				float volume = 0.f;
				float massBias = 0.f;
			};

			struct PhyRagdollConstraint
			{
				int32_t parentIndex = -1;
				int32_t childIndex = -1;

				float xmin = 0.f;
				float xmax = 0.f;
				float xfriction = 0.f;

				float ymin = 0.f;
				float ymax = 0.f;
				float yfriction = 0.f;

				float zmin = 0.f;
				float zmax = 0.f;
				float zfriction = 0.f;
			};

			struct PhyCollisionData
			{
				std::vector<PhyFaceSection> faceSections;
				std::vector<std::pair<Vector3,Vector3>> vertices; // Position, Normal
				PhyKeyValues keyValues;
				bool sourcePhyIsCollision = false;
			};
		};
		bool load_phy(const VFilePtr &f,std::vector<phy::PhyCollisionData> &collisionObjects,std::unordered_map<int32_t,phy::PhyRagdollConstraint> &constraints);
	};
};
#pragma pack(pop)

#endif
