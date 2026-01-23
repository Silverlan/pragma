// SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#ifndef __PRAGMA_OSTREAM_OPERATOR_ALIAS_HPP__
#define __PRAGMA_OSTREAM_OPERATOR_ALIAS_HPP__

// Workaround for some Linux compilers not being able to locate the << operator in the global namespace

#define DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(ns, cl)                                                                                                                                                                                                                                          \
	namespace ns {                                                                                                                                                                                                                                                                               \
		static inline std::ostream &operator<<(std::ostream &out, const cl &o) { return ::operator<<(out, o); }                                                                                                                                                                                  \
	};

#endif
