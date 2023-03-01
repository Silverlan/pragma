/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __CORE_POLICIES_HPP__
#define __CORE_POLICIES_HPP__

#include <luabind/detail/conversion_policies/conversion_policies.hpp>

namespace luabind {
	namespace detail {
		struct const_ref_policy {
			template<class T, class Direction>
			struct specialize {
				using type = luabind::detail::const_ref_converter;
			};
		};
		struct ref_policy {
			template<class T, class Direction>
			struct specialize {
				using type = luabind::detail::ref_converter;
			};
		};
		struct pointer_policy {
			template<class T, class Direction>
			struct specialize {
				using type = luabind::detail::pointer_converter;
			};
		};
		struct const_pointer_policy {
			template<class T, class Direction>
			struct specialize {
				using type = luabind::detail::const_pointer_converter;
			};
		};
		struct value_policy {
			template<class T, class Direction>
			struct specialize {
				using type = luabind::detail::value_converter;
			};
		};
	};
	template<unsigned int N>
	using const_ref_policy = meta::type_list<converter_policy_injector<N, detail::const_ref_policy>>;

	template<unsigned int N>
	using ref_policy = meta::type_list<converter_policy_injector<N, detail::ref_policy>>;

	template<unsigned int N>
	using pointer_policy = meta::type_list<converter_policy_injector<N, detail::pointer_policy>>;

	template<unsigned int N>
	using const_pointer_policy = meta::type_list<converter_policy_injector<N, detail::const_pointer_policy>>;

	template<unsigned int N>
	using value_policy = meta::type_list<converter_policy_injector<N, detail::value_policy>>;
};

#endif
