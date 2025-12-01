-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

if gui.skin_exists("default") == true then
	return
end

-------------------------------------------
------------ START OF SETTINGS ------------
-------------------------------------------

local t = {}
t.BACKGROUND_COLOR_DEFAULT = Color(38, 38, 38, 255)
t.BACKGROUND_COLOR_HOVER = Color(48, 48, 48, 255)
t.BACKGROUND_COLOR_SELECTED = Color(58, 58, 58, 255)

t.BACKGROUND_GRADIENT_START = Color(50, 50, 50, 255)
t.BACKGROUND_GRADIENT_END = t.BACKGROUND_COLOR_DEFAULT

-- WIButton
t.BUTTON_BACKGROUND_GRADIENT_HOVER_START = Color(80, 80, 80, 255)
t.BUTTON_BACKGROUND_GRADIENT_HOVER_END = t.BACKGROUND_COLOR_DEFAULT

t.BUTTON_BACKGROUND_GRADIENT_SELECTED_START = Color(30, 30, 30, 255)
t.BUTTON_BACKGROUND_GRADIENT_SELECTED_END = t.BACKGROUND_COLOR_DEFAULT

-- WITable
t.TABLE_TEXT_OFFSET_X = 12
t.TABLE_BACKGROUND_GRADIENT_START = Color(30, 30, 30, 255)
t.TABLE_BACKGROUND_GRADIENT_END = Color(10, 10, 10, 255)
t.TABLE_ROW_COLOR_HOVER = t.BACKGROUND_COLOR_HOVER
t.TABLE_ROW_COLOR_SELECTED = t.BACKGROUND_COLOR_SELECTED

-- WIMainMenu
t.MENU_ITEM_COLOR = Color(120, 120, 120, 255)
t.MENU_ITEM_SELECTED_COLOR = Color.White

t.MENU_ITEM_SHADOW_COLOR = Color(0, 0, 0, 1.0)
t.MENU_ITEM_SHADOW_OFFSET = Vector2(2, 2)
t.MENU_ITEM_SELECTED_SHADOW_COLOR = Color(255, 176, 39, 191)
t.MENU_ITEM_SELECTED_SHADOW_OFFSET = Vector2(0, 0)
t.MENU_ITEM_SHADOW_BLUR = 0.4
t.MENU_ITEM_SHADOW_TRANSITION_TIME = 0.3
t.MENU_ITEM_SELECT_SOUND_FILE = "ui/menu_select"
t.MENU_ITEM_FONT_SET = engine.get_default_font_set_name()

-- WIProgressMenu
t.PROGRESS_BAR_LABEL_OVERLAY_COLOR = Color(0, 0, 0, 255)
t.PROGRESS_BAR_LABEL_BACKGROUND_COLOR = Color.White

-------------------------------------------
------------- END OF SETTINGS -------------
-------------------------------------------

local function add_skin_element(pElement, el)
	if pElement.m_tSkinElements == nil then
		pElement.m_tSkinElements = {}
	end
	table.insert(pElement.m_tSkinElements, el)
end

local function clear_element(GUI, pElement)
	if pElement.m_tSkinElements ~= nil then
		for _, el in ipairs(pElement.m_tSkinElements) do
			if el:IsValid() then
				el:Remove()
			end
		end
		pElement.m_tSkinElements = nil
	end
end

local function create_gradient(colStart, colEnd)
	return prosper.create_gradient_texture(128, 64, prosper.FORMAT_R8G8B8A8_UNORM, Vector2(0, -1), {
		{ offset = 0.0, color = colStart },
		{ offset = 1.0, color = colEnd },
	})
end

t.BACKGROUND_GRADIENT = function(t)
	t._BACKGROUND_GRADIENT = t._BACKGROUND_GRADIENT
		or create_gradient(t.BACKGROUND_GRADIENT_START, t.BACKGROUND_GRADIENT_END)
	return t._BACKGROUND_GRADIENT
