// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.player;

export import :entities.components;
export import pragma.shared;

export class DLLCLIENT CPlayer : public pragma::ecs::CBaseEntity, public BasePlayer {
  public:
	virtual void Initialize() override;
};

void CPlayer::Initialize()
{
	CBaseEntity::Initialize();
	auto whPlComponent = AddComponent<pragma::CPlayerComponent>();
	if(whPlComponent.expired() == false)
		m_basePlayerComponent = pragma::util::WeakHandle<pragma::BasePlayerComponent>(std::static_pointer_cast<pragma::BasePlayerComponent>(whPlComponent.get()->shared_from_this()));
}
