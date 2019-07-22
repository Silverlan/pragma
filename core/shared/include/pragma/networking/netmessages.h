#ifndef __NETMESSAGES_H__
#define __NETMESSAGES_H__
#include "pragma/networkdefinitions.h"
#include <sharedutils/netpacket.hpp>
#include <networkmanager/nwm_message.h>
#include <pragma/engine.h>

namespace pragma::networking {class IServerClient;};
#define COMMA ,
class DLLNETWORK SVNetMessage
{
public:
	void (*handler)(pragma::networking::IServerClient&,NetPacket);
	unsigned int ID;
};

class DLLNETWORK CLNetMessage
{
public:
	void (*handler)(NetPacket);
	unsigned int ID;
};

template<class T>
	class NetMessageMap
{
public:
	NetMessageMap();
protected:
	std::unordered_map<unsigned int,T> m_netMessages;
	std::unordered_map<std::string,unsigned int> m_netMessageIDs;
	unsigned int m_messageID;
public:
	virtual void RegisterNetMessage(std::string name,void (*handler)(NetPacket));
	virtual void PreRegisterNetMessage(std::string name,void (*handler)(NetPacket));
	virtual void RegisterNetMessage(std::string name,void (*handler)(pragma::networking::IServerClient&,NetPacket));
	virtual void PreRegisterNetMessage(std::string name,void (*handler)(pragma::networking::IServerClient&,NetPacket));
	void GetNetMessages(std::unordered_map<unsigned int,T> **messages);
	void GetNetMessages(std::unordered_map<std::string,unsigned int> **messages);
	T *GetNetMessage(unsigned int ID);
	T *GetNetMessage(std::string identifier);
	unsigned int GetNetMessageID(std::string identifier);
};

template<class T>
	NetMessageMap<T>::NetMessageMap()
	: m_messageID(NWM_MESSAGE_RESERVED +1)
{}

template<class T> void NetMessageMap<T>::RegisterNetMessage(std::string name,void (*)(NetPacket)) {}
template<class T> void NetMessageMap<T>::RegisterNetMessage(std::string name,void (*)(pragma::networking::IServerClient&,NetPacket)) {}
template<class T> void NetMessageMap<T>::PreRegisterNetMessage(std::string name,void (*)(NetPacket)) {}
template<class T> void NetMessageMap<T>::PreRegisterNetMessage(std::string name,void (*)(pragma::networking::IServerClient&,NetPacket)) {}

template<class T>
	void NetMessageMap<T>::GetNetMessages(std::unordered_map<unsigned int,T> **messages) {*messages = &m_netMessages;}

template<class T>
	void NetMessageMap<T>::GetNetMessages(std::unordered_map<std::string,unsigned int> **messages) {*messages = &m_netMessageIDs;}

template<class T>
	T *NetMessageMap<T>::GetNetMessage(unsigned int ID)
{
	typename std::unordered_map<unsigned int,T>::iterator i = m_netMessages.find(ID);
	if(i == m_netMessages.end())
		return NULL;
	return &i->second;
}

template<class T>
	T *NetMessageMap<T>::GetNetMessage(std::string identifier)
{
	unsigned int ID = GetNetMessageID(identifier);
	if(ID == 0)
		return NULL;
	return GetNetMessage(ID);
}

template<class T>
	unsigned int NetMessageMap<T>::GetNetMessageID(std::string identifier)
{
	typename std::unordered_map<std::string,unsigned int>::iterator i = m_netMessageIDs.find(identifier);
	if(i == m_netMessageIDs.end())
		return 0;
	return i->second;
}



class DLLNETWORK ClientMessageMap
	: public NetMessageMap<CLNetMessage>
{
public:
	void RegisterNetMessage(std::string name,void (*handler)(NetPacket));
	void PreRegisterNetMessage(std::string name,void (*handler)(NetPacket));
};

class DLLNETWORK ServerMessageMap
	: public NetMessageMap<SVNetMessage>
{
public:
	void RegisterNetMessage(std::string name,void (*handler)(pragma::networking::IServerClient&,NetPacket));
	void PreRegisterNetMessage(std::string name,void (*handler)(pragma::networking::IServerClient&,NetPacket));
};

#define net_newglobal_dec(glname,args) \
	DLLNETWORK void Register##glname##NetMessage(std::string name,void (*handler)(args)); \
	DLLNETWORK glname##MessageMap *Get##glname##MessageMap(); \
	class DLLNETWORK __reg_net_##glname \
	{ \
	public: \
		__reg_net_##glname(std::string name,void (*handler)(args)) \
		{ \
			Register##glname##NetMessage(name,handler); \
			delete this; \
		} \
	};

net_newglobal_dec(Server,pragma::networking::IServerClient& COMMA NetPacket);
net_newglobal_dec(Client,NetPacket);

#define _REGISTER_NETMESSAGE_SV(msgName,handler) \
	__reg_net_Server *__reg_net_Server_##msgName = new __reg_net_Server(#msgName,handler);

#define _REGISTER_NETMESSAGE_CL(msgName,handler) \
	__reg_net_Client *__reg_net_Client_##msgName = new __reg_net_Client(#msgName,handler);

#ifdef ENGINE_NETWORK // NetMessages are only DECLARED (Unless they're shared) within shared.dll, the function definitions must be done in a different library
	#define REGISTER_NETMESSAGE_SV(msgName,handler) \
		__reg_net_Server *__reg_net_Server_##msgName = new __reg_net_Server(#msgName,NULL);

	#define REGISTER_NETMESSAGE_CL(msgName,handler) \
		__reg_net_Client *__reg_net_Client_##msgName = new __reg_net_Client(#msgName,NULL);
#else
	#define REGISTER_NETMESSAGE_SV(msgName,handler) \
		_REGISTER_NETMESSAGE_SV(msgName,handler);
	#define REGISTER_NETMESSAGE_CL(msgName,handler) \
		_REGISTER_NETMESSAGE_CL(msgName,handler);
#endif
#endif