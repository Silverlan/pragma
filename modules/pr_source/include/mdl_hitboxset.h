#ifndef __MDL_HITBOXSET_H__
#define __MDL_HITBOXSET_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudiohitboxset_t
		{
			int32_t sznameindex;
			int32_t numhitboxes;
			int32_t hitboxindex;
		};

		struct mstudiobbox_t
		{
			int32_t bone;
			int32_t group;
			Vector3 bbmin;
			Vector3 bbmax;
			int32_t szhitboxnameindex;
			std::array<int32_t,8> unused;
		};

		class HitboxSet
		{
		public:
			struct Hitbox
			{
				std::string name;
				int32_t boneId;
				int32_t groupId;
				std::pair<Vector3,Vector3> boundingBox;
			};
		private:
			std::string m_name;
			std::vector<Hitbox> m_hitboxes;
		public:
			HitboxSet(const VFilePtr &f);
			const std::string &GetName() const;
			const std::vector<Hitbox> &GetHitboxes() const;

			void DebugPrint(const std::string &t="");
		};
	};
};
#pragma pack(pop)

#endif
