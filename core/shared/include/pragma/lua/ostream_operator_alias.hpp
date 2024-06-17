/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __PRAGMA_OSTREAM_OPERATOR_ALIAS_HPP__
#define __PRAGMA_OSTREAM_OPERATOR_ALIAS_HPP__

// Workaround for some Linux compilers not being able to locate the << operator in the global namespace

#define DEFINE_OSTREAM_OPERATOR_NAMESPACE_ALIAS(ns, cl)                                                                                                                                                                                                                                          \
	namespace ns {                                                                                                                                                                                                                                                                               \
		static inline std::ostream &operator<<(std::ostream &out, const cl &o)                                                                                                                                                                                                                   \
		{                                                                                                                                                                                                                                                                                        \
			return ::operator<<(out, o);                                                                                                                                                                                                                                                         \
		}                                                                                                                                                                                                                                                                                        \
	};

#endif
