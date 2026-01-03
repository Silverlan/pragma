// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

export module pragma.server:ai.tasks.play_sound;

export import :ai.schedule;

export namespace pragma {
	namespace ai {
		class DLLSERVER TaskPlaySound : public BehaviorNode {
		  public:
			enum class Parameter : uint32_t { SoundName = 0u, Gain, Pitch };
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override { return BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>(); }
			virtual Result Start(const Schedule *sched, BaseAIComponent &ent) override;
			virtual void Print(const Schedule *sched, std::ostream &o) const override;
			float GetGain(const Schedule *sched) const;
			float GetPitch(const Schedule *sched) const;
			const std::string *GetSoundName(const Schedule *sched) const;

			void SetSoundName(const std::string &sndName);
			void SetGain(float gain);
			void SetPitch(float pitch);
		};
	};
};