end
t.TABLE_BACKGROUND_GRADIENT = function(t)
	t._TABLE_BACKGROUND_GRADIENT = t._TABLE_BACKGROUND_GRADIENT
		or create_gradient(t.TABLE_BACKGROUND_GRADIENT_START, t.TABLE_BACKGROUND_GRADIENT_END)
	return t._TABLE_BACKGROUND_GRADIENT
end
t.BUTTON_BACKGROUND_GRADIENT_HOVER = function(t)
	t._BUTTON_BACKGROUND_GRADIENT_HOVER = t._BUTTON_BACKGROUND_GRADIENT_HOVER
		or create_gradient(t.BUTTON_BACKGROUND_GRADIENT_HOVER_START, t.BUTTON_BACKGROUND_GRADIENT_HOVER_END)
	return t._BUTTON_BACKGROUND_GRADIENT_HOVER
end
t.BUTTON_BACKGROUND_GRADIENT_SELECTED = function(t)
	t._BUTTON_BACKGROUND_GRADIENT_SELECTED = t._BUTTON_BACKGROUND_GRADIENT_SELECTED
		or create_gradient(t.BUTTON_BACKGROUND_GRADIENT_SELECTED_START, t.BUTTON_BACKGROUND_GRADIENT_SELECTED_END)
	return t._BUTTON_BACKGROUND_GRADIENT_SELECTED
end
t.MENU_ITEM_SELECT_SOUND = function(t)
	if t._MENU_ITEM_SELECT_SOUND == nil then
		sound.precache(t.MENU_ITEM_SELECT_SOUND_FILE)
		t._MENU_ITEM_SELECT_SOUND = true
	end
	return t.MENU_ITEM_SELECT_SOUND_FILE
end
t.MENU_ITEM_FONT = function(t)
	if t._MENU_ITEM_FONT == nil then
		engine.create_font(
			"MainMenu_Regular",
			t.MENU_ITEM_FONT_SET,
			bit.bor(engine.FONT_FEATURE_FLAG_SANS_BIT, engine.FONT_FEATURE_FLAG_MONO_BIT),
			36
		)
		t._MENU_ITEM_FONT = true
	end
	return "MainMenu_Regular"
end
t.HEADER_FONT = function(t)
	if t._HEADER_FONT == nil then
		engine.create_font(
			"header",
			t.MENU_ITEM_FONT_SET,
			bit.bor(engine.FONT_FEATURE_FLAG_SANS_BIT, engine.FONT_FEATURE_FLAG_MONO_BIT),
			48
		)
		t._HEADER_FONT = true
	end
	return "header"
end
t.HEADER2_FONT = function(t)
	if t._HEADER2_FONT == nil then
		engine.create_font(
			"header2",
			t.MENU_ITEM_FONT_SET,
			bit.bor(engine.FONT_FEATURE_FLAG_SANS_BIT, engine.FONT_FEATURE_FLAG_MONO_BIT),
			30
		)
		t._HEADER2_FONT = true
	end
	return "header2"
end
t.CREDITS_FONT = function(t)
	if t._CREDITS_FONT == nil then
		engine.create_font(
			"credits",
			t.MENU_ITEM_FONT_SET,
			bit.bor(engine.FONT_FEATURE_FLAG_SANS_BIT, engine.FONT_FEATURE_FLAG_BOLD_BIT),
			18
		)
		t._CREDITS_FONT = true
	end
	return "credits"
end

