// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/lua/luaapi.h"
#include "mathutil/uvec.h"

export module pragma.shared:scripting.lua.entity_iterator;

export import :entities.iterator;

export {
	class LuaBaseEntityIterator {
	public:
		LuaBaseEntityIterator(const BaseEntityIterator &iterator);
		LuaBaseEntityIterator(const LuaBaseEntityIterator &other);
		LuaBaseEntityIterator &operator=(const LuaBaseEntityIterator &other) = delete;
		LuaBaseEntityIterator &operator++();
		LuaBaseEntityIterator operator++(int);
		luabind::object operator*();
		luabind::object operator->();
		bool operator==(const LuaBaseEntityIterator &other);
		bool operator!=(const LuaBaseEntityIterator &other);
	private:
		BaseEntityIterator m_iterator;
	};

	struct LuaEntityIteratorFilterBase {
		virtual void Attach(EntityIterator &iterator) = 0;
	};

	class LuaEntityIterator {
	public:
		LuaEntityIterator(lua_State *l, EntityIterator::FilterFlags filterFlags = EntityIterator::FilterFlags::Default);
		LuaEntityIterator(lua_State *l, pragma::ComponentId componentId, EntityIterator::FilterFlags filterFlags = EntityIterator::FilterFlags::Default);
		LuaEntityIterator(lua_State *l, const std::string &componentName, EntityIterator::FilterFlags filterFlags = EntityIterator::FilterFlags::Default);
		LuaBaseEntityIterator begin() const;
		LuaBaseEntityIterator end() const;
		void AttachFilter(LuaEntityIteratorFilterBase &filter);

		EntityIterator &GetIterator();
	private:
		std::shared_ptr<EntityIterator> m_iterator = nullptr;
	};

	struct LuaEntityIteratorFilterName : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterName(const std::string &name, bool caseSensitive = false, bool exactMatch = true);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		std::string m_name;
		bool m_bCaseSensitive = false;
		bool m_bExactMatch = true;
	};

	struct LuaEntityIteratorFilterModel : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterModel(const std::string &mdlName);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		std::string m_modelName;
	};

	struct LuaEntityIteratorFilterUuid : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterUuid(const std::string &uuid);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		std::string m_uuid;
	};

	struct LuaEntityIteratorFilterClass : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterClass(const std::string &className, bool caseSensitive = false, bool exactMatch = true);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		std::string m_className;
		bool m_bCaseSensitive = false;
		bool m_bExactMatch = true;
	};

	struct LuaEntityIteratorFilterNameOrClass : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterNameOrClass(const std::string &name, bool caseSensitive = false, bool exactMatch = true);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		std::string m_name;
		bool m_bCaseSensitive = false;
		bool m_bExactMatch = true;
	};

	struct LuaEntityIteratorFilterEntity : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterEntity(const std::string &name);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		std::string m_name;
	};

	struct LuaEntityIteratorFilterSphere : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterSphere(const Vector3 &origin, float radius);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		Vector3 m_origin;
		float m_radius = 0.f;
	};

	struct LuaEntityIteratorFilterBox : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterBox(const Vector3 &min, const Vector3 &max);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		Vector3 m_min;
		Vector3 m_max;
	};

	struct LuaEntityIteratorFilterCone : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterCone(const Vector3 &origin, const Vector3 &dir, float radius, float angle);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		Vector3 m_origin;
		Vector3 m_direction;
		float m_radius = 0.f;
		float m_angle = 0.f;
	};

	struct LuaEntityIteratorFilterComponent : public LuaEntityIteratorFilterBase {
		LuaEntityIteratorFilterComponent(luabind::object);
		LuaEntityIteratorFilterComponent(pragma::ComponentId componentId);
		LuaEntityIteratorFilterComponent(lua_State *l, const std::string &componentName);
		virtual void Attach(EntityIterator &iterator) override;
	private:
		pragma::ComponentId m_componentId = pragma::INVALID_COMPONENT_ID;
	};

	///////////////

	class LuaBaseEntityComponentIterator {
	public:
		LuaBaseEntityComponentIterator(const BaseEntityIterator &iterator);
		LuaBaseEntityComponentIterator(const LuaBaseEntityComponentIterator &other);
		LuaBaseEntityComponentIterator &operator=(const LuaBaseEntityComponentIterator &other) = delete;
		LuaBaseEntityComponentIterator &operator++();
		LuaBaseEntityComponentIterator operator++(int);
		std::pair<BaseEntity *, pragma::BaseEntityComponent *> operator*();
		std::pair<BaseEntity *, pragma::BaseEntityComponent *> operator->();
		bool operator==(const LuaBaseEntityComponentIterator &other);
		bool operator!=(const LuaBaseEntityComponentIterator &other);
	private:
		BaseEntityIterator m_iterator;
	};

	class CEntityComponentIterator : public EntityIterator {
	public:
		CEntityComponentIterator() = default;
		CEntityComponentIterator(Game &game, FilterFlags filterFlags = FilterFlags::Default);
		CEntityComponentIterator(Game &game, pragma::ComponentId componentId, FilterFlags filterFlags = FilterFlags::Default);
		CEntityComponentIterator(Game &game, const std::string &componentName, FilterFlags filterFlags = FilterFlags::Default);
		CEntityComponentIterator(std::vector<BaseEntity *> &ents);
		CEntityComponentIterator &operator++();
		CEntityComponentIterator operator++(int);
		pragma::BaseEntityComponent &operator*();
		pragma::BaseEntityComponent *operator->();
		bool operator==(const CEntityComponentIterator &other);
		bool operator!=(const CEntityComponentIterator &other);
	protected:
		std::size_t m_currentIndex = std::numeric_limits<std::size_t>::max(); // Note: Intentional overflow at first iteration
		pragma::ComponentId m_componentId = pragma::INVALID_COMPONENT_ID;
		std::vector<BaseEntity *> *m_ents = nullptr;
	};

	class LuaEntityComponentIterator {
	public:
		LuaEntityComponentIterator(lua_State *l, pragma::ComponentId componentId, EntityIterator::FilterFlags filterFlags = EntityIterator::FilterFlags::Default);
		LuaEntityComponentIterator(lua_State *l, const std::string &componentName, EntityIterator::FilterFlags filterFlags = EntityIterator::FilterFlags::Default);
		LuaBaseEntityComponentIterator begin() const;
		LuaBaseEntityComponentIterator end() const;
		void AttachFilter(LuaEntityIteratorFilterBase &filter);

		EntityIterator &GetIterator();
	private:
		std::shared_ptr<CEntityComponentIterator> m_iterator = nullptr;
	};
}
