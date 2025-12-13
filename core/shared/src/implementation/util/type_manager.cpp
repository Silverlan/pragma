// SPDX-FileCopyrightText: (c) 2019 Silverlan <opensource@pragma-engine.com>
// SPDX-License-Identifier: MIT
module;

module pragma.shared;

import :util.type_manager;

pragma::util::BaseNamedType::BaseNamedType(TypeId id, const std::string &name) : m_id {id}, m_name {name} {}
pragma::util::TypeId pragma::util::BaseNamedType::GetId() const { return m_id; }
const std::string &pragma::util::BaseNamedType::GetName() const { return m_name; }
