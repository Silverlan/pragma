// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.player;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CPlayerComponent final : public BasePlayerComponent, public CBaseNetComponent {
	  private:
		static std::vector<CPlayerComponent *> s_players;
		const float VIEW_BODY_OFFSET = -20.f;
	  public:
		static void RegisterLuaBindings(lua::State *l, luabind::module_ &modEnts);

		CPlayerComponent(ecs::BaseEntity &ent);
		virtual ~CPlayerComponent() override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
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
		virtual void PrintMessage(std::string message, console::MESSAGE type) override;

		void UpdateViewModelTransform();
		void UpdateViewFOV();
	  protected:
		CallbackHandle m_cbCalcOrientationView = {};
		std::unique_ptr<math::DeltaOffset> m_crouchViewOffset = nullptr;
		std::unique_ptr<math::DeltaTransform> m_upDirOffset = nullptr;
		std::shared_ptr<audio::ALSound> m_sndUnderwater = nullptr;
		EntityHandle m_cbUnderwaterDsp = {};
		void OnWaterSubmerged();
		void OnWaterEmerged();
		void OnUpdateMatrices(Mat4 &transformMatrix);
		void OnDeployWeapon(ecs::BaseEntity &ent);
		void OnSetActiveWeapon(ecs::BaseEntity *ent);
		void OnSetUpDirection(const Vector3 &direction);
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		void OnSetCharacterOrientation(const Vector3 &up);
		bool ShouldDraw() const;
		bool ShouldDrawShadow() const;
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
