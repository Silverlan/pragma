// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"
#include "util_enum_flags.hpp"

export module pragma.shared:entities.iterator;

import :entities.base_entity;
export import :entities.enums;
import :game.game;
export import :types;

export {
	namespace pragma {
		class Game;
	}
	struct DLLNETWORK IEntityIteratorFilter {
		IEntityIteratorFilter() = default;
		IEntityIteratorFilter(pragma::Game &game) {}
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) = 0;
	};

#pragma warning(push)
#pragma warning(disable : 4251)
	struct BaseEntityContainer {
		BaseEntityContainer(std::size_t count) : count {count} {}
		virtual ~BaseEntityContainer() {}
		// Returns number of actual (non-NULL) items
		std::size_t Count() const { return count; }
		virtual std::size_t Size() const = 0;
		virtual pragma::ecs::BaseEntity *At(std::size_t index) = 0;
	  private:
		std::size_t count = 0ull;
	};
	struct EntityContainer : public BaseEntityContainer {
		EntityContainer(std::vector<pragma::ecs::BaseEntity *> &ents, std::size_t count) : BaseEntityContainer(count), ents {ents} {}
		// Returns container size (potentially includes NULL elements)
		virtual std::size_t Size() const override;
		virtual pragma::ecs::BaseEntity *At(std::size_t index) override;
	  private:
		std::vector<pragma::ecs::BaseEntity *> &ents;
	};
	struct EntityIteratorData {
		EntityIteratorData(pragma::Game &game);
		EntityIteratorData(pragma::Game &game, const std::vector<pragma::BaseEntityComponent *> &components, std::size_t count);
		bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) const;
		std::size_t GetCount() const;
		pragma::Game &game;
		std::unique_ptr<BaseEntityContainer> entities = nullptr;
		std::vector<std::shared_ptr<IEntityIteratorFilter>> filters = {};
	};

	class DLLNETWORK BaseEntityIterator {
	  public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = pragma::ecs::BaseEntity;
		using difference_type = pragma::ecs::BaseEntity;
		using pointer = pragma::ecs::BaseEntity *;
		using reference = pragma::ecs::BaseEntity &;

		BaseEntityIterator(const std::shared_ptr<EntityIteratorData> &itData, bool bEndIterator);
		BaseEntityIterator(const BaseEntityIterator &other) = default;
		BaseEntityIterator &operator=(const BaseEntityIterator &other) = default;

		BaseEntityIterator &operator++();
		BaseEntityIterator operator++(int);
		pragma::ecs::BaseEntity *operator*();
		pragma::ecs::BaseEntity *operator->();
		bool operator==(const BaseEntityIterator &other);
		bool operator!=(const BaseEntityIterator &other);

		std::size_t GetCount() const;

		// For internal use only!
		EntityIteratorData *GetIteratorData() { return m_iteratorData.get(); }
		std::size_t GetCurrentIndex() const { return m_currentIndex; }
	  protected:
		std::shared_ptr<EntityIteratorData> m_iteratorData;
		std::size_t m_currentIndex = 0ull;
	  private:
		bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) const;
	};

	namespace pragma::ecs {
		class DLLNETWORK EntityIterator {
		  public:
			friend BaseEntityIterator;
			enum class FilterFlags : uint32_t {
				None = 0u,
				Spawned = 1u,
				Pending = Spawned << 1u,                   // All entities that aren't spawned yet
				IncludeShared = Pending << 1u,             // Include shared entities
				IncludeNetworkLocal = IncludeShared << 1u, // Include all entities that are either serverside- or clientside-only

				// Type flags; If none of these are set, ALL types will pass.
				// If at least one of these is set, only the set types will pass!
				Character = IncludeNetworkLocal << 1u,
				Player = Character << 1u,
				Weapon = Player << 1u,
				Vehicle = Weapon << 1u,
				NPC = Vehicle << 1u,
				Physical = NPC << 1u,
				Scripted = Physical << 1u,
				MapEntity = Scripted << 1u,
				//

				HasTransform = MapEntity << 1u,
				HasModel = HasTransform << 1u,

				AnyType = Character | Player | Weapon | Vehicle | NPC | Physical | Scripted | MapEntity,
				Default = Spawned | IncludeShared | IncludeNetworkLocal,
				Any = Spawned | Pending | IncludeShared | IncludeNetworkLocal | AnyType
			};

			EntityIterator(Game &game, FilterFlags filterFlags = FilterFlags::Default);
			EntityIterator(Game &game, ComponentId componentId, FilterFlags filterFlags = FilterFlags::Default);
			EntityIterator(Game &game, const std::string &componentName, FilterFlags filterFlags = FilterFlags::Default);
			EntityIterator(const EntityIterator &) = default;
			EntityIterator(EntityIterator &&) = delete;
			EntityIterator &operator=(const EntityIterator &) = default;
			EntityIterator &operator=(EntityIterator &&) = default;

			BaseEntityIterator begin() const;
			BaseEntityIterator end() const;
			std::size_t GetCount() const;

			template<class TFilter, typename... TARGS>
			void AttachFilter(TARGS... args);

			// Internal use only!
			EntityIteratorData *GetIteratorData() { return m_iteratorData.get(); }
		  protected:
			EntityIterator() = default;
			EntityIterator(Game &game, bool /* dummy */);
			void SetBaseComponentType(ComponentId componentId);
			void SetBaseComponentType(std::type_index typeIndex);
			void SetBaseComponentType(const std::string &componentName);

			std::shared_ptr<EntityIteratorData> m_iteratorData;
		  private:
			template<typename T, typename = int>
			struct HasGetType : std::false_type {};

			template<typename T>
			struct HasGetType<T, decltype(&T::GetType, 0)> : std::true_type {};

			template<typename T>
			typename std::enable_if<HasGetType<T>::value>::type GetIteratorFilterComponentType(std::optional<std::type_index> &typeIndex)
			{
				typeIndex = T::GetType();
			}

			template<typename T>
			typename std::enable_if<!HasGetType<T>::value>::type GetIteratorFilterComponentType(std::optional<std::type_index> &typeIndex)
			{
			}
		};
		using namespace pragma::math::scoped_enum::bitwise;
	}
	REGISTER_ENUM_FLAGS(pragma::ecs::EntityIterator::FilterFlags)

