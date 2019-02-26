#ifndef __MDL_POSE_PARAMETER_H__
#define __MDL_POSE_PARAMETER_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>
#include "mdl_shared.h"

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudioposeparamdesc_t
		{
			int32_t sznameindex;
			int32_t flags;
			float start;
			float end;
			float loop;
		};

		class PoseParameter
		{
		private:
			std::string m_name;
			float m_start = 0.f;
			float m_end = 0.f;
			float m_loop = 0.f;
		public:
			PoseParameter(const VFilePtr &f);
			const std::string &GetName() const;
			float GetStart() const;
			float GetEnd() const;
			float GetLoop() const;

			void DebugPrint(const std::string &t="");
		};
	};
};
#pragma pack(pop)

#endif