local skin = {}
------------ WIButton ------------
skin["wibutton"] = {
	Initialize = function(GUI, pElement)
		local gradient = GUI:BACKGROUND_GRADIENT()
		if gradient ~= nil then
			local bg = gui.create("WITexturedRect", pElement)
			bg:SetTexture(gradient)
			bg:SetName("background")
			bg:SetAutoAlignToParent(true)
			bg:SetZPos(-2)
			pElement.m_pBackground = bg
			add_skin_element(pElement, bg)
		end

		local fcCursorEntered = function()
			pElement.m_bMouseOver = true
			if pElement.m_bPressed == true then
				return
			end
			if pElement.m_pBackground == nil or not pElement.m_pBackground:IsValid() then
				return
			end
			local gradient = GUI:BUTTON_BACKGROUND_GRADIENT_HOVER()
			if gradient == nil then
				return
			end
			pElement.m_pBackground:SetTexture(gradient)
		end
		local cbCursorEntered = pElement:AddCallback("OnCursorEntered", fcCursorEntered)
		add_skin_element(pElement, cbCursorEntered)
		local cbCursorExited = pElement:AddCallback("OnCursorExited", function()
			pElement.m_bMouseOver = false
			if pElement.m_bPressed == true then
				return
			end
			if pElement.m_pBackground == nil or not pElement.m_pBackground:IsValid() then
				return
			end
			local gradient = GUI:BACKGROUND_GRADIENT()
			if gradient == nil then
				return
			end
			pElement.m_pBackground:SetTexture(gradient)
		end)
		add_skin_element(pElement, cbCursorExited)
		local cbMousePressed = pElement:AddCallback("OnMousePressed", function()
			pElement.m_bPressed = true
			if pElement.m_pBackground == nil or not pElement.m_pBackground:IsValid() then
				return
			end
			local gradient = GUI:BUTTON_BACKGROUND_GRADIENT_SELECTED()
			if gradient == nil then
				return
			end
			pElement.m_pBackground:SetTexture(gradient)
		end)
		add_skin_element(pElement, cbMousePressed)
		local cbMouseReleased = pElement:AddCallback("OnMouseReleased", function()
			pElement.m_bPressed = false
			if pElement.m_bMouseOver == true then
				fcCursorEntered()
				return
			end
			if pElement.m_pBackground == nil or not pElement.m_pBackground:IsValid() then
				return
			end
			local gradient = GUI:BACKGROUND_GRADIENT()
			if gradient == nil then
				return
			end
			pElement.m_pBackground:SetTexture(gradient)
		end)
		add_skin_element(pElement, cbMouseReleased)

		local fcSetSize = function()
			local pText = pElement:GetFirstChild("witext")
			if pText ~= nil and pText:IsValid() then
				pText:SetPos(
					pElement:GetWidth() * 0.5 - pText:GetWidth() * 0.5,
					pElement:GetHeight() * 0.5 - pText:GetHeight() * 0.5
				)
			end
		end
		local cbSetSize = pElement:AddCallback("SetSize", fcSetSize)
		fcSetSize()
		add_skin_element(pElement, cbSetSize)
	end,
	Release = clear_element,
	children = {
		["witext"] = {
			Initialize = function(GUI, pElement)
				local cbSetSize = pElement:AddCallback("SetSize", function()
					local p = pElement:GetParent()
					if not p:IsValid() then
						return
					end
					pElement:SetPos(
						p:GetWidth() * 0.5 - pElement:GetWidth() * 0.5,
						p:GetHeight() * 0.5 - pElement:GetHeight() * 0.5
					)
				end)
				add_skin_element(pElement, cbSetSize)

				pElement:SetColorRGB(Color(255, 255, 255, 255))
				pElement:SetText(pElement:GetText():upper())
				pElement:SizeToContents()
			end,
			Release = clear_element,
		},
	},
}
-----------------------------------------

------------ WIFrame ------------
skin["wiframe"] = {
	Initialize = function(GUI, pElement)
		pElement:SetColorRGB(GUI.BACKGROUND_COLOR_DEFAULT)
	end,
	children = {
		["move_rect"] = {
			Initialize = function(GUI, pElement)
				pElement:SetHeight(20)
			end,
		},
		["frame_titlebar"] = {
			Initialize = function(GUI, pElement)
				pElement:SetColorRGB(GUI.BACKGROUND_COLOR_DEFAULT)
				local pFrame = pElement:GetParent()
				if pFrame:IsValid() then
					pFrame:ScheduleUpdate()
				end
			end,
		},
		["frame_title"] = {
			Initialize = function(GUI, pElement)
				pElement:SetColorRGB(Color(255, 255, 255, 255))
			end,
		},
	},
}
-----------------------------------------

