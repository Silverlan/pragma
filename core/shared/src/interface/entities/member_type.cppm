// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:entities.member_type;

export import :entities.enums;
export import :entities.universal_reference;

export {
	namespace pragma::ents {
		using Element = udm::PProperty;

		template<class T>
		struct tag_t {
			using type = T;
		};
		template<class T>
		constexpr tag_t<T> tag = {};
#pragma warning(push)
#pragma warning(disable : 4715)
#ifdef __linux__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#endif
		constexpr std::variant<tag_t<EntityURef>, tag_t<MultiEntityURef>, tag_t<EntityUComponentMemberRef>, tag_t<Element>> get_managed_member_type_tag(EntityMemberType e)
		{
			switch(e) {
			case EntityMemberType::Entity:
				return tag<EntityURef>;
			case EntityMemberType::MultiEntity:
				return tag<MultiEntityURef>;
			case EntityMemberType::ComponentProperty:
				return tag<EntityUComponentMemberRef>;
			case EntityMemberType::Element:
				return tag<Element>;
			}
			std::unreachable();
		}

		template<bool ENABLE_DEFAULT_RETURN = true, typename T>
		constexpr decltype(auto) visit_member(EntityMemberType type, T vs)
		{
			if(is_managed_member_type_f(type))
				return std::visit(vs, get_managed_member_type_tag(type));
			else
				return udm::visit<true, true, true, ENABLE_DEFAULT_RETURN>(member_type_to_udm_type(type), vs);
		}

#pragma warning(pop)
#ifdef __linux__
#pragma GCC diagnostic pop
#endif

		template<typename T>
		concept is_managed_member_type = std::is_same_v<T, EntityURef> || std::is_same_v<T, MultiEntityURef> || std::is_same_v<T, EntityUComponentMemberRef> || std::is_same_v<T, Element>;
	};

	namespace pragma {
		namespace ents {
			template<typename T>
			constexpr EntityMemberType member_type_to_enum()
			{
				if constexpr(is_managed_member_type<T>) {
					if constexpr(std::is_same_v<T, EntityURef>)
						return EntityMemberType::Entity;
					if constexpr(std::is_same_v<T, MultiEntityURef>)
						return EntityMemberType::MultiEntity;
					if constexpr(std::is_same_v<T, EntityUComponentMemberRef>)
						return EntityMemberType::ComponentProperty;
					if constexpr(std::is_same_v<T, Element>)
						return EntityMemberType::Element;
				}
				else
					return static_cast<EntityMemberType>(udm::type_to_enum<T>());
			}
		};

		template<typename T>
		concept is_valid_component_property_type_v = is_valid_component_property_type(ents::member_type_to_enum<T>());
	};
};
