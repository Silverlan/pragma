#ifndef __CLIENTSESSIONINFO_H__
#define __CLIENTSESSIONINFO_H__

#include "pragma/networkdefinitions.h"
#include "pragma/serverdefinitions.h"
#include <vector>
#include "pragma/networking/resource.h"

class SGame;
class EntityHandle;
namespace pragma {class SPlayerComponent;};
#pragma warning(push)
#pragma warning(disable : 4251)
class DLLSERVER ClientSessionInfo
{
public:
	ClientSessionInfo(pragma::SPlayerComponent *player=nullptr);
	~ClientSessionInfo();
	enum class TransferState : uint32_t
	{
		Initial = 0,
		Started = 1,
		Complete = 2
	};
private:
	SGame *m_game;
	util::WeakHandle<pragma::SPlayerComponent> m_player = {};
	bool m_bTransferring;
	std::vector<std::shared_ptr<Resource>> m_resourceTransfer;
	TransferState m_initialResourceTransferState = TransferState::Initial;
public:
	ClientSessionInfo(const ClientSessionInfo&)=delete;
	ClientSessionInfo &operator=(const ClientSessionInfo&)=delete;
	pragma::SPlayerComponent *GetPlayer();
	void SetPlayer(pragma::SPlayerComponent *pl);
	const std::vector<std::shared_ptr<Resource>> &GetResourceTransfer() const;
	bool AddResource(const std::string &fileName,bool stream=true);
	void RemoveResource(uint32_t i);
	void ClearResourceTransfer();
	bool IsInitialResourceTransferComplete() const;
	void SetInitialResourceTransferState(TransferState state);
	TransferState GetInitialResourceTransferState();
	void SetTransferComplete(bool b);
	bool IsTransferring() const;
};
#pragma warning(pop)

#endif