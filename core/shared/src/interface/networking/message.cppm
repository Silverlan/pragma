// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

#include "definitions.hpp"

export module pragma.shared:networking.message;

export import std.compat;
export import :networking.enums;

export namespace pragma::networking {
	using MessageId = uint32_t;
	struct DLLNETWORK Message {
		static std::unique_ptr<Message> Create(MessageId msgId, std::pmr::synchronized_pool_resource &pool, void *messageBuffer, size_t size, Protocol protocol);
		~Message();

		template<typename T>
		Message &operator<<(const T &value);
		template<typename T>
		const Message &operator>>(T &value);

		bool Read(void *data, size_t size);
		bool Read(void *data, size_t size, size_t offset) const;
		bool Write(const void *data, size_t size);

		uint8_t *GetDataPointer(size_t offset = 0);
		const uint8_t *GetDataPointer(size_t offset = 0) const;

		MessageId GetMessageId() const;
		Protocol GetProtocol() const;
		size_t GetSize() const;
	  private:
		Message(MessageId msgId, std::pmr::synchronized_pool_resource &pool, void *messageBuffer, size_t size);
		Protocol m_protocol = Protocol::SlowReliable;
		uint8_t *m_messageBuffer = nullptr;
		std::pmr::synchronized_pool_resource &m_pool;
		size_t m_pos = 0;
		size_t m_size = 0;
		MessageId m_messageId = std::numeric_limits<MessageId>::max();
	};

	template<typename T>
	Message &Message::operator<<(const T &value)
	{
		if(!Write(&value, sizeof(T)))
			throw std::runtime_error {"Unable to write value: Out of memory"};
		return *this;
	}
	template<typename T>
	const Message &Message::operator>>(T &value)
	{
		if(!Read(&value, sizeof(T)))
			throw std::runtime_error {"Unable to read value: Out of memory bounds"};
		return *this;
	}

	struct DLLNETWORK MessageReader {
		MessageReader(const Message &msg) : m_message {msg} {}

		template<typename T>
		const MessageReader &operator>>(T &value);

		bool Read(void *data, size_t size);
	  private:
		const Message &m_message;
		size_t m_pos = 0;
	};

	template<typename T>
	const MessageReader &MessageReader::operator>>(T &value)
	{
		auto res = Read(&value, sizeof(T));
		if(!res)
			throw std::runtime_error {"Unable to read value: Out of memory bounds"};
		return *this;
	}

	// For backwards compatibility. These should be avoided.
	DLLNETWORK NetPacket message_to_net_packet(const Message &msg);
	DLLNETWORK void net_packet_to_message(const NetPacket &packet, Message &msg);
}