------------ WIServerBrowser ------------
skin["serverbrowser"] = {
	children = {
		["witable"] = {
			Initialize = function(GUI, pElement)
				pElement:SetRowHeight(24)
			end,
		},
		["wisilkicon"] = {
			Initialize = function(GUI, pElement)
				local pCell = pElement:GetParent()
				if pCell:IsValid() and pCell:GetClass() == "witablecell" then
					pElement:SetY(pCell:GetHeight() * 0.5 - pElement:GetHeight() * 0.5)
					pElement:SetX(GUI.TABLE_TEXT_OFFSET_X)
				end
			end,
		},
	},
}
-----------------------------------------

---------------- WITable ----------------
skin["witable"] = {
	Initialize = function(GUI, pElement)
		local bg = gui.create("WITexturedRect", pElement)
		bg:SetZPos(-2)
		bg:SetTexture(GUI:TABLE_BACKGROUND_GRADIENT())
		bg:SetName("background")
		bg:SetBackgroundElement()
		pElement.m_pBgGradient = bg
		add_skin_element(pElement, bg)
	end,
	Release = clear_element,
}
skin["table_row_header"] = {
	Initialize = function(GUI, pElement)
		local gradient = GUI:BACKGROUND_GRADIENT()
		if gradient ~= nil then
			local bg = gui.create("WITexturedRect", pElement)
			bg:SetZPos(0)
			bg:SetTexture(gradient)
			bg:SetName("background")
			bg:SetAutoAlignToParent(true)
			add_skin_element(pElement, bg)
		end
	end,
	Release = clear_element,
	children = {
		["witablecell"] = {
			Initialize = function(GUI, pElement)
				pElement:SetZPos(1)
				local fcSetSize = function()
					local sz = pElement:GetSize()
					local c = pElement:GetFirstChild("witext")
					if c ~= nil and c:IsValid() then
						c:SetY(sz.y * 0.5 - c:GetHeight() * 0.5)
						c:SetX(sz.x * 0.5 - c:GetWidth() * 0.5)
					end
				end
				local cbSetSize = pElement:AddCallback("SetSize", fcSetSize)
				add_skin_element(pElement, cbSetSize)
				fcSetSize()
			end,
			Release = clear_element,
		},
		["witext"] = {
			Initialize = function(GUI, pElement)
				pElement:SetColorRGB(Color(255, 255, 255, 255))
			end,
		},
	},
}
skin["table_row"] = {
	Initialize = function(GUI, pElement)
		local bg = gui.create("WIRect", pElement)
		bg:SetZPos(0)
		bg:SetName("background")
		bg:SetAutoAlignToParent(true)
		bg:SetVisible(false)
		add_skin_element(pElement, bg)

		local cbCursorEntered = pElement:AddCallback("OnCursorEntered", function()
			if pElement:IsSelected() then
				return
			end
			local bg = pElement:FindChildByName("background")
			if bg ~= nil and bg:IsValid() then
				bg:SetVisible(true)
				bg:SetColorRGB(GUI.TABLE_ROW_COLOR_HOVER)
			end
		end)
		local cbCursorExited = pElement:AddCallback("OnCursorExited", function()
			if pElement:IsSelected() then
				return
			end
			local bg = pElement:FindChildByName("background")
			if bg ~= nil and bg:IsValid() then
				bg:SetVisible(false)
			end
		end)
		local cbSelected = pElement:AddCallback("OnSelected", function()
			local bg = pElement:FindChildByName("background")
			if bg ~= nil and bg:IsValid() then
				bg:SetVisible(true)
				bg:SetColorRGB(GUI.TABLE_ROW_COLOR_SELECTED)
			end
		end)
		local cbDeselected = pElement:AddCallback("OnDeselected", function()
			local bg = pElement:FindChildByName("background")
			if bg ~= nil and bg:IsValid() then
				bg:SetVisible(false)
			end
		end)
		add_skin_element(pElement, cbCursorEntered)
		add_skin_element(pElement, cbCursorExited)
		add_skin_element(pElement, cbSelected)
		add_skin_element(pElement, cbDeselected)
	end,
	Release = clear_element,
	children = {
		["witablecell"] = {
			Initialize = function(GUI, pElement)
				pElement:SetZPos(1)
			end,
		},
		["witext"] = {
			Initialize = function(GUI, pElement)
				local pCell = pElement:GetParent()
				if
					not pCell:IsValid()
					or (pCell:GetClass() ~= "widropdownmenu" and pCell:GetClass() ~= "witextentrybase")
				then
					pElement:SetColorRGB(Color(255, 255, 255, 255))
				end
			end,
			Release = clear_element,
		},
	},
}
skin["table_row_offset"] = {
	children = {
		["witext"] = {
			Initialize = function(GUI, pElement)
				local pCell = pElement:GetParent()
				if pCell:IsValid() then
					local childIdx = pCell:FindChildIndex(pElement)
					if childIdx == 0 and pCell:GetClass() == "witablecell" then
						local fcSetSize = function()
							if pElement:IsValid() == false then
								return
							end
							local sz = pCell:GetSize()
							pElement:SetY(pCell:GetHeight() * 0.5 - pElement:GetHeight() * 0.5)
							pElement:SetX(GUI.TABLE_TEXT_OFFSET_X)
						end
						local cbSetSize = pCell:AddCallback("SetSize", fcSetSize)
						add_skin_element(pElement, cbSetSize)
						fcSetSize()
					end
				end
			end,
			Release = clear_element,
		},
	},
}
-----------------------------------------

