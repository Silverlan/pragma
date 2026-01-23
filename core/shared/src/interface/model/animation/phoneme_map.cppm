// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:model.animation.phoneme_map;

export import pragma.math;

export {
	struct DLLNETWORK PhonemeInfo {
		std::unordered_map<std::string, float> flexControllers;
		bool operator==(const PhonemeInfo &other) const
		{
			if(flexControllers.size() != other.flexControllers.size())
				return false;
			for(auto &pair : flexControllers) {
				auto it = other.flexControllers.find(pair.first);
				if(it == other.flexControllers.end())
					return false;
				if(pragma::math::abs(pair.second - it->second) > 0.001f)
					return false;
			}
			return true;
		}
		bool operator!=(const PhonemeInfo &other) const { return !operator==(other); }
	};

	struct DLLNETWORK PhonemeMap {
		std::unordered_map<std::string, PhonemeInfo> phonemes;
		bool operator==(const PhonemeMap &other) const { return phonemes == other.phonemes; }
		bool operator!=(const PhonemeMap &other) const { return !operator==(other); }
	};
};
