// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.policies.core;

export import luabind;

export namespace luabind {
	namespace detail {
		struct const_ref_policy {
			template<class T, class Direction>
			struct specialize {
				using type = const_ref_converter;
			};
		};
		struct ref_policy {
			template<class T, class Direction>
			struct specialize {
				using type = ref_converter;
			};
		};
		struct pointer_policy {
			template<class T, class Direction>
			struct specialize {
				using type = pointer_converter;
			};
		};
		struct const_pointer_policy {
			template<class T, class Direction>
			struct specialize {
				using type = const_pointer_converter;
			};
		};
		struct value_policy {
			template<class T, class Direction>
			struct specialize {
				using type = value_converter;
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
