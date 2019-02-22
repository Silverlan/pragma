#ifndef __BASE_NAME_COMPONENT_HPP__
#define __BASE_NAME_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include <sharedutils/property/util_property.hpp>

namespace pragma
{
	class DLLNETWORK BaseNameComponent
		: public BaseEntityComponent
	{
	public:
		virtual void Initialize() override;

		virtual void SetName(std::string name);
		const std::string &GetName() const;
		const util::PStringProperty &GetNameProperty() const;

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;
	protected:
		BaseNameComponent(BaseEntity &ent);
		util::PStringProperty m_name;
	};
};

#endif
