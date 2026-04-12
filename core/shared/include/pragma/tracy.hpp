// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_TRACY_HPP__
#define __PRAGMA_TRACY_HPP__

#ifdef PRAGMA_WITH_TRACY_PROFILING

#include <tracy/Tracy.hpp>

#define TracyPlotRSS() \
	TracyPlot("RSS", static_cast<int64_t>(pragma::get_current_rss()));

#endif

#endif
