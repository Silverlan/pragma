#ifndef __C_ALSOUND_HPP__
#define __C_ALSOUND_HPP__

#include "pragma/clientdefinitions.h"
#include <pragma/lua/classes/lalsound.h>
#include <alsound_buffer.hpp>

namespace Lua
{
	namespace ALSound
	{
		namespace Client
		{
			DLLCLIENT void register_class(luabind::class_<::ALSound> &classDef);
			DLLCLIENT void register_buffer(luabind::class_<al::SoundBuffer> &classDef);
		};
	};
};

#endif