#pragma warning(pop)

	///////////////

#pragma warning(push)
#pragma warning(disable : 4251)
	struct DLLNETWORK EntityIteratorFilterName : public IEntityIteratorFilter {
		EntityIteratorFilterName(pragma::Game &game, const std::string &name, bool caseSensitive = false, bool exactMatch = true);
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		std::string m_name;
		bool m_bCaseSensitive = false;
		bool m_bExactMatch = true;
	};

	struct DLLNETWORK EntityIteratorFilterModel : public IEntityIteratorFilter {
		EntityIteratorFilterModel(pragma::Game &game, const std::string &mdlName);
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		std::string m_modelName;
	};

	struct DLLNETWORK EntityIteratorFilterUuid : public IEntityIteratorFilter {
		EntityIteratorFilterUuid(pragma::Game &game, const pragma::util::Uuid &uuid);
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		pragma::util::Uuid m_uuid;
	};

	struct DLLNETWORK EntityIteratorFilterClass : public IEntityIteratorFilter {
		EntityIteratorFilterClass(pragma::Game &game, const std::string &name, bool caseSensitive = false, bool exactMatch = true);
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		std::string m_name;
		bool m_bCaseSensitive = false;
		bool m_bExactMatch = true;
	};

	struct DLLNETWORK EntityIteratorFilterNameOrClass : public IEntityIteratorFilter {
		EntityIteratorFilterNameOrClass(pragma::Game &game, const std::string &name, bool caseSensitive = false, bool exactMatch = true);
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		std::string m_name;
		bool m_bCaseSensitive = false;
		bool m_bExactMatch = true;
	};

	struct DLLNETWORK EntityIteratorFilterEntity : public IEntityIteratorFilter {
		EntityIteratorFilterEntity(pragma::Game &game, const std::string &name);
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		std::vector<pragma::util::WeakHandle<pragma::BaseFilterComponent>> m_filterEnts;
		pragma::ComponentId m_filterNameComponentId = pragma::INVALID_COMPONENT_ID;
		pragma::ComponentId m_filterClassComponentId = pragma::INVALID_COMPONENT_ID;
		std::string m_name;
	};

	struct DLLNETWORK EntityIteratorFilterFlags : public IEntityIteratorFilter {
		EntityIteratorFilterFlags(pragma::Game &game, pragma::ecs::EntityIterator::FilterFlags flags);
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		pragma::ecs::EntityIterator::FilterFlags m_flags = pragma::ecs::EntityIterator::FilterFlags::None;
	};

	struct DLLNETWORK EntityIteratorFilterComponent : public IEntityIteratorFilter {
		EntityIteratorFilterComponent(pragma::Game &game, pragma::ComponentId componentId);
		EntityIteratorFilterComponent(pragma::Game &game, const std::string &componentName);

		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		pragma::ComponentId m_componentId = pragma::INVALID_COMPONENT_ID;
	};

	struct DLLNETWORK EntityIteratorFilterUser : public IEntityIteratorFilter {
		EntityIteratorFilterUser(pragma::Game &game, const std::function<bool(pragma::ecs::BaseEntity &, std::size_t)> &fUserFilter);

		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		std::function<bool(pragma::ecs::BaseEntity &, std::size_t)> m_fUserFilter = nullptr;
	};

	struct DLLNETWORK EntityIteratorFilterSphere : public IEntityIteratorFilter {
		EntityIteratorFilterSphere(pragma::Game &game, const Vector3 &origin, float radius);

		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  protected:
		bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index, Vector3 &outClosestPointOnEntityBounds, float &outDistToEntity) const;

		Vector3 m_origin;
		float m_radius = 0.f;
	};

	struct DLLNETWORK EntityIteratorFilterBox : public IEntityIteratorFilter {
		EntityIteratorFilterBox(pragma::Game &game, const Vector3 &min, const Vector3 &max);

		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		Vector3 m_min;
		Vector3 m_max;
	};

	struct DLLNETWORK EntityIteratorFilterCone : public EntityIteratorFilterSphere {
		EntityIteratorFilterCone(pragma::Game &game, const Vector3 &origin, const Vector3 &dir, float radius, float angle);

		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override;
	  private:
		Vector3 m_direction;
		float m_angle = 0.f;
	};
