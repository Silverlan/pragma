#ifndef __MDL_FLEXCONTROLLER_H__
#define __MDL_FLEXCONTROLLER_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudioflexcontroller_t
		{
			int32_t sztypeindex;
			int32_t sznameindex;
			mutable int32_t localToGlobal;	// remapped at load time to master list
			float min;
			float max;
		};
		class FlexController
		{
		public:
			FlexController(const VFilePtr &f);
			const std::string &GetName() const;
			const std::string &GetType() const;
			std::pair<float,float> GetRange() const;
		private:
			std::string m_name;
			std::string m_type;
			float m_min = 0.f;
			float m_max = 0.f;
		};

		struct mstudioflexcontrollerui_t
		{
			int32_t sznameindex;

			int32_t szindex0;
			int32_t szindex1;
			int32_t szindex2;

			uint8_t remaptype;	// See the FlexControllerRemapType_t enum
			bool stereo;		// Is this a stereo control?
			std::array<uint8_t,2> unused;
		};

		class FlexControllerUi
		{
		public:
			FlexControllerUi(const VFilePtr &f);
			const std::string &GetName() const;
			bool IsStereo() const;
			uint8_t GetRemapType() const;
			int32_t GetConfig0() const;
			int32_t GetConfig1() const;
			int32_t GetConfig2() const;
		private:
			std::string m_name;
			std::array<int32_t,3> m_szIndices;
			uint8_t m_remapType = 0;
			bool m_stereo = false;
		};
	};
};
#pragma pack(pop)

#endif
