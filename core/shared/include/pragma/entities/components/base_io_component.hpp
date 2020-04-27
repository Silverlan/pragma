/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __BASE_IO_COMPONENT_HPP__
#define __BASE_IO_COMPONENT_HPP__

#include "pragma/entities/components/base_entity_component.hpp"
#include "pragma/entities/output.h"
#include <sharedutils/property/util_property.hpp>

struct AttachmentData;
namespace pragma
{
	struct DLLNETWORK CEInputData
		: public ComponentEvent
	{
		CEInputData(const std::string &input,BaseEntity *activator,BaseEntity *caller,const std::string &data);
		virtual void PushArguments(lua_State *l) override;
		const std::string &input;
		BaseEntity * const activator;
		BaseEntity * const caller;
		const std::string &data;
	};
	class DLLNETWORK BaseIOComponent
		: public BaseEntityComponent
	{
	public:
		static pragma::ComponentEventId EVENT_HANDLE_INPUT;
		static void RegisterEvents(pragma::EntityComponentManager &componentManager);
		virtual void Initialize() override;

		void StoreOutput(std::string name,std::string entities,std::string input,std::string param,float delay=0.f,int times=-1);
		void StoreOutput(std::string name,std::string info);
		void TriggerOutput(std::string name,BaseEntity *activator);

		virtual void Input(std::string input,BaseEntity *activator,BaseEntity *caller,std::string data);
		void Input(const std::string input,BaseEntity *activator=nullptr,BaseEntity *caller=nullptr);
	protected:
		BaseIOComponent(BaseEntity &ent);
		bool FireSingleOutput(Output &output,BaseEntity *activator);

		std::unordered_map<std::string,std::vector<Output>> m_outputs;
	};
};

#endif
