#ifndef __MDL_BONE_H__
#define __MDL_BONE_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudiobone_t
		{
			int32_t sznameindex;
			int32_t parent; // parent bone
			std::array<int32_t,6> bonecontroller; // bone controller index, -1 == none

			// default values
			Vector3 pos;
			Quat quat;
			Vector3 rot;
			// compression scale
			Vector3 posscale;
			Vector3 rotscale;

			Mat3x4 poseToBone;
			Quat qAlignment;
			int32_t flags;
			int32_t proctype;
			int32_t procindex; // procedural rule
			int32_t physicsbone; // index into physically simulated bone
			int32_t surfacepropidx; // index into string tablefor property name
			int32_t contents; // See BSPFlags.h for the contents flags

			int32_t surfacepropLookup;
			std::array<int32_t,7> unused; // remove as appropriate
		};
		class Bone
		{
		private:
			uint32_t m_id;
			Vector3 m_pos;
			Quat m_rot;
			Vector3 m_angles;
			int32_t m_flags;
			int32_t m_parentId;
			Vector3 m_posScale;
			Vector3 m_rotScale;
			Mat3x4 m_poseToBone;
			std::vector<std::shared_ptr<Bone>> m_children;
			std::shared_ptr<Bone> m_parent;
			std::string m_name;
			std::string m_surfaceProp;
		public:
			Bone(uint32_t id,const VFilePtr &f);
			const Vector3 &GetPos() const;
			const Quat &GetRot() const;
			const Vector3 &GetAngles() const;
			void SetPos(const Vector3 &pos);
			void SetRot(const Quat &rot);
			int32_t GetFlags() const;
			uint32_t GetID() const;
			const Mat3x4 GetPoseToBone() const;
			const Vector3 &GetPosScale() const;
			const Vector3 &GetRotScale() const;
			const std::string &GetName() const;
			const std::string &GetSurfaceProp() const;
			const std::shared_ptr<Bone> &GetParent() const;
			const std::vector<std::shared_ptr<Bone>> &GetChildren() const;

			void DebugPrint(const std::string &t="");
			static void BuildHierarchy(std::vector<std::shared_ptr<Bone>> &bones);
			static void TransformLocalTransformsToGlobal(const std::vector<std::shared_ptr<Bone>> &bones);
		};
	};
};
#pragma pack(pop)

#endif
