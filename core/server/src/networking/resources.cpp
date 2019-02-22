#include "stdafx_server.h"
#include <pragma/serverstate/serverstate.h>
/*
void ServerState::HandleNextResource(ClientSession *session)
{
	std::unordered_map<ClientSession*,std::vector<Resource*>*>::iterator i = m_resourceTransfer.find(session);
	if(i == m_resourceTransfer.end())
	{
		std::vector<std::string> *resources;
		ResourceManager::GetResources(&resources);
		std::vector<Resource*> *res = new std::vector<Resource*>;
		for(int i=0;i<resources->size();i++)
		{
			Resource *r = new Resource((*resources)[i]);
			res->push_back(r);
		}
		m_resourceTransfer.insert(std::pair<ClientSession*,std::vector<Resource*>*>(session,res));
		i = m_resourceTransfer.find(session);
	}
	else if((*i->second).size() > 0)
	{
		FileManager::CloseFile((*i->second)[0]->name);
		delete (*i->second)[0];
		i->second->erase(i->second->begin());
	}
	if((*i->second).size() == 0)
	{
		Con::cout<<"All resources have been sent!"<<Con::endl;
		delete i->second;
		m_resourceTransfer.erase(i);
		NetPacket p;
		SendTCPMessageToClient("resourcecomplete",&p,session);
		return;
	}
	Resource *r = (*i->second)[0];
	FILE *f = FileManager::OpenFile(r->name,"rb");
	if(f == NULL)
	{
		Con::cout<<"WARNING: (SERVER) [ResourceManager] Unable to open file '"<<r->name<<"'. Skipping..."<<Con::endl;
		HandleServerNextResource(session);
		return;
	}
	unsigned long long size = FileManager::GetFileSize(f);
	NetPacket packetRes;
	packetRes.WriteString(r->name);
	packetRes.Write<unsigned long long>(size);
	SendTCPMessageToClient("resourceinfo",&packetRes,session);
}

void ServerState::HandleResourceStart(ClientSession *session,NetPacket *packet)
{
	std::unordered_map<ClientSession*,std::vector<Resource*>*>::iterator i = m_resourceTransfer.find(session);
	if(i == m_resourceTransfer.end())
		return;
	bool send = packet->Read<bool>();
	if(send)
	{
		Con::cout<<"Sending file '"<<(*i->second)[0]->name<<"' to client "<<session->GetClientAddress()<<Con::endl;
		HandleServerResourceFragment(session);
	}
	else
		HandleServerNextResource(session);
}

void ServerState::HandleResourceFragment(ClientSession *session)
{
	std::unordered_map<ClientSession*,std::vector<Resource*>*>::iterator i = m_resourceTransfer.find(session);
	if(i == m_resourceTransfer.end())
		return;
	Resource *r = (*i->second)[0];
	FILE *f = FileManager::OpenFile(r->name,"rb");
	if(f == NULL)
	{
		Con::cout<<"WARNING: (SERVER) [ResourceManager] Unable to open file '"<<r->name<<"'. Skipping..."<<Con::endl;
		HandleServerNextResource(session); // TODO: Close and delete file on client
		return;
	}
	fseek(f,r->offset,SEEK_SET);
	NetPacket fragment;
	char buf[1400];
	unsigned long long size = FileManager::GetFileSize(f);
	unsigned int read = size -r->offset;
	if(read > 1400)
		read = 1400;
	fread(buf,read,1,f);
	fragment.Write<unsigned int>(read);
	fragment.Write(buf,read);
	r->offset += read;
	SendTCPMessageToClient("resource_fragment",&fragment,session);
}

DLLSERVER void NET_sv_resource_request(ClientSession *session,NetPacket *packet)
{
	bool b = packet->Read<bool>();
	if(b)
		server->HandleServerNextResource(session);
	else
		server->HandleServerResourceFragment(session);
}

DLLSERVER void NET_sv_resource_begin(ClientSession *session,NetPacket *packet)
{
	bool bSend = packet->Read<bool>() && server->GetConVarBool("sv_allowdownload");
	if(bSend)
		server->HandleServerNextResource(session);
	else
	{
		NetPacket p;
		server->SendTCPMessageToClient("resourcecomplete",&p,session);
	}
}

DLLSERVER void NET_sv_resourceinfo_response(ClientSession *session,NetPacket *packet) {server->HandleServerResourceStart(session,packet);}
*/ // WEAVETODO