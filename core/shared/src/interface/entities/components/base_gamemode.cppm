// SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "pragma/networkdefinitions.h"

#undef GetClassName

#include <string>

export module pragma.shared:entities.components.base_gamemode;

export import :entities.components.base;
export import :game.damage_info;
export import :game.game_mode_manager;
export import :networking.enums;

export namespace pragma {
	class BasePlayerComponent;
	class DLLNETWORK BaseGamemodeComponent : public BaseEntityComponent {
	  public:
		static ComponentEventId EVENT_ON_PLAYER_DEATH;
		static ComponentEventId EVENT_ON_PLAYER_SPAWNED;
		static ComponentEventId EVENT_ON_PLAYER_DROPPED;
		static ComponentEventId EVENT_ON_PLAYER_READY;
		static ComponentEventId EVENT_ON_PLAYER_JOINED;
		static ComponentEventId EVENT_ON_GAME_INITIALIZED;
		static ComponentEventId EVENT_ON_MAP_INITIALIZED;
		static ComponentEventId EVENT_ON_GAME_READY;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager, TRegisterComponentEvent registerEvent);

		virtual void Initialize() override;
		virtual void OnRemove() override;

		virtual void Save(udm::LinkedPropertyWrapperArg udm) override;
		virtual void Load(udm::LinkedPropertyWrapperArg udm, uint32_t version) override;

		virtual void OnPlayerDeath(BasePlayerComponent &pl, DamageInfo *dmgInfo);
		virtual void OnPlayerSpawned(BasePlayerComponent &pl);
		virtual void OnPlayerDropped(BasePlayerComponent &pl, pragma::networking::DropReason reason);
		virtual void OnPlayerReady(BasePlayerComponent &pl);
		virtual void OnPlayerJoined(BasePlayerComponent &pl);
		virtual void OnGameInitialized();
		virtual void OnMapInitialized();
		virtual void OnGameReady();

		GameModeInfo *GetGameModeInfo();
		const GameModeInfo *GetGameModeInfo() const { return const_cast<BaseGamemodeComponent *>(this)->GetGameModeInfo(); }
		const std::string &GetName() const;
		const std::string &GetIdentifier() const;
		const std::string &GetComponentName() const;
		const std::string &GetAuthor() const;
		::util::Version GetGamemodeVersion() const;
	  protected:
		BaseGamemodeComponent(pragma::ecs::BaseEntity &ent);
	};
	struct DLLNETWORK CEPlayerDeath : public ComponentEvent {
		CEPlayerDeath(BasePlayerComponent &pl, DamageInfo *dmgInfo);
		virtual void PushArguments(lua_State *l) override;
		BasePlayerComponent &player;
		DamageInfo *dmgInfo = nullptr;
	};
	struct DLLNETWORK CEPlayerDropped : public ComponentEvent {
		CEPlayerDropped(BasePlayerComponent &pl, pragma::networking::DropReason reason);
		virtual void PushArguments(lua_State *l) override;
		BasePlayerComponent &player;
		pragma::networking::DropReason reason;
	};
	struct DLLNETWORK CEPlayerSpawned : public ComponentEvent {
		CEPlayerSpawned(BasePlayerComponent &pl);
		virtual void PushArguments(lua_State *l) override;
		BasePlayerComponent &player;
	};
	using CEPlayerReady = CEPlayerSpawned;
	using CEPlayerJoined = CEPlayerSpawned;
};
