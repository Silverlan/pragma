#ifndef __GLOBAL_COMPONENT_HPP__
#define __GLOBAL_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"

namespace pragma
{
	// Note: 'GlobalComponent' cannot be used as name, otherwise there are conflicts with 'GlobalHandle' class of winbase.h
	class DLLNETWORK GlobalNameComponent final
		: public BaseEntityComponent
	{
	public:
		GlobalNameComponent(BaseEntity &ent);
		virtual void Initialize() override;

		virtual void Save(DataStream &ds) override;
		virtual void Load(DataStream &ds,uint32_t version) override;

		const std::string &GetGlobalName() const;
		void SetGlobalName(const std::string &name);

		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	protected:
		std::string m_globalName = {};
	};
};

#endif
