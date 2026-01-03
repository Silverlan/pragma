// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.debug;
import :debug;
import :entities.components.color;
import :entities.components.toggle;
import :entities.components.transform;

export namespace pragma {
	template<class TBaseComponent>
	class DLLCLIENT TCBaseDebugComponent : public TBaseComponent {
	  public:
		TCBaseDebugComponent(ecs::BaseEntity &ent);
		virtual void Initialize() override;
		virtual void OnEntitySpawn() override;
		virtual void OnRemove() override;
		void SetColorOverride(const Color &color);
		void ClearColorOverride();
		const std::optional<Color> &GetColorOverride() const;

		void SetIgnoreDepthBuffer(bool ignoreDepthBuffer);
		bool ShouldIgnoreDepthBuffer() const;
	  protected:
		virtual void OnEntityComponentAdded(BaseEntityComponent &component) override;
		virtual void OnEntityComponentRemoved(BaseEntityComponent &component) override;
		void ReloadDebugObject();
		virtual void DoReloadDebugObject(class Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) = 0;
		std::shared_ptr<debug::DebugRenderer::BaseObject> m_debugObject = nullptr;
		std::optional<Color> m_colorOverride {};
		bool m_ignoreDepthBuffer = false;
		CallbackHandle m_poseCallback = {};
		CallbackHandle m_colorCallback = {};
	};

	template<class TBaseComponent>
	TCBaseDebugComponent<TBaseComponent>::TCBaseDebugComponent(ecs::BaseEntity &ent) : TBaseComponent(ent)
	{
	}
	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::Initialize()
	{
		TBaseComponent::Initialize();
		auto &ent = this->GetEntity();
		ent.template AddComponent<LogicComponent>();

		this->BindEventUnhandled(cToggleComponent::EVENT_ON_TURN_ON, [this](std::reference_wrapper<ComponentEvent> evData) { ReloadDebugObject(); });
		this->BindEventUnhandled(cToggleComponent::EVENT_ON_TURN_OFF, [this](std::reference_wrapper<ComponentEvent> evData) { ReloadDebugObject(); });
	}
	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::OnEntitySpawn()
	{
		TBaseComponent::OnEntitySpawn();
		ReloadDebugObject();
	}
	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::OnRemove()
	{
		TBaseComponent::OnRemove();
		m_debugObject = nullptr;
	}
	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::SetColorOverride(const Color &color)
	{
		m_colorOverride = color;
	}
	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::ClearColorOverride()
	{
		m_colorOverride = {};
	}
	template<class TBaseComponent>
	const std::optional<Color> &TCBaseDebugComponent<TBaseComponent>::GetColorOverride() const
	{
		return m_colorOverride;
	}

	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::SetIgnoreDepthBuffer(bool ignoreDepthBuffer)
	{
		m_ignoreDepthBuffer = ignoreDepthBuffer;
		ReloadDebugObject();
	}
	template<class TBaseComponent>
	bool TCBaseDebugComponent<TBaseComponent>::ShouldIgnoreDepthBuffer() const
	{
		return m_ignoreDepthBuffer;
	}
	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::OnEntityComponentAdded(BaseEntityComponent &component)
	{
		TBaseComponent::OnEntityComponentAdded(component);
		if(typeid(component) == typeid(CColorComponent)) {
			if(m_colorCallback.IsValid())
				m_colorCallback.Remove();
			m_colorCallback = static_cast<CColorComponent &>(component).GetColorProperty()->AddCallback([this](std::reference_wrapper<const Vector4> oldColor, std::reference_wrapper<const Vector4> color) { ReloadDebugObject(); });
		}
		else if(typeid(component) == typeid(CTransformComponent)) {
			if(m_poseCallback.IsValid())
				m_poseCallback.Remove();
			auto &trC = static_cast<CTransformComponent &>(component);
			m_poseCallback = trC.AddEventCallback(cTransformComponent::EVENT_ON_POSE_CHANGED, [this, &trC](std::reference_wrapper<ComponentEvent> evData) -> util::EventReply {
				if(math::is_flag_set(static_cast<CEOnPoseChanged &>(evData.get()).changeFlags, TransformChangeFlags::PositionChanged) == false)
					return util::EventReply::Unhandled;
				if(m_debugObject != nullptr)
					m_debugObject->SetPos(trC.GetPosition());
				return util::EventReply::Unhandled;
			});
		}
	}
	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::OnEntityComponentRemoved(BaseEntityComponent &component)
	{
		TBaseComponent::OnEntityComponentRemoved(component);
		if(typeid(component) == typeid(CColorComponent)) {
			if(m_colorCallback.IsValid())
				m_colorCallback.Remove();
		}
		else if(typeid(component) == typeid(CTransformComponent)) {
			if(m_poseCallback.IsValid())
				m_poseCallback.Remove();
		}
	}
	template<class TBaseComponent>
	void TCBaseDebugComponent<TBaseComponent>::ReloadDebugObject()
	{
		m_debugObject = nullptr;
		auto &ent = this->GetEntity();
		auto pToggleComponent = ent.template GetComponent<CToggleComponent>();
		if(pToggleComponent.valid() && pToggleComponent->IsTurnedOn() == false)
			return;
		auto color = colors::White.ToVector4();
		if(m_colorOverride)
			color = m_colorOverride->ToVector4();
		else {
			auto pColorComponent = ent.template GetComponent<CColorComponent>();
			if(pColorComponent.valid())
				color = pColorComponent->GetColor();
		}
		auto pTrComponent = ent.template GetComponent<CTransformComponent>();
		auto pos = Vector3 {};
		if(pTrComponent.valid())
			pos = pTrComponent->GetPosition();
		debug::DebugRenderInfo renderInfo {color};
		renderInfo.pose.SetOrigin(pos);
		renderInfo.ignoreDepthBuffer = ShouldIgnoreDepthBuffer();
		DoReloadDebugObject(color, pos, renderInfo);
	}

