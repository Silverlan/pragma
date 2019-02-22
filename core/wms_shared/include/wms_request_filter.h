#ifndef __WMS_REQUEST_FILTER_H__
#define __WMS_REQUEST_FILTER_H__

#include "wms_shareddefinitions.h"
enum class DLLWMSSHARED RequestFilter : unsigned int
{
	NONE = 0,
	OR = 1,
	AND = 2,
	NOT_EMPTY = 4,
	NOT_FULL = 8,
	EMPTY = 16,
	NO_PASSWORD = 32
};
DLLWMSSHARED RequestFilter operator&(const RequestFilter &a,const RequestFilter &b);
DLLWMSSHARED RequestFilter operator|(const RequestFilter &a,const RequestFilter &b);
DLLWMSSHARED RequestFilter operator|=(const RequestFilter &a,const RequestFilter &b);
DLLWMSSHARED bool operator!(const RequestFilter &a);

#endif