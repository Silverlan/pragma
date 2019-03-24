#ifndef __PR_DDS_HPP__
#define __PR_DDS_HPP__

#include <sharedutils/util_library.hpp>

#ifndef DLLSPEC_IDDS
#define DLLSPEC_IDDS
#endif

struct DLLSPEC_IDDS IDDS final
{
	IDDS(util::Library &lib)
		: initialize{lib.FindSymbolAddress<decltype(initialize)>("pr_dds_initialize")}
	{
		m_bValid = initialize != nullptr &&
			get_build_id != nullptr &&
			shutdown != nullptr;
	}
	IDDS()=default;
	bool(* const initialize)() = nullptr;

	bool valid() const {return m_bValid;}
private:
	bool m_bValid = false;
};

#endif