	////////////////

	class DLLCLIENT CDebugTextComponent final : public TCBaseDebugComponent<BaseDebugTextComponent>, public CBaseNetComponent {
	  public:
		CDebugTextComponent(ecs::BaseEntity &ent) : TCBaseDebugComponent<BaseDebugTextComponent>(ent) {}
		virtual void SetText(const std::string &text) override;
		virtual void SetSize(float size) override;
		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual void DoReloadDebugObject(Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) override;
		std::string m_text = {};
	};

	////////////////

	class DLLCLIENT CBaseDebugOutlineComponent : public CBaseNetComponent {
	  public:
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};

	////////////////

	class DLLCLIENT CDebugPointComponent final : public TCBaseDebugComponent<BaseDebugPointComponent>, public CBaseNetComponent {
	  public:
		CDebugPointComponent(ecs::BaseEntity &ent) : TCBaseDebugComponent<BaseDebugPointComponent>(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void DoReloadDebugObject(Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) override;
	};

	////////////////

	class DLLCLIENT CDebugLineComponent final : public TCBaseDebugComponent<BaseDebugLineComponent>, public CBaseNetComponent {
	  public:
		CDebugLineComponent(ecs::BaseEntity &ent) : TCBaseDebugComponent<BaseDebugLineComponent>(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void DoReloadDebugObject(Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) override;
	};

	////////////////

	class DLLCLIENT CDebugBoxComponent final : public TCBaseDebugComponent<BaseDebugBoxComponent>, public CBaseDebugOutlineComponent {
	  public:
		CDebugBoxComponent(ecs::BaseEntity &ent) : TCBaseDebugComponent<BaseDebugBoxComponent>(ent) {}
		virtual void SetBounds(const Vector3 &min, const Vector3 &max) override;

		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual void DoReloadDebugObject(Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) override;
	};

	////////////////

	class DLLCLIENT CDebugSphereComponent final : public TCBaseDebugComponent<BaseDebugSphereComponent>, public CBaseDebugOutlineComponent {
	  public:
		CDebugSphereComponent(ecs::BaseEntity &ent) : TCBaseDebugComponent<BaseDebugSphereComponent>(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual void DoReloadDebugObject(Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) override;
	};

	////////////////

	class DLLCLIENT CDebugConeComponent final : public TCBaseDebugComponent<BaseDebugConeComponent>, public CBaseDebugOutlineComponent {
	  public:
		CDebugConeComponent(ecs::BaseEntity &ent) : TCBaseDebugComponent<BaseDebugConeComponent>(ent) {}
		virtual void SetConeAngle(float angle) override;
		virtual void SetStartRadius(float radius) override;

		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual void DoReloadDebugObject(Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) override;
	};

	////////////////

	class DLLCLIENT CDebugCylinderComponent final : public TCBaseDebugComponent<BaseDebugCylinderComponent>, public CBaseDebugOutlineComponent {
	  public:
		CDebugCylinderComponent(ecs::BaseEntity &ent) : TCBaseDebugComponent<BaseDebugCylinderComponent>(ent) {}
		virtual void SetLength(float length) override;

		virtual Bool ReceiveNetEvent(NetEventId eventId, NetPacket &packet) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
		virtual void DoReloadDebugObject(Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) override;
	};

	////////////////

	class DLLCLIENT CDebugPlaneComponent final : public TCBaseDebugComponent<BaseDebugPlaneComponent>, public CBaseNetComponent {
	  public:
		CDebugPlaneComponent(ecs::BaseEntity &ent) : TCBaseDebugComponent<BaseDebugPlaneComponent>(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual void ReceiveData(NetPacket &packet) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		virtual void DoReloadDebugObject(Color color, const Vector3 &pos, debug::DebugRenderInfo renderInfo) override;
	};
};
