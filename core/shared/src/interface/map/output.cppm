// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "pragma/networkdefinitions.h"

export module pragma.shared:map.output;

export import pragma.filesystem;

export {
	namespace pragma::asset {
		struct DLLNETWORK Output {
			std::string name;
			std::string target;
			std::string input;
			std::string param;
			float delay = 0.f;
			int32_t times = -1;

			void Write(VFilePtrReal &f);
			void Read(VFilePtr &f);
		};
	};
	DLLNETWORK std::ostream &operator<<(std::ostream &out, const pragma::asset::Output &output);
};
