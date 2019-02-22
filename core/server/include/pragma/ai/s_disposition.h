#ifndef __S_DISPOSITION_H__
#define __S_DISPOSITION_H__

#include "pragma/serverdefinitions.h"

enum class DLLSERVER DISPOSITION : uint32_t
{
	HATE,
	FEAR,
	NEUTRAL,
	LIKE,
	COUNT
};

#endif