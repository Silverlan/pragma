// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.networked;

import :entities.components.entity;

export namespace pragma {
	class DLLSERVER SNetworkedComponent final : public BaseNetworkedComponent, public SBaseNetComponent {
	  public:
		SNetworkedComponent(ecs::BaseEntity &ent) : BaseNetworkedComponent(ent) {}
		virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
		virtual void SetNetworkFlags(NetworkFlags flags) override;
		virtual bool ShouldTransmitNetData() const override { return true; }
		virtual void InitializeLuaObject(lua::State *l) override;
	  protected:
#if NETWORKED_VARS_ENABLED != 0
		template<class TProperty>
		void add_networked_variable_callback(NetworkedVariable::Id id, pragma::util::BaseProperty &prop);
		template<typename T>
		static NetworkedVariable::Type get_networked_variable_type();
		virtual void OnNetworkedVariableCreated(NetworkedVariable &nwVar) override;
#endif
	};
};
