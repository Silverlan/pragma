// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module;

#define RESOURCE_TRANSFER_VERBOSE 0

module pragma.client;

import :client_state;
import :game;
import :model;

void ClientState::StartResourceTransfer()
{
	FileManager::CreatePath(m_svInfo->GetDownloadPath().c_str());

	if(m_client != nullptr)
		m_client->SetTimeoutDuration(0.0); // Disable timeout until resource transfer has been completed
	NetPacket resourceReq;
	resourceReq->Write<bool>(GetConVarBool("cl_allowdownload"));
	SendPacket("resource_begin", resourceReq, pragma::networking::Protocol::SlowReliable);
}

void ClientState::HandleClientResource(NetPacket &packet)
{
	std::string file = packet->ReadString();
	if(!IsValidResource(file)) {
		NetPacket response;
		response->Write<bool>(false);
		SendPacket("resourceinfo_response", response, pragma::networking::Protocol::SlowReliable);
		return;
	}
	auto bDefaultPath = true;
	std::string ext;
	auto fileDst = file;
	if(ufile::get_extension(file, &ext) == true) {
		if(ext == Lua::FILE_EXTENSION_PRECOMPILED) {
			bDefaultPath = false;
			fileDst = m_svInfo->GetDownloadPath() + fileDst; // Lua-files are placed into a sub-directory specific to this server
		}
	}
	if(bDefaultPath == true)
		fileDst = "downloads\\" + fileDst; // Files are placed into 'downloads' directory by default (Which is automatically mounted)

	FileManager::CreatePath(fileDst.substr(0, fileDst.find_last_of('\\')).c_str());
	auto size = packet->Read<UInt64>();
	Con::ccl << "Downloading file '" << file << "' (" << util::get_pretty_bytes(size) << ")..." << Con::endl;
	auto f = FileManager::OpenFile(file.c_str(), "rb"); //,fsys::SearchFlags::Local);
	NetPacket response;
	bool bSkip = false;
	if(f != nullptr) {
		if(f->GetSize() == size) {
			bSkip = true;
			Con::ccl << "File '" << file << "' doesn't differ from server's. Skipping..." << Con::endl;
			f.reset();
			response->Write<bool>(false);
		}
		else
			f = nullptr;
	}
	if(bSkip == false)
		f = FileManager::OpenFile((fileDst + ".part").c_str(), "wb");
	if(!bSkip) {
		if(f == nullptr) {
			response->Write<bool>(false);
			Con::cwar << Con::PREFIX_CLIENT << "[ResourceManager] Unable to write file '" << fileDst << "'. Skipping..." << Con::endl;
		}
		else {
			response->Write<bool>(true);
			m_resDownload = std::make_unique<ResourceDownload>(std::static_pointer_cast<VFilePtrInternalReal>(f), fileDst, CUInt32(size));
		}
	}
	SendPacket("resourceinfo_response", response, pragma::networking::Protocol::SlowReliable);
}

void ClientState::HandleClientResourceFragment(NetPacket &packet)
{
	if(m_resDownload == nullptr)
		return;
	std::array<uint8_t, RESOURCE_TRANSFER_FRAGMENT_SIZE> buf;
	unsigned int read = packet->Read<unsigned int>();
	packet->Read(buf.data(), read);
	auto &res = m_resDownload;
	auto f = res->file;
	f->Write(buf.data(), read);
	NetPacket resourceReq;
#if RESOURCE_TRANSFER_VERBOSE == 1
	Con::ccl << "[ResourceManager] " << ((f->Tell() / float(res->size)) * 100) << "%" << Con::endl;
#endif
	if(read < RESOURCE_TRANSFER_FRAGMENT_SIZE) {
		auto resName = res->name;
		res = nullptr;
		f = nullptr;
		resourceReq->Write<bool>(true);

		if((FileManager::Exists(resName.c_str()) == true && FileManager::RemoveFile(resName.c_str()) == false) || FileManager::RenameFile((resName + ".part").c_str(), resName.c_str()) == false)
			Con::ccl << "File '" << (resName + ".part") << "' successfully received, but unable to rename to '" << resName << "'... Requesting next..." << Con::endl;
		else
			Con::ccl << "File '" << resName << "' successfully received... Requesting next..." << Con::endl;
	}
	else
		resourceReq->Write<bool>(false);
	SendPacket("resource_request", resourceReq, pragma::networking::Protocol::SlowReliable);
}
