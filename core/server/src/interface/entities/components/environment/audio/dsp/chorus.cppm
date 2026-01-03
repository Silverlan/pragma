// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:entities.components.audio.dsp.chorus;

import :entities;
import :entities.components.audio.dsp.base;

export {
	namespace pragma {
		class DLLSERVER SSoundDspChorusComponent final : public SBaseSoundDspComponent, public BaseEnvSoundDspChorus {
		  public:
			SSoundDspChorusComponent(ecs::BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
			virtual bool OnSetKeyValue(const std::string &key, const std::string &val) override;
			virtual void SendData(NetPacket &packet, networking::ClientRecipientFilter &rp) override;
			virtual void InitializeLuaObject(lua::State *l) override;
		};
	};

	class DLLSERVER EnvSoundDspChorus : public SBaseEntity {
	  public:
		virtual void Initialize() override;
	};
};
