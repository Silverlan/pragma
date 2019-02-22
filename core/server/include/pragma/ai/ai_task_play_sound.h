#ifndef __AI_TASK_PLAY_SOUND_H__
#define __AI_TASK_PLAY_SOUND_H__

#include "pragma/serverdefinitions.h"
#include "pragma/ai/ai_behavior.h"

namespace pragma
{
	namespace ai
	{
		class DLLSERVER TaskPlaySound
			: public ai::BehaviorNode
		{
		public:
			enum class Parameter : uint32_t
			{
				SoundName = 0u,
				Gain,
				Pitch
			};
			using BehaviorNode::BehaviorNode;
			virtual std::shared_ptr<BehaviorNode> Copy() const override {return ai::BehaviorNode::Copy<std::remove_const_t<std::remove_reference_t<decltype(*this)>>>();}
			virtual ai::BehaviorNode::Result Start(const Schedule *sched,pragma::SAIComponent &ent) override;
			virtual void Print(const Schedule *sched,std::ostream &o) const override;
			float GetGain(const Schedule *sched) const;
			float GetPitch(const Schedule *sched) const;
			const std::string *GetSoundName(const Schedule *sched) const;

			void SetSoundName(const std::string &sndName);
			void SetGain(float gain);
			void SetPitch(float pitch);
		};
	};
};

#endif
