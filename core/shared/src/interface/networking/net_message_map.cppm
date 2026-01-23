// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:networking.net_message_map;

#define COMMA ,

export import pragma.network_manager;

export import std;

export {
	namespace pragma::networking {
		using ServerClientHandle = void *;
		class DLLNETWORK SVNetMessage {
		public:
			using Handler = void (*)(ServerClientHandle, NetPacket);
			Handler handler;
			unsigned int ID;
		};

		class DLLNETWORK CLNetMessage {
		public:
			using Handler = void (*)(NetPacket);
			Handler handler;
			unsigned int ID;
		};

		class IBaseNetMessageMap {
		  public:
			virtual void RegisterNetMessage(const std::string_view &name) = 0;
		};

		template<class T>
		class NetMessageMap : public IBaseNetMessageMap {
		  public:
			NetMessageMap();
		  public:
			void GetNetMessages(std::unordered_map<unsigned int, T> **messages);
			void GetNetMessages(util::StringMap<unsigned int> **messages);
			T *GetNetMessage(unsigned int ID);
			T *GetNetMessage(std::string identifier);
			unsigned int GetNetMessageID(std::string identifier);

			void RegisterNetMessage(const std::string_view &name) override;
			void RegisterNetMessage(const std::string_view &name, const T::Handler &handler);
		  protected:
			std::unordered_map<unsigned int, T> m_netMessages;
			util::StringMap<unsigned int> m_netMessageIDs;
			unsigned int m_messageID;
		};

		template<class T>
		NetMessageMap<T>::NetMessageMap() : m_messageID(NWM_MESSAGE_RESERVED + 1)
		{
		}

		template<class T>
		void NetMessageMap<T>::GetNetMessages(std::unordered_map<unsigned int, T> **messages)
		{
			*messages = &m_netMessages;
		}

		template<class T>
		void NetMessageMap<T>::GetNetMessages(util::StringMap<unsigned int> **messages)
		{
			*messages = &m_netMessageIDs;
		}

		template<class T>
		T *NetMessageMap<T>::GetNetMessage(unsigned int ID)
		{
			typename std::unordered_map<unsigned int, T>::iterator i = m_netMessages.find(ID);
			if(i == m_netMessages.end())
				return nullptr;
			return &i->second;
		}

		template<class T>
		T *NetMessageMap<T>::GetNetMessage(std::string identifier)
		{
			unsigned int ID = GetNetMessageID(identifier);
			if(ID == 0)
				return nullptr;
			return GetNetMessage(ID);
		}

		template<class T>
		unsigned int NetMessageMap<T>::GetNetMessageID(std::string identifier)
		{
			typename util::StringMap<unsigned int>::iterator i = m_netMessageIDs.find(identifier);
			if(i == m_netMessageIDs.end())
				return 0;
			return i->second;
		}

		template<class T>
		void NetMessageMap<T>::RegisterNetMessage(const std::string_view &name)
		{
			if(m_netMessageIDs.find(name) != m_netMessageIDs.end())
				return;
			T msg;
			msg.ID = m_messageID;
			msg.handler = nullptr;
			m_netMessageIDs.insert(std::make_pair(name, msg.ID));
			m_netMessages.insert(std::make_pair(msg.ID, msg));
			m_messageID++;
		}

		template<class T>
		void NetMessageMap<T>::RegisterNetMessage(const std::string_view &name, const T::Handler &handler)
		{
			util::StringMap<unsigned int>::iterator i = m_netMessageIDs.find(name);
			if(i != m_netMessageIDs.end()) {
				unsigned int ID = i->second;
				auto j = m_netMessages.find(ID);
				if(j != m_netMessages.end())
					j->second.handler = handler;
				return;
			}
			T msg;
			msg.ID = m_messageID;
			msg.handler = handler;
			m_netMessageIDs.insert(std::make_pair(name, msg.ID));
			m_netMessages.insert(std::make_pair(msg.ID, msg));
			m_messageID++;
		}

		using ClientMessageMap = NetMessageMap<CLNetMessage>;
		using ServerMessageMap = NetMessageMap<SVNetMessage>;


		DLLNETWORK ClientMessageMap *get_client_message_map();
		DLLNETWORK ServerMessageMap *get_server_message_map();
		void register_net_messages();
	}
};
