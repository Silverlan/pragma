// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#include "definitions.hpp"

export module pragma.server:networking.resource;

export import pragma.filesystem;

export {
#pragma warning(push)
#pragma warning(disable : 4251)
	struct DLLSERVER Resource {
		Resource(std::string name, bool bStream = true);
		~Resource();
		bool Construct();
		std::string name;
		unsigned int offset;
		std::shared_ptr<VFilePtrInternal> file;
		bool stream;
	};
#pragma warning(pop)
}
