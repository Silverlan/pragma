// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

export module pragma.shared:scripting.lua.classes.property_entity;

export import :entities.property;
export import :scripting.lua.classes.property_generic;

export {
	// Entity
	class LEntityPropertyWrapper : public LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle> {
	  public:
		LEntityPropertyWrapper(const EntityHandle &v) : LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle> {v} {}
		LEntityPropertyWrapper(const std::shared_ptr<pragma::EntityProperty> &v) : LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle> {v} {}
		LEntityPropertyWrapper() : LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle> {} {}

		pragma::EntityProperty &operator*() { return LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::operator*(); }
		const pragma::EntityProperty &operator*() const { return LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::operator*(); }
		pragma::EntityProperty *operator->() { return LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::operator->(); }
		const pragma::EntityProperty *operator->() const { return LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::operator->(); }

		using LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::LSimplePropertyWrapper;
		using LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::operator*;
		using LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::operator->;
		virtual pragma::EntityProperty &GetProperty() const override { return *static_cast<pragma::EntityProperty *>(prop.get()); }
	};
	using LEntityProperty = LEntityPropertyWrapper;
};
