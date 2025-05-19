/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_PLAYER_COMPONENT_HPP__
#define __C_PLAYER_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_player_component.hpp>
#include <pragma/util/util_handled.hpp>
#include <pragma/math/deltaoffset.h>
#include <pragma/math/deltatransform.h>

class CBaseEntity;
struct DeltaTransform;
namespace pragma {
	class CObservableComponent;
	class DLLCLIENT CPlayerComponent final : public BasePlayerComponent, public CBaseNetComponent {
	  private:
		static std::vector<CPlayerComponent *> s_players;
		const float VIEW_BODY_OFFSET = -20.f;
	  public:
		CPlayerComponent(BaseEntity &ent);
		virtual ~CPlayerComponent() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua_State *l) override;
	  public:
		static unsigned int GetPlayerCount();
		static const std::vector<CPlayerComponent *> &GetAll();
		virtual Con::c_cout &print(Con::c_cout &) override;
		virtual std::ostream &print(std::ostream &) override;
		virtual void Initialize() override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnTick(double tDelta) override;
		virtual void OnCrouch() override;
		virtual void OnUnCrouch() override;
		virtual void SetLocalPlayer(bool b) override;
		virtual util::EventReply HandleEvent(ComponentEventId eventId, ComponentEvent &evData) override;
		bool IsInFirstPersonMode() const;

		virtual void ApplyViewRotationOffset(const EulerAngles &ang, float dur = 0.5f) override;
		virtual void PrintMessage(std::string message, MESSAGE type) override;

		void UpdateViewModelTransform();
		void UpdateViewFOV();
	  protected:
		CallbackHandle m_cbCalcOrientationView = {};
		std::unique_ptr<DeltaOffset> m_crouchViewOffset = nullptr;
		std::unique_ptr<DeltaTransform> m_upDirOffset = nullptr;
		std::shared_ptr<ALSound> m_sndUnderwater = nullptr;
		EntityHandle m_cbUnderwaterDsp = {};
		void OnWaterSubmerged();
		void OnWaterEmerged();
		void OnUpdateMatrices(Mat4 &transformMatrix);
		void OnDeployWeapon(BaseEntity &ent);
		void OnSetActiveWeapon(BaseEntity *ent);
		void OnSetUpDirection(const Vector3 &direction);
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId, NetPacket &packet) override;
		void OnSetCharacterOrientation(const Vector3 &up);
		bool ShouldDraw() const;
		bool ShouldDrawShadow() const;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};

#endif
