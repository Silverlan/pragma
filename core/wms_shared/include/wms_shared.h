#ifndef __WMS_SHARED_H__
#define __WMS_SHARED_H__

#include <string>

#define WMS_PROTOCOL_VERSION 2

std::string GetMasterServerIP();
unsigned short GetMasterServerPort();

#endif