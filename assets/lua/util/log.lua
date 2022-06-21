--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

pfm = pfm or {}

pfm.LOG_SEVERITY_NORMAL = log.SEVERITY_INFO
pfm.LOG_SEVERITY_WARNING = log.SEVERITY_WARNING
pfm.LOG_SEVERITY_ERROR = log.SEVERITY_ERROR
pfm.LOG_SEVERITY_CRITICAL = log.SEVERITY_CRITICAL

pfm.MAX_LOG_CATEGORIES = 30
local g_enabledCategories = bit.lshift(1,pfm.MAX_LOG_CATEGORIES) -1 -- Enable all call categories by default
pfm.is_log_category_enabled = function(categories)
	return categories == 0 or bit.band(categories,g_enabledCategories) ~= 0
end

pfm.set_log_category_enabled = function(category,enabled)
	g_enabledCategories = math.set_flag_enabled(g_enabledCategories,category,enabled)
end

pfm.set_enabled_log_categories = function(categories)
	g_enabledCategories = categories
end

pfm.log = function(msg,categories,severity)
	severity = severity or pfm.LOG_SEVERITY_NORMAL
	categories = categories or pfm.LOG_CATEGORY_PFM
	if(pfm.is_log_category_enabled(categories) == false) then return false end
	msg = "[PFM] " .. msg
	if(severity == pfm.LOG_SEVERITY_NORMAL) then console.print_messageln(msg)
	elseif(severity == pfm.LOG_SEVERITY_WARNING) then console.print_warning(msg)
	elseif(severity == pfm.LOG_SEVERITY_ERROR) then console.print_error(msg)
	elseif(severity == pfm.LOG_SEVERITY_CRITICAL) then console.print_error(msg)
	else return false end
	return true
end

pfm.error = function(msg)
	local severity = pfm.LOG_SEVERITY_ERROR
	local category = 0
	local r = pfm.log(msg,category,severity)
	error(msg)
	return r
end

local g_logCategories = {}
pfm.register_log_category = function(name)
	local catName = "LOG_CATEGORY_" .. name:upper()
	if(pfm[catName] ~= nil) then return pfm[catName] end
	if(#g_logCategories >= pfm.MAX_LOG_CATEGORIES) then
		console.print_warning("Unable to register log category '" .. name .. "': Max log category count of " .. pfm.MAX_LOG_CATEGORIES .. " has been exceeded!")
		return -1
	end
	local catId = bit.lshift(1,#g_logCategories)
	table.insert(g_logCategories,{
		name = name
	})
	pfm[catName] = catId
	return catId
end

-- TODO: Move these to filmmaker addon
pfm.register_log_category("pfm")
pfm.register_log_category("pfm_game")
pfm.register_log_category("pfm_interface")
pfm.register_log_category("pfm_render")
pfm.register_log_category("pfm_cache")
pfm.register_log_category("pfm_unirender")
pfm.register_log_category("pfm_bake")

log = log or {}
log.register_category = pfm.register_log_category
log.is_category_enabled = pfm.is_log_category_enabled
log.set_category_enabled = pfm.set_log_category_enabled
log.set_enabled_log_categories = pfm.set_enabled_log_categories
log.msg = pfm.log
log.err = pfm.error

console.register_command("log",function(pl,...)
	local categories = {...}
	if(#categories == 0) then
		print("Currently enabled log categories:")
		for _,logCat in ipairs(g_logCategories) do
			if(log.is_category_enabled(pfm["LOG_CATEGORY_" .. logCat.name:upper()])) then
				print(logCat.name)
			end
		end
		return
	end
	local logCategories = 0
	for _,catName in ipairs(categories) do
		if(catName:lower() == "all") then
			logCategories = bit.lshift(1,pfm.MAX_LOG_CATEGORIES) -1
			break
		end
		if(pfm["LOG_CATEGORY_" .. catName:upper()] ~= nil) then
			logCategories = bit.bor(logCategories,pfm["LOG_CATEGORY_" .. catName:upper()])
		else
			console.print_warning("Unknown log category '" .. catName .. "'! Ignoring...")
		end
	end
	pfm.set_enabled_log_categories(logCategories)
end)
