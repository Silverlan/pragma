// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT

#include "pragma/networking/ip_address.hpp"

pragma::networking::IPAddress::IPAddress() : IPAddress {"", 0} {}
pragma::networking::IPAddress::IPAddress(const std::string &ip, uint16_t port) : m_ip {ip}, m_port {port} {}

std::string pragma::networking::IPAddress::ToString() const { return m_ip; }
const std::string &pragma::networking::IPAddress::GetIP() const { return m_ip; }
uint16_t pragma::networking::IPAddress::GetPort() const { return m_port; }
