/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "pragma/util/util_typed_manager.hpp"

pragma::BaseNamedType::BaseNamedType(TypeId id,const std::string &name)
	: m_id{id},m_name{name}
{}
pragma::TypeId pragma::BaseNamedType::GetId() const {return m_id;}
const std::string &pragma::BaseNamedType::GetName() const {return m_name;}
