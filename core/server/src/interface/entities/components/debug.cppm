// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.debug;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SDebugTextComponent final : public BaseDebugTextComponent, public SBaseNetComponent {
	  public:
		SDebugTextComponent(ecs::BaseEntity &ent) : BaseDebugTextComponent(ent) {}
		virtual void SetText(const std::string &text) override;
		virtual void SetSize(float size) override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};

	////////////////

	class DLLSERVER SBaseDebugOutlineComponent : public SBaseNetComponent {
	  public:
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	  protected:
		SBaseDebugOutlineComponent() = default;
	};

	////////////////

	class DLLSERVER SDebugPointComponent final : public BaseDebugPointComponent, public SBaseNetComponent {
	  public:
		SDebugPointComponent(ecs::BaseEntity &ent) : BaseDebugPointComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};

	////////////////

	class DLLSERVER SDebugLineComponent final : public BaseDebugLineComponent, public SBaseNetComponent {
	  public:
		SDebugLineComponent(ecs::BaseEntity &ent) : BaseDebugLineComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};

	////////////////

	class DLLSERVER SDebugBoxComponent final : public BaseDebugBoxComponent, public SBaseDebugOutlineComponent {
	  public:
		SDebugBoxComponent(ecs::BaseEntity &ent) : BaseDebugBoxComponent(ent) {}
		virtual void SetBounds(const Vector3 &min, const Vector3 &max) override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};

	////////////////

	class DLLSERVER SDebugSphereComponent final : public BaseDebugSphereComponent, public SBaseDebugOutlineComponent {
	  public:
		SDebugSphereComponent(ecs::BaseEntity &ent) : BaseDebugSphereComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};

	////////////////

	class DLLSERVER SDebugConeComponent final : public BaseDebugConeComponent, public SBaseDebugOutlineComponent {
	  public:
		SDebugConeComponent(ecs::BaseEntity &ent) : BaseDebugConeComponent(ent) {}
		virtual void SetConeAngle(float angle) override;
		virtual void SetStartRadius(float radius) override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};

	////////////////

	class DLLSERVER SDebugCylinderComponent final : public BaseDebugCylinderComponent, public SBaseDebugOutlineComponent {
	  public:
		SDebugCylinderComponent(ecs::BaseEntity &ent) : BaseDebugCylinderComponent(ent) {}
		virtual void SetLength(float length) override;

		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
	};

	////////////////

	class DLLSERVER SDebugPlaneComponent final : public BaseDebugPlaneComponent {
	  public:
		SDebugPlaneComponent(ecs::BaseEntity &ent) : BaseDebugPlaneComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};
