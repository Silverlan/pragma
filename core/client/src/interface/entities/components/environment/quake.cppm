// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.env_quake;

export import :entities.base_entity;
export import :entities.components.entity;

export {
	namespace pragma {
		class DLLCLIENT CQuakeComponent final : public BaseEnvQuakeComponent, public CBaseNetComponent {
		  public:
			CQuakeComponent(ecs::BaseEntity &ent) : BaseEnvQuakeComponent(ent) {}
			virtual ~CQuakeComponent() override;

			virtual void ReceiveData(NetPacket &packet) override;
			virtual void InitializeLuaObject(lua::State *l) override;
			virtual bool ShouldTransmitNetData() const override { return true; }
		  protected:
			CallbackHandle m_cbScreenShake;
			virtual void StartShake() override;
			void CancelScreenShake();
		};
	};

	class DLLCLIENT CEnvQuake : public pragma::ecs::CBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
