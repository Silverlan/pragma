/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __S_PLAYER_COMPONENT_HPP__
#define __S_PLAYER_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/networking/ip_address.hpp>
#include <networkmanager/wrappers/nwm_ip_address.hpp>

class Player;
namespace pragma {
	namespace networking {
		class IServerClient;
	};
	class DLLSERVER SPlayerComponent final : public BasePlayerComponent, public SBaseNetComponent {
	  public:
		// Same as PlayActivity, but doesn't automatically transmit to clients if called serverside
		virtual bool PlaySharedActivity(Activity activity) override;
		static unsigned int GetPlayerCount();
		static const std::vector<SPlayerComponent *> &GetAll();
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		// Same as SetViewOrientation, but doesn't transmit anything to the client
		void UpdateViewOrientation(const Quat &rot);
		void Kick(const std::string &reason);
		std::vector<InputAction> &GetKeyStack();
		void ClearKeyStack();
		void SetWalkSpeed(float speed);
		void SetRunSpeed(float speed);
		void SetSprintSpeed(float speed);
		void SetCrouchedWalkSpeed(float speed);
		void SetClientSession(networking::IServerClient &session);
		Con::c_cout &print(Con::c_cout &);
		std::ostream &print(std::ostream &);
		networking::IServerClient *GetClientSession();
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual std::string GetClientIP() override;
		virtual unsigned short GetClientPort() override;
		networking::IPAddress GetClientIPAddress() const;
		void InitializeGlobalNameComponent();
		bool IsAuthed();
		void SetAuthed(bool b);
		void SetStandHeight(float height);
		void SetCrouchHeight(float height);
		void SetStandEyeLevel(float eyelevel);
		void SetCrouchEyeLevel(float eyelevel);
		bool IsGameReady() const;
		void SetGameReady(bool b);
		bool SendResource(const std::string &fileName) const;
		virtual void PrintMessage(std::string message, MESSAGE type) override;

		virtual void SetViewRotation(const Quat &rot) override;

		virtual void ApplyViewRotationOffset(const EulerAngles &ang, float dur = 0.5f) override;

		virtual bool ShouldTransmitNetData() const override { return true; };
		virtual void InitializeLuaObject(lua_State *l) override;
	  protected:
		mutable util::WeakHandle<networking::IServerClient> m_session = {};
		bool m_bGameReady;
		bool m_bAuthed;
		std::vector<InputAction> m_keyStack;
		void InitializeFlashlight();
		void OnTakeDamage(DamageInfo &info);
		void OnSetSlopeLimit(float limit);
		void OnSetStepOffset(float offset);
		virtual void Initialize() override;
		void OnRespawn();
		virtual void OnEntitySpawn() override;
		void OnSetViewOrientation(const Quat &orientation);
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	  private:
		static std::vector<SPlayerComponent *> s_players;
	  public:
		SPlayerComponent(BaseEntity &ent);
		virtual ~SPlayerComponent() override;
	};
};

#endif
