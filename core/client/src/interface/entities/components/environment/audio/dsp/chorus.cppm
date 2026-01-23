// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.client:entities.components.audio.dsp.chorus;

export import :entities.components.audio.dsp.base;

export namespace pragma {
	class DLLCLIENT CSoundDspChorusComponent final : public CBaseSoundDspComponent, public BaseEnvSoundDspChorus {
	  public:
		CSoundDspChorusComponent(ecs::BaseEntity &ent) : CBaseSoundDspComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void OnEntitySpawn() override;
		virtual void InitializeLuaObject(lua::State *l) override;
	};
};

export class DLLCLIENT CEnvSoundDspChorus : public pragma::ecs::CBaseEntity {
  public:
	virtual void Initialize() override;
};
