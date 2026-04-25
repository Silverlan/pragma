// SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :networking.message;

std::unique_ptr<pragma::networking::Message> pragma::networking::Message::Create(MessageId msgId, std::pmr::synchronized_pool_resource &pool, void *messageBuffer, size_t size, Protocol protocol)
{
	auto msg = std::unique_ptr<Message> {new Message {msgId, pool, messageBuffer, size}};
	if(!msg)
		return nullptr;
	msg->m_protocol = protocol;
	return msg;
}
pragma::networking::Message::~Message() { m_pool.deallocate(m_messageBuffer, m_size); }

bool pragma::networking::Message::Read(void *data, size_t size)
{
	auto res = Read(data, size, m_pos);
	if(!res)
		return false;
	m_pos += size;
	return true;
}

bool pragma::networking::Message::Read(void *data, size_t size, size_t offset) const
{
	if(offset + size > m_size)
		return false;
	auto *ptr = GetDataPointer(offset);
	memcpy(data, ptr, size);
	return true;
}

bool pragma::networking::Message::Write(const void *data, size_t size)
{
	if(m_pos + size > m_size)
		return false;
	auto *ptr = GetDataPointer(m_pos);
	memcpy(ptr, data, size);
	m_pos += size;
	return true;
}

uint8_t *pragma::networking::Message::GetDataPointer(size_t offset) { return m_messageBuffer + offset; }
const uint8_t *pragma::networking::Message::GetDataPointer(size_t offset) const { return const_cast<Message *>(this)->GetDataPointer(offset); }

pragma::networking::MessageId pragma::networking::Message::GetMessageId() const { return m_messageId; }
pragma::networking::Protocol pragma::networking::Message::GetProtocol() const { return m_protocol; }
size_t pragma::networking::Message::GetSize() const { return m_size; }

pragma::networking::Message::Message(MessageId msgId, std::pmr::synchronized_pool_resource &pool, void *messageBuffer, size_t size) : m_pool {pool}, m_messageBuffer {static_cast<uint8_t *>(messageBuffer)}, m_size {size}, m_messageId {msgId} {}

bool pragma::networking::MessageReader::Read(void *data, size_t size)
{
	auto res = m_message.Read(data, size, m_pos);
	if(!res)
		return false;
	m_pos += size;
	return true;
}

NetPacket pragma::networking::message_to_net_packet(const Message &msg)
{
	NetPacket packet;
	packet.SetMessageID(msg.GetMessageId());
	packet->Write(msg.GetDataPointer(), msg.GetSize());
	packet->SetOffset(0);
	return packet;
}
void pragma::networking::net_packet_to_message(const NetPacket &packet, Message &msg)
{
	//
}
