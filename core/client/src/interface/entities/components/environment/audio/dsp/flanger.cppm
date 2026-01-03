// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.audio.dsp.flanger;

export import :entities.components.audio.dsp.base;

export namespace pragma {
	class DLLCLIENT CSoundDspFlangerComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspFlanger {
	  public:
		CSoundDspFlangerComponent(ecs::BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDspFlanger : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
