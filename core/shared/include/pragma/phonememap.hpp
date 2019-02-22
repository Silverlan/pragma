#ifndef __PHONEMEMAP_HPP__
#define __PHONEMEMAP_HPP__

#include "pragma/networkdefinitions.h"
#include <string>
#include <unordered_map>

struct DLLNETWORK PhonemeInfo
{
	std::unordered_map<std::string,float> flexControllers;
};

struct DLLNETWORK PhonemeMap
{
	std::unordered_map<std::string,PhonemeInfo> phonemes;
};

#endif
