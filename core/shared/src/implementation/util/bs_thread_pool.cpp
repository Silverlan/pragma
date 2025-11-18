// SPDX-FileCopyrightText: (c) 2025 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.util;

import BS.thread_pool;

// msvc fix for unresolved external symbols
thread_local std::optional<std::size_t> BS::this_thread::my_index = std::nullopt;
thread_local std::optional<void *> BS::this_thread::my_pool = std::nullopt;