#pragma warning(pop)

	template<class TComponent>
	struct TEntityIteratorFilterComponent : public IEntityIteratorFilter {
		using IEntityIteratorFilter::IEntityIteratorFilter;
		static std::type_index GetType() { return std::type_index(typeid(TComponent)); }
		virtual bool ShouldPass(pragma::ecs::BaseEntity &ent, std::size_t index) override { return ent.HasComponent<TComponent>(); }
	};

	struct ComponentContainer : public BaseEntityContainer {
		ComponentContainer(const std::vector<pragma::BaseEntityComponent *> &components, std::size_t count) : BaseEntityContainer(count), components {components} {}
		virtual std::size_t Size() const override;
		virtual pragma::ecs::BaseEntity *At(std::size_t index) override;

		// For internal use only!
		const std::vector<pragma::BaseEntityComponent *> &components;
	};

	template<class TComponent>
	class BaseEntityComponentIterator : public BaseEntityIterator {
	  public:
		using iterator_category = std::forward_iterator_tag;
		using value_type = pragma::ecs::BaseEntity;
		using difference_type = pragma::ecs::BaseEntity;
		using pointer = pragma::ecs::BaseEntity *;
		using reference = pragma::ecs::BaseEntity &;

		BaseEntityComponentIterator(const std::shared_ptr<EntityIteratorData> &itData, bool bEndIterator) : BaseEntityIterator {itData, bEndIterator}, m_components {&static_cast<ComponentContainer *>(m_iteratorData->entities.get())->components} {}
		BaseEntityComponentIterator(const BaseEntityIterator &other) : BaseEntityIterator {other} {}
		BaseEntityComponentIterator(const BaseEntityComponentIterator<TComponent> &other) : BaseEntityIterator {other}, m_components {other.m_components} {}
		BaseEntityComponentIterator<TComponent> &operator=(const BaseEntityIterator &other)
		{
			BaseEntityIterator::operator=(other);
			return *this;
		}
		BaseEntityComponentIterator<TComponent> &operator=(const BaseEntityComponentIterator<TComponent> &other)
		{
			BaseEntityIterator::operator=(other);
			m_components = other.m_components;
			return *this;
		}

		TComponent &operator*() { return static_cast<TComponent &>(*(*m_components)[m_currentIndex]); }
		TComponent *operator->() { return static_cast<TComponent *>((*m_components)[m_currentIndex]); }
		bool operator==(const BaseEntityComponentIterator<TComponent> &other) { return BaseEntityIterator::operator==(other); }
		bool operator!=(const BaseEntityComponentIterator<TComponent> &other) { return BaseEntityIterator::operator!=(other); }
	  private:
		const std::vector<pragma::BaseEntityComponent *> *m_components = nullptr;
	};

	template<class TComponent>
	class EntityCIterator : public pragma::ecs::EntityIterator {
	  public:
		EntityCIterator(pragma::Game &game, FilterFlags filterFlags = FilterFlags::Default) : EntityIterator {game, false} { SetBaseComponentType(std::type_index(typeid(TComponent))); }
		BaseEntityComponentIterator<TComponent> begin() const { return BaseEntityComponentIterator<TComponent> {m_iteratorData, false}; }
		BaseEntityComponentIterator<TComponent> end() const { return BaseEntityComponentIterator<TComponent> {m_iteratorData, true}; }
	};

	///////////////

	template<class TComponent>
	class EntityComponentIterator {
	  public:
		EntityComponentIterator() = default;
		EntityComponentIterator(std::vector<pragma::ecs::BaseEntity *> &ents) : m_ents(&ents) {}
		EntityComponentIterator &operator++()
		{
			while(++m_currentIndex < m_ents->size()) {
				auto *ent = m_ents->at(m_currentIndex);
				if(ent != nullptr && ent->HasComponent<TComponent>())
					break;
			}
			return *this;
		}
		EntityComponentIterator operator++(int)
		{
			auto r = *this;
			operator++();
			return r;
		}
		TComponent &operator*() { return *operator->(); }
		TComponent *operator->() { return m_ents->at(m_currentIndex)->GetComponent<TComponent>().get(); }
		bool operator==(const EntityComponentIterator &other) { return m_ents == other.m_ents && m_currentIndex == other.m_currentIndex; }
		bool operator!=(const EntityComponentIterator &other) { return !operator==(other); }
	  protected:
		std::size_t m_currentIndex = std::numeric_limits<std::size_t>::max(); // Note: Intentional overflow at first iteration
		std::vector<pragma::ecs::BaseEntity *> *m_ents = nullptr;
	};

	template<class TComponent>
	class TEntityIterator {
	  public:
		TEntityIterator(pragma::Game &game) : m_game(game) {}
		EntityComponentIterator<TComponent> begin() { return EntityComponentIterator<TComponent> {m_game.GetBaseEntities()}; }
		EntityComponentIterator<TComponent> end() { return EntityComponentIterator<TComponent> {}; }
	  private:
		pragma::Game &m_game;
	};

	namespace pragma::ecs {
		template<class TFilter, typename... TARGS>
		void EntityIterator::AttachFilter(TARGS... args)
		{
			static_assert(std::is_base_of<IEntityIteratorFilter, TFilter>::value, "TFilter must be a descendant of IEntityIteratorFilter!");
			if(typeid(*m_iteratorData->entities) == typeid(EntityContainer)) {
				if constexpr(std::is_same_v<EntityIteratorFilterComponent, TFilter>) {
					// If a component filter was attached, we can optimize by only iterating the components of
					// that type (instead of iterating over all entities). In this case we don't actually need to
					// attach a filter.
					SetBaseComponentType(std::forward<TARGS>(args)...);
					return;
				}

				std::optional<std::type_index> componentTypeIndex {};
				GetIteratorFilterComponentType<TFilter>(componentTypeIndex);
				if(componentTypeIndex.has_value()) {
					// Same as the block above, but for TEntityIteratorFilterComponent
					SetBaseComponentType(*componentTypeIndex);
					return;
				}
			}
			m_iteratorData->filters.emplace_back(std::make_unique<TFilter>(m_iteratorData->game, std::forward<TARGS>(args)...));
		}
	}
};
