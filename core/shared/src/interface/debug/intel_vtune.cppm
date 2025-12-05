// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:debug.intel_vtune;

export import std;

export {
#ifdef PRAGMA_ENABLE_VTUNE_PROFILING
	struct ___itt_domain;
	struct ___itt_string_handle;
	namespace pragma::debug {
		struct DLLNETWORK VTuneTask {
			void Begin();
			void End();
			VTuneTask(___itt_domain *domain, ___itt_string_handle *handle) : domain {domain}, handle {handle} {}
			___itt_domain *domain;
			___itt_string_handle *handle;
		};
		class DLLNETWORK VTuneDomain {
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
};
