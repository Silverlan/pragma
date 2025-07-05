-- SPDX-FileCopyrightText: (c) 2024 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

include("/gui/vbox.lua")
include("/gui/pfm/controls_menu/controls_menu.lua")

util.open_generic_window = function(title, onOpen)
	time.create_simple_timer(0.0, function()
		local w = 512
		local h = 512
		local createInfo = prosper.WindowCreateInfo()
		createInfo.width = w
		createInfo.height = h
		if title ~= nil then
			createInfo.title = title
		end

		local windowHandle = prosper.create_window(createInfo)

		if windowHandle ~= nil then
			local elBase = gui.get_base_element(windowHandle)
			if util.is_valid(elBase) then
				local bg = gui.create("WIRect")
				bg:SetColor(Color.White)
				bg:SetSize(512, 512)

				local contents = gui.create("WIVBox", bg, 0, 0, bg:GetWidth(), bg:GetHeight(), 0, 0, 1, 1)
				contents:SetAutoFillContents(true)

				local p = gui.create("WIPFMControlsMenu", contents)
				p:SetAutoFillContentsToWidth(true)
				p:SetAutoFillContentsToHeight(false)

				if onOpen ~= nil then
					onOpen(windowHandle, contents, p)
				end
				p:Update()
				p:SizeToContents()

				bg:SetParentAndUpdateWindow(elBase)
				bg:SetAnchor(0, 0, 1, 1)
				bg:TrapFocus(true)
				bg:RequestFocus()
			end
		end
	end)
end