--------------- MAIN MENU ---------------
skin["main_menu"] = {
	children = {
		["witable"] = {
			Initialize = function(GUI, pElement)
				if pElement.m_pBgGradient ~= nil and pElement.m_pBgGradient:IsValid() then
					pElement.m_pBgGradient:SetAlpha(220)
				end
			end,
			children = {
				["witablerow"] = {
					Initialize = function(GUI, pElement)
						local outline = gui.create("WIOutlinedRect", pElement)
						outline:SetColor(Color(188, 188, 188, 128))
						outline:SetAutoAlignToParent(true)
					end,
				},
			},
		},
	},
}
skin["wimainmenuelement"] = {
	Initialize = function(GUI, pElement)
		local pText = pElement:GetFirstChild("witext")
		if pText ~= nil and pText:IsValid() then
			pText:SetShadowColor(GUI.MENU_ITEM_SHADOW_COLOR)
			pText:SetShadowOffset(GUI.MENU_ITEM_SHADOW_OFFSET)

			pText:EnableShadow(true)
			pText:SetFont(GUI:MENU_ITEM_FONT())
			pText:SetText(pText:GetText())
			pText:SetColorRGB(GUI.MENU_ITEM_COLOR)
			pText:SizeToContents()

			pElement:AddCallback("Select", function()
				local index = pElement:GetParent():FindChildIndex(pElement)
				local playInfo = sound.PlayInfo()
				playInfo.pitch = 0.8
				sound.play(GUI:MENU_ITEM_SELECT_SOUND(), sound.TYPE_GUI, playInfo)
				if pText:IsValid() then
					pText:SetColorRGB(GUI.MENU_ITEM_SELECTED_COLOR)
				end
			end)
			pElement:AddCallback("Deselect", function()
				if pText:IsValid() then
					pText:SetColorRGB(GUI.MENU_ITEM_COLOR)
				end
			end)
		end
	end,
}
-----------------------------------------
------------- WIProgressBar -------------
skin["wiprogressbar"] = {
	children = {
		["progressbar_label_overlay"] = {
			Initialize = function(GUI, pElement)
				pElement:SetColorRGB(GUI.PROGRESS_BAR_LABEL_OVERLAY_COLOR)
			end,
		},
		["progressbar_label_background"] = {
			Initialize = function(GUI, pElement)
				pElement:SetColorRGB(GUI.PROGRESS_BAR_LABEL_BACKGROUND_COLOR)
			end,
		},
	},
	Initialize = function(GUI, pElement)
		local pOutline = gui.create("WIOutlinedRect", pElement)
		pOutline:SetAutoAlignToParent(true)
		pOutline:SetColorRGB(Color.Black)
	end,
}
skin["wislider"] = skin["wiprogressbar"]
-----------------------------------------
------------- WITooltip -------------
skin["witooltip"] = {
	Initialize = function(GUI, pElement)
		local pText = pElement:GetFirstChild("witext")
		if pText == nil then
			return
		end
		pText:SetColorRGB(Color.LightGrey)
		local gradient = GUI:BACKGROUND_GRADIENT()
		if gradient ~= nil then
			local pRect = gui.create("WITexturedRect", pElement)
			pRect:SetTexture(gradient)
			pRect:SetAutoAlignToParent(true)
			pRect:SetZPos(0)

			local pRectOutline = gui.create("WIOutlinedRect", pElement)
			pRectOutline:SetColor(Color(188, 188, 188, 128))
			pRectOutline:SetAutoAlignToParent(true)
			pRectOutline:SetZPos(0)

			pText:SetZPos(1)
		end
		local cbSize = pText:AddCallback("SetSize", function(pText)
			local sz = pText:GetSize()
			pText:SetPos(10, 5)

			pElement:SetSize(sz.x + 20, sz.y + 10)

			local szWindow = gui.get_window_size()
			if pElement:GetRight() >= szWindow.x then
				pElement:SetX(szWindow.x - pElement:GetWidth())
			end
			if pElement:GetBottom() >= szWindow.y then
				pElement:SetY(szWindow.y - pElement:GetHeight())
			end
		end)
		add_skin_element(pElement, cbSize)
	end,
	Release = clear_element,
}
-----------------------------------------
------------- WIOptionsList -------------
skin["wioptionslist"] = {
	Initialize = function(GUI, pElement)
		--pElement:SetWidth(600)
		pElement:SizeToContents()
	end,
}
skin["wislider"] = skin["wiprogressbar"]
-----------------------------------------
------------- Window Frame --------------
skin["window_frame"] = {
	Initialize = function(GUI, pElement)
		local bg = gui.create("WIRect", pElement)
		bg:SetZPos(-2)
		bg:SetColorRGB(GUI.BACKGROUND_COLOR_DEFAULT)
		bg:SetName("background")
		bg:SetAutoAlignToParent(true)
		add_skin_element(pElement, bg)
	end,
	Release = clear_element,
	children = {
		["witext"] = {
			Initialize = function(GUI, pElement)
				local pParent = pElement:GetParent()
				if
					not pParent:IsValid()
					or (pParent:GetClass() ~= "widropdownmenu" and pParent:GetClass() ~= "witextentrybase")
				then
					pElement:SetColorRGB(Color.White)
				end
			end,
		},
	},
}
-----------------------------------------
skin["credits_text"] = {
	Initialize = function(GUI, pElement)
		pElement:SetFont(GUI:CREDITS_FONT())
		pElement:SizeToContents()
		pElement:SetX(pElement:GetParent():GetWidth() / 2 - pElement:GetWidth() / 2)
		pElement:SetColor(Color(200, 200, 200))
	end,
}
skin["credits_logo"] = {
	Initialize = function(GUI, pElement)
		pElement:SetX(pElement:GetParent():GetWidth() / 2 - pElement:GetWidth() / 2)
	end,
}
-----------------------------------------
skin["header"] = {
	Initialize = function(GUI, pElement)
		pElement:SetFont(GUI:HEADER_FONT())
		pElement:SizeToContents()
		pElement:SetX(pElement:GetParent():GetWidth() / 2 - pElement:GetWidth() / 2)
	end,
}
skin["header2"] = {
	Initialize = function(GUI, pElement)
		pElement:SetFont(GUI:HEADER2_FONT())
		pElement:SizeToContents()
		pElement:SetX(pElement:GetParent():GetWidth() / 2 - pElement:GetWidth() / 2)
	end,
}
-----------------------------------------
skin["bg_gradient"] = {
	Initialize = function(GUI, pElement)
		pElement:SetTexture(GUI:TABLE_BACKGROUND_GRADIENT())
	end,
	Release = clear_element,
}
-----------------------------------------
gui.register_skin("default", t, skin)
gui.set_skin("default")
