#ifndef __MDL_ATTACHMENT_H__
#define __MDL_ATTACHMENT_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudioattachment_t
		{
			int32_t sznameindex;
			uint32_t flags;
			int32_t localbone;
			Mat3x4 local; // attachment point
			std::array<int32_t,8> unused;
		};
		class Attachment
		{
		private:
			uint32_t m_flags;
			int32_t m_bone;
			Mat3x4 m_transform;
			std::string m_name;
		public:
			Attachment(const VFilePtr &f);
			uint32_t GetFlags() const;
			int32_t GetBone() const;
			const Mat3x4 &GetTransform() const;
			const std::string &GetName() const;

			void DebugPrint(const std::string &t="");
		};
	};
};
#pragma pack(pop)

#endif
