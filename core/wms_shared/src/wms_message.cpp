#include "wms_message.h"
#include "wms_shared.h"

WMSMessageHeader::WMSMessageHeader(unsigned int _id)
	: id(_id),version(WMS_PROTOCOL_VERSION),size(0)
{}

WMSMessageHeader::WMSMessageHeader()
	: WMSMessageHeader(0)
{}