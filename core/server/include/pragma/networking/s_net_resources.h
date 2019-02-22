#ifndef __S_NET_RESOURCES_H__
#define __S_NET_RESOURCES_H__
#include "pragma/serverdefinitions.h"
#include "pragma/networkdefinitions.h"
#include "pragma/networking/netmessages.h"
DLLSERVER void NET_sv_resourceinfo_response(WVServerClient *session,NetPacket packet);
DLLSERVER void NET_sv_resource_request(WVServerClient *session,NetPacket packet);
DLLSERVER void NET_sv_resource_begin(WVServerClient *session,NetPacket packet);
DLLSERVER void NET_sv_query_resource(WVServerClient *session,NetPacket packet);
DLLSERVER void NET_sv_query_model_texture(WVServerClient *session,NetPacket packet);
REGISTER_NETMESSAGE_SV(resourceinfo_response,NET_sv_resourceinfo_response);
REGISTER_NETMESSAGE_SV(resource_request,NET_sv_resource_request);
REGISTER_NETMESSAGE_SV(resource_begin,NET_sv_resource_begin);
REGISTER_NETMESSAGE_SV(query_resource,NET_sv_query_resource);
REGISTER_NETMESSAGE_SV(query_model_texture,NET_sv_query_model_texture);
#endif