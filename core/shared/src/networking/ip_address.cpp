/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "pragma/networking/ip_address.hpp"

pragma::networking::IPAddress::IPAddress() : IPAddress {"", 0} {}
pragma::networking::IPAddress::IPAddress(const std::string &ip, uint16_t port) : m_ip {ip}, m_port {port} {}

std::string pragma::networking::IPAddress::ToString() const { return m_ip; }
const std::string &pragma::networking::IPAddress::GetIP() const { return m_ip; }
uint16_t pragma::networking::IPAddress::GetPort() const { return m_port; }
