// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

module pragma.client;

import :client_state;
import :game;
import :model;

#define RESOURCE_TRANSFER_VERBOSE 0

void pragma::ClientState::StartResourceTransfer()
{
	fs::create_path(m_svInfo->GetDownloadPath());

	if(m_client != nullptr)
		m_client->SetTimeoutDuration(0.0); // Disable timeout until resource transfer has been completed
	NetPacket resourceReq;
	resourceReq->Write<bool>(GetConVarBool("cl_allowdownload"));
	SendPacket(networking::net_messages::server::RESOURCE_BEGIN, resourceReq, networking::Protocol::SlowReliable);
}

void pragma::ClientState::HandleClientResource(NetPacket &packet)
{
	std::string file = packet->ReadString();
	if(!networking::is_valid_resource(file)) {
		NetPacket response;
		response->Write<bool>(false);
		SendPacket(networking::net_messages::server::RESOURCEINFO_RESPONSE, response, networking::Protocol::SlowReliable);
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

	fs::create_path(fileDst.substr(0, fileDst.find_last_of('\\')));
	auto size = packet->Read<UInt64>();
	Con::CCL << "Downloading file '" << file << "' (" << util::get_pretty_bytes(size) << ")..." << Con::endl;
	auto f = pragma::fs::open_file(file.c_str(), fs::FileMode::Read | fs::FileMode::Binary); //,fs::SearchFlags::Local);
	NetPacket response;
	bool bSkip = false;
	if(f != nullptr) {
		if(f->GetSize() == size) {
			bSkip = true;
			Con::CCL << "File '" << file << "' doesn't differ from server's. Skipping..." << Con::endl;
			f.reset();
			response->Write<bool>(false);
		}
		else
			f = nullptr;
	}
	if(bSkip == false)
		f = pragma::fs::open_file((fileDst + ".part").c_str(), fs::FileMode::Write | fs::FileMode::Binary);
	if(!bSkip) {
		if(f == nullptr) {
			response->Write<bool>(false);
			Con::CWAR << Con::PREFIX_CLIENT << "[ResourceManager] Unable to write file '" << fileDst << "'. Skipping..." << Con::endl;
		}
		else {
			response->Write<bool>(true);
			m_resDownload = std::make_unique<ResourceDownload>(std::static_pointer_cast<fs::VFilePtrInternalReal>(f), fileDst, CUInt32(size));
		}
	}
	SendPacket(networking::net_messages::server::RESOURCEINFO_RESPONSE, response, networking::Protocol::SlowReliable);
}

void pragma::ClientState::HandleClientResourceFragment(NetPacket &packet)
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
	Con::CCL << "[ResourceManager] " << ((f->Tell() / float(res->size)) * 100) << "%" << Con::endl;
#endif
	if(read < RESOURCE_TRANSFER_FRAGMENT_SIZE) {
		auto resName = res->name;
		res = nullptr;
		f = nullptr;
		resourceReq->Write<bool>(true);

		if((fs::exists(resName.c_str()) == true && fs::remove_file(resName) == false) || fs::rename_file((resName + ".part"), resName) == false)
			Con::CCL << "File '" << (resName + ".part") << "' successfully received, but unable to rename to '" << resName << "'... Requesting next..." << Con::endl;
		else
			Con::CCL << "File '" << resName << "' successfully received... Requesting next..." << Con::endl;
	}
	else
		resourceReq->Write<bool>(false);
	SendPacket(networking::net_messages::server::RESOURCE_REQUEST, resourceReq, networking::Protocol::SlowReliable);
}
