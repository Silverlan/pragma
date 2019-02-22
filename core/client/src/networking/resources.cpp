#include "stdafx_client.h"
#include "pragma/clientstate/clientstate.h"
/*
void ClientState::HandleClientResource(NetPacket &packet)
{
	std::string file = packet->ReadString();
	if(!ResourceManager::IsValidResource(file))
		return;
	CreatePath(file.substr(0,file.find_last_of('\\')));
	unsigned int size = packet->Read<unsigned int>();
	Con::cout<<"Downloading file '"<<file<<"' ("<<size<<" byte)..."<<Con::endl;
	std::string path = SubPath(file);
	FILE *f;
	NetPacket response;
	bool bSkip = false;
	if(GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES)
	{
		f = fopen(path.c_str(),"rb");
		if(FileManager::GetFileSize(f) == size)
		{
			bSkip = true;
			Con::cout<<"File '"<<file<<"' doesn't differ from server's. Skipping..."<<Con::endl;
			fclose(f);
		}
		else
			f = freopen(NULL,"wb",f);
		response.Write<bool>(false);
	}
	else f = fopen(path.c_str(),"wb");
	if(!bSkip)
	{
		if(f == NULL)
		{
			response.Write<bool>(false);
			Con::cout<<"WARNING: (CLIENT) [ResourceManager] Unable to write file '"<<file<<"'. Skipping..."<<Con::endl;
		}
		else
		{
			response.Write<bool>(true);
			m_resDownload = new ResourceDownload(f,file,size);
		}
	}
	SendTCPMessageToServer("resourceinfo_response",&response);
}

void ClientState::HandleClientResourceFragment(NetPacket &packet)
{
	if(m_resDownload == NULL)
		return;
	char buf[1400];
	unsigned int read = packet->Read<unsigned int>();
	packet->Read(buf,read);
	ResourceDownload *res = m_resDownload;
	FILE *f = res->file;
	fwrite(buf,1,read,f);
	NetPacket resourceReq;
	Con::cout<<((ftell(f) /float(res->size)) *100)<<"%"<<Con::endl;
	if(read < 1400)
	{
		Con::cout<<"File '"<<res->name<<"' successfully received... Requesting next..."<<Con::endl;
		delete res;
		m_resDownload = NULL;
		resourceReq.Write<bool>(true);
	}
	else resourceReq.Write<bool>(false);
	SendTCPMessageToServer("resource_request",&resourceReq);
}

DLLCLIENT void NET_cl_resource_fragment(NetPacket &packet) {client->HandleClientResourceFragment(packet);}

DLLCLIENT void NET_cl_resourceinfo(NetPacket &packet) {client->HandleClientResource(packet);}

DLLCLIENT void NET_cl_resourcecomplete(NetPacket &packet)
{
	Con::cout<<"[CLIENT] All resources have been received!"<<Con::endl;
	Con::cout<<"[CLIENT] Requesting Game Info..."<<Con::endl;
	client->SendUserInfo();
}*/