#ifndef __MDL_FLEXDESC_H__
#define __MDL_FLEXDESC_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudioflexdesc_t
		{
			int32_t szFACSindex;
		};
		class FlexDesc
		{
		public:
			FlexDesc(const VFilePtr &f);
			const std::string &GetName() const;
		private:
			std::string m_name;
		};
	};
};
#pragma pack(pop)

#endif
