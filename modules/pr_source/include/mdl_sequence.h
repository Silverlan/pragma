#ifndef __MDL_SEQUENCE_H__
#define __MDL_SEQUENCE_H__

#include <mathutil/uvec.h>
#include <fsys/filesystem.h>
#include <array>

#pragma pack(push,1)
namespace import
{
	namespace mdl
	{
		struct mstudioevent_t
		{
			float cycle;
			int32_t event;
			int32_t type;

			std::array<char,64> options;

			int32_t szeventindex;
		};

		struct mstudioseqdesc_t
		{
			int32_t baseptr;
			int32_t szlabelindex;

			int32_t szactivitynameindex;

			int32_t flags; // looping/non-looping flags

			int32_t activity; // initialized at loadtime to game DLL values
			int32_t actweight;

			int32_t numevents;
			int32_t eventindex;
	
			Vector3 bbmin; // per sequence bounding box
			Vector3 bbmax;		

			int32_t numblends;

			// Index into array of shorts which is groupsize[0] x groupsize[1] in length
			int32_t animindexindex;

			int32_t movementindex; // [blend] float array for blended movement
			std::array<int32_t,2> groupsize;
			std::array<int32_t,2> paramindex; // X, Y, Z, XR, YR, ZR
			std::array<float,2> paramstart; // local (0..1) starting value
			std::array<float,2> paramend; // local (0..1) ending value
			int32_t paramparent;

			float fadeintime; // ideal cross fate in time (0.2 default)
			float fadeouttime; // ideal cross fade out time (0.2 default)

			int32_t localentrynode; // transition node at entry
			int32_t localexitnode; // transition node at exit
			int32_t nodeflags; // transition rules

			float entryphase; // used to match entry gait
			float exitphase; // used to match exit gait
	
			float lastframe; // frame that should generation EndOfSequence

			int32_t nextseq; // auto advancing sequences
			int32_t pose; // index of delta animation between end and nextseq

			int32_t numikrules;

			int32_t numautolayers;
			int32_t autolayerindex;

			int32_t weightlistindex;

			// FIXME: make this 2D instead of 2x1D arrays
			int32_t posekeyindex;

			int32_t numiklocks;
			int32_t iklockindex;

			// Key values
			int32_t keyvalueindex;
			int32_t keyvaluesize;

			int32_t cycleposeindex; // index of pose parameter to use as cycle index

			int32_t activitymodifierindex;
			int32_t numactivitymodifiers;

			std::array<int32_t,5> unused; // remove/add as appropriate (grow back to 8 ints on version change!)
		};
		static std::string translate_event_name(const std::string &name);
		class Sequence
		{
		public:
			class Event
			{
				float m_cycle;
				int32_t m_event;
				std::string m_name;
				std::string m_options;
			public:
				Event(const VFilePtr &f);
				void DebugPrint(const std::string &t="");

				float GetCycle() const;
				int32_t GetEvent() const;
				const std::string &GetName() const;
				const std::string &GetOptions() const;
			};
			struct PoseParameter
			{
				std::array<int32_t,2> paramIdx = {-1,-1};
				std::array<float,2> start = {0.f,0.f};
				std::array<float,2> end = {0.f,0.f};
				uint32_t numBlends = 0;
			};
		private:
			std::string m_name;
			std::string m_activityName;
			int32_t m_activity;
			int32_t m_activityWeight;
			int32_t m_flags;
			float m_fadeInTime;
			float m_fadeOutTime;
			Vector3 m_min;
			Vector3 m_max;
			PoseParameter m_poseParam = {};
			std::vector<int16_t> m_animations;
			std::vector<Event> m_events;
			std::vector<float> m_weights;
			bool m_bDefaultBoneWeights = true;
		public:
			Sequence(const VFilePtr &f,int32_t boneCount);
			void DebugPrint(const std::string &t="");

			const std::string &GetName() const;
			const std::string &GetActivityName() const;
			int32_t GetActivity() const;
			int32_t GetActivityWeight() const;
			int32_t GetFlags() const;
			float GetFadeInTime() const;
			float GetFadeOutTime() const;
			const Vector3 &GetMin() const;
			const Vector3 &GetMax() const;
			const PoseParameter &GetPoseParameter() const;
			const std::vector<Event> &GetEvents() const;
			const std::vector<int16_t> &GetAnimationIndices() const;
			const std::vector<float> &GetWeights() const;
			bool IsUsingDefaultBoneWeights() const;
		};
	};
};
#pragma pack(pop)

#endif
