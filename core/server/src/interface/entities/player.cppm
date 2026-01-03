// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.player;

import :entities.base;

export class DLLSERVER Player : public SBaseEntity, public BasePlayer {
  public:
	virtual void Initialize() override;
};
