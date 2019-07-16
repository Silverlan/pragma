#include "pragma/util/util_typed_manager.hpp"

pragma::BaseNamedType::BaseNamedType(TypeId id,const std::string &name)
	: m_id{id},m_name{name}
{}
pragma::TypeId pragma::BaseNamedType::GetId() const {return m_id;}
const std::string &pragma::BaseNamedType::GetName() const {return m_name;}
