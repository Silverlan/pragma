/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SOUND_LOAD_H__
#define __C_SOUND_LOAD_H__

#include "pragma/c_enginedefinitions.h"
#include <string>
#include <cinttypes>

class VFilePtrInternal;
struct ALAudio;
enum class ALChannel : uint32_t;
namespace openal {
	bool load_ogg(std::string &path, std::shared_ptr<VFilePtrInternal> f, ALChannel mode, ALAudio &audio);
	bool load_alure(std::string &path, std::shared_ptr<VFilePtrInternal> f, ALChannel mode, ALAudio &audio);
#ifdef WEAVE_MP3_SUPPORT_ENABLED
	bool load_ffmpeg(std::string &path, std::shared_ptr<VFilePtrInternal> f, ALChannel mode, ALAudio &audio);
#endif
};

#endif
