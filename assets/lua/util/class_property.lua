-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class_property = function(class, name, default, settings)
	local postfix = name:sub(0, 1):upper() .. name:sub(2)
	local memberName = "m_prop" .. postfix
	local setterName = "Set" .. postfix
	local getterName = "Get" .. postfix
	if settings ~= nil then
		setterName = settings.setter or setterName
		getterName = settings.getter or getterName
	end
	class[setterName] = function(self, value)
		self[memberName] = value
	end
	class[getterName] = function(self)
		if self[memberName] == nil then
			return default
		end
		return self[memberName]
	end
end
