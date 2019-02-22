#ifndef __C_SOUND_EFX_HPP__
#define __C_SOUND_EFX_HPP__

#include "pragma/clientdefinitions.h"
#include <vector>
#include <string>
#include <memory>

namespace ds {class Block;};
namespace al
{
	class Effect;
	DLLCLIENT std::shared_ptr<al::Effect> create_aux_effect(const std::string &type,ds::Block &block);
	DLLCLIENT std::shared_ptr<al::Effect> create_aux_effect(const std::string &name,const std::string &type,ds::Block &block);
	DLLCLIENT const std::vector<std::string> &get_aux_types();
};

#endif
