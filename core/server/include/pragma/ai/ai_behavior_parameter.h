/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __AI_BEHAVIOR_PARAMETER_H__
#define __AI_BEHAVIOR_PARAMETER_H__

#include "pragma/serverdefinitions.h"
#include <mathutil/glmutil.h>
#include <memory>
#include <vector>
#include <string>

class BaseEntity;
namespace pragma {
	namespace ai {
		class DLLSERVER BehaviorParameter {
		  public:
			enum class DLLSERVER Type : uint32_t { None = 0, Bool, Int, Float, String, Vector, Quaternion, EulerAngles, Entity };
		  protected:
			std::shared_ptr<void> m_data;
			Type m_type;
		  public:
			BehaviorParameter(const BehaviorParameter &) = delete;
			BehaviorParameter &operator=(const BehaviorParameter &) = delete;
			BehaviorParameter(bool b);
			BehaviorParameter(int32_t i);
			BehaviorParameter(float f);
			BehaviorParameter(const std::string &s);
			BehaviorParameter(const BaseEntity *ent);
			BehaviorParameter(const Vector3 &vec);
			BehaviorParameter(const Quat &rot);
			BehaviorParameter(const EulerAngles &ang);
			BehaviorParameter *Copy() const;
			bool GetBool() const;
			int32_t GetInt() const;
			float GetFloat() const;
			const std::string *GetString() const;
			const BaseEntity *GetEntity() const;
			const Vector3 *GetVector() const;
			const Quat *GetQuaternion() const;
			const EulerAngles *GetEulerAngles() const;
			std::string ToString() const;
			Type GetType() const;
		};
		class BehaviorNode;
		class DLLSERVER ParameterBase {
		  protected:
			struct ParameterInfo {
				ParameterInfo() = default;
				ParameterInfo(BehaviorParameter *param);
				ParameterInfo(const ParameterInfo &other);
				std::unique_ptr<BehaviorParameter> parameter = nullptr;
				std::vector<std::pair<BehaviorNode *, uint8_t>> links;
			};
			std::vector<std::unique_ptr<ParameterInfo>> m_params;
			virtual void OnParameterChanged(uint8_t paramId);
			ParameterInfo &InitParameter(uint8_t paramId);
		  public:
			ParameterBase() = default;
			ParameterBase(const ParameterBase &other);
			ParameterBase &operator=(const ParameterBase &) = delete;
			void SetParameter(uint8_t idx, bool b);
			void SetParameter(uint8_t idx, int32_t i);
			void SetParameter(uint8_t idx, float f);
			void SetParameter(uint8_t idx, const std::string &s);
			void SetParameter(uint8_t idx, const BaseEntity *ent);
			void SetParameter(uint8_t idx, const Vector3 &vec);
			void SetParameter(uint8_t idx, const Quat &rot);
			void SetParameter(uint8_t idx, const EulerAngles &ang);
			const BehaviorParameter *GetParameter(uint8_t paramID) const;

			void LinkParameter(uint8_t paramId, BehaviorNode &other, uint8_t paramIdOther);
			void LinkParameter(uint8_t paramId, BehaviorNode &other);
		};
	};
};

#endif
