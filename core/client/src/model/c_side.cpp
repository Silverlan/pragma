#include "stdafx_client.h"
#include "pragma/model/c_side.h"
#include <mathutil/umath.h>

CSide::CSide()
	: Side(),NormalMesh()
{
}

DLLCLIENT Con::c_cout& operator<<(Con::c_cout &os,const CSide side)
{
	return os<<(Side*)&side;
}

DLLCLIENT std::ostream& operator<<(std::ostream &os,const CSide side)
{
	return os<<(Side*)&side;
}
