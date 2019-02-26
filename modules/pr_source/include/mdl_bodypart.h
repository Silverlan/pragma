#ifndef __MDL_BODYPART_H__
#define __MDL_BODYPART_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>
#include "mdl_shared.h"

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudiobodyparts_t
		{
			int32_t sznameindex;
			int32_t nummodels;
			int32_t base;
			int32_t modelindex;
		};

		struct mstudio_modelvertexdata_t
		{
			// base of external vertex data stores
			int32_t pVertexData; // Pointer
			int32_t pTangentData; // Pointer
		};

		struct mstudiomodel_t
		{
			std::array<int8_t,64> name;

			int32_t type;

			float boundingradius;

			int32_t nummeshes;	
			int32_t meshindex;

			// cache purposes
			int32_t numvertices; // number of unique vertices/normals/texcoords
			int32_t vertexindex; // vertex Vector
			int32_t tangentsindex; // tangents Vector

			int32_t numattachments;
			int32_t attachmentindex;

			int32_t numeyeballs;
			int32_t eyeballindex;

			mstudio_modelvertexdata_t vertexdata;

			std::array<int32_t,8> unused; // remove as appropriate
		};

		struct mstudio_meshvertexdata_t
		{
			// indirection to this mesh's model's vertex data
			int32_t modelvertexdata; // Pointer to mstudio_modelvertexdata_t

			// used for fixup calcs when culling top level lods
			// expected number of mesh verts at desired lod
			std::array<int32_t,MAX_NUM_LODS> numLODVertexes;
		};

		struct mstudiomesh_t
		{
			int32_t material;

			int32_t modelindex;

			int32_t numvertices; // number of unique vertices/normals/texcoords
			int32_t vertexoffset; // vertex mstudiovertex_t

			int32_t numflexes; // vertex animation
			int32_t flexindex;

			// special codes for material operations
			int32_t materialtype;
			int32_t materialparam;

			// a unique ordinal for this mesh
			int32_t meshid;

			Vector3 center;

			mstudio_meshvertexdata_t vertexdata;

			std::array<int32_t,8> unused; // remove as appropriate
		};

		struct mstudioflex_t
		{
			int32_t flexdesc; // input value

			float target0; // zero
			float target1; // one
			float target2; // one
			float target3; // zero

			int32_t numverts;
			int32_t vertindex;

			int32_t flexpair; // second flex desc
			uint8_t vertanimtype; // See StudioVertAnimType_t
			std::array<uint8_t,3> unusedchar;
			std::array<int32_t,6> unused;
		};

		struct mstudiovertanim_t
		{
		public:
			uint16_t index;
			uint8_t speed; // 255/max_length_in_flex
			uint8_t side; // 255/left_right

			// JasonM changing this type a lot, to prefer fixed point 16 bit...
			union
			{
				std::array<int16_t,3> delta;
				std::array<uint16_t,3> flDelta;
			};
	
			union
			{
				std::array<int16_t,3> ndelta;
				std::array<uint16_t,3> flNDelta;
			};
		public:
			mstudiovertanim_t()=default;
			mstudiovertanim_t(const mstudiovertanim_t &vOther)=delete;
		};

		struct mstudiovertanim_wrinkle_t : public mstudiovertanim_t
		{
			int16_t wrinkledelta;
		};

		enum class StudioVertAnimType_t : uint32_t
		{
			STUDIO_VERT_ANIM_NORMAL = 0,
			STUDIO_VERT_ANIM_WRINKLE,
		};

		class BodyPart
		{
		public:
			struct Model
			{
				struct Mesh
				{
					mstudiomesh_t stdMesh;
					struct Flex
					{
						mstudioflex_t stdFlex;
						std::vector<std::shared_ptr<mstudiovertanim_t>> vertAnim;
					};
					std::vector<Flex> flexes;
				};
				mstudiomodel_t stdMdl;
				std::vector<Mesh> meshes;
				uint32_t vertexCount = 0;
			};
		private:
			std::string m_name;
			std::vector<Model> m_models;

			void ReadFlexes(const VFilePtr &f,uint64_t offset,Model::Mesh &mesh);
			void ReadVertAnims(const VFilePtr &f,uint64_t offset,Model::Mesh::Flex &flex);
		public:
			BodyPart(const VFilePtr &f);
			const std::string &GetName() const;
			const std::vector<Model> &GetModels() const;

			void DebugPrint(const std::string &t="");
		};
	};
};
#pragma pack(pop)

#endif
