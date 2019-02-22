#include "stdafx_server.h"
#include "pragma/networking/clientsessioninfo.h"
#include "pragma/entities/player.h"
#include <pragma/entities/baseentity_handle.h>
#include "pragma/entities/components/s_player_component.hpp"

extern DLLSERVER ServerState *server;

ClientSessionInfo::ClientSessionInfo(pragma::SPlayerComponent *player)
	: m_bTransferring(false)
{
	SetPlayer(player);
}
ClientSessionInfo::~ClientSessionInfo() {ClearResourceTransfer();}
pragma::SPlayerComponent *ClientSessionInfo::GetPlayer() {return m_player.get();}
void ClientSessionInfo::SetTransferComplete(bool b) {m_bTransferring = !b;}
bool ClientSessionInfo::IsTransferring() const {return m_bTransferring;}
bool ClientSessionInfo::IsInitialResourceTransferComplete() const {return (m_initialResourceTransferState == TransferState::Complete) ? true : false;}
void ClientSessionInfo::SetInitialResourceTransferState(TransferState state) {m_initialResourceTransferState = state;}
ClientSessionInfo::TransferState ClientSessionInfo::GetInitialResourceTransferState() {return m_initialResourceTransferState;}
void ClientSessionInfo::SetPlayer(pragma::SPlayerComponent *pl)
{
	m_player = (pl != nullptr) ? util::WeakHandle<pragma::SPlayerComponent>(std::static_pointer_cast<pragma::SPlayerComponent>(pl->shared_from_this())) : util::WeakHandle<pragma::SPlayerComponent>{};
}
const std::vector<std::shared_ptr<Resource>> &ClientSessionInfo::GetResourceTransfer() const {return m_resourceTransfer;}
bool ClientSessionInfo::AddResource(const std::string &fileName,bool stream)
{
	auto canonName = FileManager::GetCanonicalizedPath(fileName);
	auto it = std::find_if(m_resourceTransfer.begin(),m_resourceTransfer.end(),[canonName](const std::shared_ptr<Resource> &res) {
		return (res->name == canonName) ? true : false;
	});
	if(it != m_resourceTransfer.end())
	{
		if(stream == false)
			return true;
		(*it)->stream = stream;
		return true;
	}
	auto res = std::make_shared<Resource>(canonName,stream);
	if(res->Construct() == false)
		return false;
	it = m_resourceTransfer.end();
	if(stream == false)
	{
		// Insert new resource before streamed resources
		it = m_resourceTransfer.begin();
		for(auto itRes=m_resourceTransfer.begin();itRes!=m_resourceTransfer.end();++itRes)
		{
			if((*itRes)->stream == stream)
				it = itRes +1;
			else
				break;
		}
	}
	m_resourceTransfer.insert(it,res);
	return true;
}
void ClientSessionInfo::RemoveResource(uint32_t i)
{
	m_resourceTransfer.erase(m_resourceTransfer.begin() +i);
}
void ClientSessionInfo::ClearResourceTransfer()
{
	m_resourceTransfer.clear();
}
