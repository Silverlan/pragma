// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;


export module pragma.shared:scripting.lua.classes.property_entity;

export {
	// Entity
	class LEntityPropertyWrapper : public LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle> {
	public:
		using LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::LSimplePropertyWrapper;
		using LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::operator*;
		using LSimplePropertyWrapper<pragma::EntityProperty, EntityHandle>::operator->;
		virtual pragma::EntityProperty &GetProperty() const override { return *static_cast<pragma::EntityProperty *>(prop.get()); }

		LEntityPropertyWrapper() : LSimplePropertyWrapper() {}
		LEntityPropertyWrapper(const EntityHandle &v) : LSimplePropertyWrapper(v) {}
	};
	using LEntityProperty = LEntityPropertyWrapper;
};
