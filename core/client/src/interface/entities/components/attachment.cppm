// SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.attachment;

export import :entities.components.entity;

export namespace pragma {
	class DLLCLIENT CAttachmentComponent final : public BaseAttachmentComponent, public CBaseNetComponent {
	  public:
		CAttachmentComponent(ecs::BaseEntity &ent) : BaseAttachmentComponent(ent) {}
		virtual void InitializeLuaObject(lua::State *l) override;
		virtual bool ShouldTransmitNetData() const override { return true; }

		virtual void ReceiveData(NetPacket &packet) override;
		virtual void UpdateViewAttachmentOffset(ecs::BaseEntity *ent, BaseCharacterComponent &pl, Vector3 &pos, Quat &rot, Bool bYawOnly = false) const override;
	  protected:
		virtual void GetBaseTypeIndex(std::type_index &outTypeIndex) const override;
	};
};
