--[[
    Copyright (C) 2019  Florian Weischer

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util.register_class_property = function(class,name,default,settings)
	local postfix = name:sub(0,1):upper() .. name:sub(2)
	local memberName = "m_prop" .. postfix
	local setterName = "Set" .. postfix
	local getterName = "Get" .. postfix
	if(settings ~= nil) then
		setterName = settings.setter or setterName
		getterName = settings.getter or getterName
	end
	class[setterName] = function(self,value)
		self[memberName] = value
	end
	class[getterName] = function(self)
		if(self[memberName] == nil) then return default end
		return self[memberName]
	end
end
