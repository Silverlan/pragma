#include "wms_shared.h"

std::string GetMasterServerIP()
{
//#ifdef _DEBUG
//	return "127.0.0.1";
//#else
	return "85.214.192.20";
	//return "::1";
//#endif
}
unsigned short GetMasterServerPort()
{
//#ifdef _DEBUG
//	return 27015;
//#else
	return 29155;
//#endif
}
