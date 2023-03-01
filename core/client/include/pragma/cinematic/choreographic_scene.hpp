/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __CHOREOGRAPHIC_SCENE_HPP__
#define __CHOREOGRAPHIC_SCENE_HPP__

#include <util_timeline_scene.hpp>
#include <util_timeline_channel.hpp>
#include <util_timeline_event.hpp>
#include <pragma/entities/baseentity_handle.h>

class CBaseEntity;
namespace choreography {
	class Event : public uts::Event {
	  public:
		virtual ~Event() override = default;
		CBaseEntity *GetActor() const;
		void SetActor(CBaseEntity &actor);
		void ClearActor();
	  protected:
		Event(uts::Channel &channel);
		friend uts::Event;

		mutable EntityHandle m_actor = {};
	};

	class AudioEvent : public Event {
	  public:
		virtual void Initialize() override;
		virtual void Start() override;
		virtual void Reset() override;
	  protected:
		AudioEvent(uts::Channel &channel, const std::string &snd);
		friend uts::Event;
		virtual State HandleTick(double t, double dt) override;
		std::shared_ptr<ALSound> m_sound = nullptr;
		std::string m_soundSource;
	};

	class FacialFlexEvent : public Event {
	  public:
		struct TimeValue {
			float time = 0.f;
			float value = 0.f;
		};
		virtual void Initialize() override;
		virtual void Start() override;
		virtual void Reset() override;

		void SetStereo(bool b);
		void SetFlexControllerValues(const std::string &name, const std::vector<TimeValue> &values, const std::vector<TimeValue> *lrDistribution = nullptr);
	  protected:
		FacialFlexEvent(uts::Channel &channel);
		friend uts::Event;
		virtual State HandleTick(double t, double dt) override;
		bool GetInterpolatedValue(double t, const std::vector<TimeValue> &srcValues, float &v, bool bGetLastAvailable = false) const;
		struct FlexControllerInfo {
			std::vector<TimeValue> values;
			std::vector<TimeValue> leftRightDistribution;
			bool stereo = false;
			uint32_t m_lastValue = std::numeric_limits<uint32_t>::max();
		};
		std::unordered_map<std::string, FlexControllerInfo> m_values;
	};

	class Channel : public uts::Channel {
	  public:
		virtual ~Channel() override = default;
	  protected:
		Channel(uts::TimelineScene &scene, const std::string &name);
		friend uts::Channel;
	};
	class Scene : public uts::TimelineScene {
	  public:
		virtual ~Scene() override = default;
	  protected:
		Scene();
		friend uts::TimelineScene;
	};
};

#endif
