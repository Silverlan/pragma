/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __INTEL_VTUNE_HPP__
#define __INTEL_VTUNE_HPP__

// #define PRAGMA_ENABLE_VTUNE_PROFILING

#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
#include "pragma/networkdefinitions.h"

struct ___itt_domain;
struct ___itt_string_handle;
namespace debug
{
	struct DLLNETWORK VTuneTask
	{
		void Begin();
		void End();
		VTuneTask(___itt_domain *domain,___itt_string_handle *handle)
			: domain{domain},handle{handle}
		{}
		___itt_domain *domain;
		___itt_string_handle *handle;
	};
	class DLLNETWORK VTuneDomain
	{
	public:
		VTuneDomain();
		~VTuneDomain();
		VTuneTask BeginTask(const std::string &name);
		void EndTask();
	private:
		___itt_domain *m_domain = nullptr;
	};

	VTuneDomain &open_domain();
	DLLNETWORK VTuneDomain &get_domain();
	void close_domain();
};
#endif

#endif
