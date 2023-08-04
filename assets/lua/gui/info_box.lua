--[[
    Copyright (C) 2021 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

local InfoBox = util.register_class("gui.InfoBox", gui.Base)
InfoBox.TYPE_INFO = 0
InfoBox.TYPE_WARNING = 1
InfoBox.TYPE_ERROR = 2
function InfoBox:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetSize(128, 128)

	local bg = gui.create("WIRect", self, 0, 0, self:GetWidth(), self:GetHeight(), 0, 0, 1, 1)
	self.m_bg = bg

	local bar = gui.create("WIRect", self, 0, 0, 3, self:GetHeight(), 0, 0, 0, 1)
	self.m_bar = bar

	self.m_btClose = gui.PFMButton.create(self, "gui/pfm/icon_clear", "gui/pfm/icon_clear_activated", function()
		self:CallCallbacks("OnClose")
		if self.m_removeOnClose then
			self:RemoveSafely()
			return
		end
		self:SetVisible(false)
	end)
	self.m_btClose:SetSize(11, 11)
	self.m_btClose:SetPos(self:GetWidth() - self.m_btClose:GetWidth() - 3, 3)
	self.m_btClose:SetAnchor(1, 0, 1, 0)

	local contents = gui.create("WIBase", self)
	contents:AnchorWithMargin(20)
	self.m_contents = contents

	local text = gui.create("WIText", contents)
	text:SetAutoBreakMode(gui.Text.AUTO_BREAK_WHITESPACE)
	text:SetTagsEnabled(true)
	self.m_text = text
	contents:AddCallback("SetSize", function()
		text:SetSize(contents:GetSize())
	end)

	self:SetType(InfoBox.TYPE_INFO)
end
function InfoBox:SetShowCloseButton(show)
	self.m_btClose:SetVisible(show)
end
function InfoBox:SetRemoveOnClose(removeOnClose)
	self.m_removeOnClose = removeOnClose
end
function InfoBox:SetType(type)
	self.m_type = type

	self.m_bg:RemoveStyleClass("bg_info")
	self.m_bg:RemoveStyleClass("bg_warning")
	self.m_bg:RemoveStyleClass("bg_error")
	self.m_text:RemoveStyleClass("fg_info")
	self.m_text:RemoveStyleClass("fg_warning")
	self.m_text:RemoveStyleClass("fg_error")
	self.m_bar:RemoveStyleClass("fg_info")
	self.m_bar:RemoveStyleClass("fg_warning")
	self.m_bar:RemoveStyleClass("fg_error")
	if type == InfoBox.TYPE_INFO then
		self.m_bg:AddStyleClass("bg_info")
		self.m_text:AddStyleClass("fg_info")
		self.m_bar:AddStyleClass("fg_info")
	elseif type == InfoBox.TYPE_WARNING then
		self.m_bg:AddStyleClass("bg_warning")
		self.m_text:AddStyleClass("fg_warning")
		self.m_bar:AddStyleClass("fg_warning")
	elseif type == InfoBox.TYPE_ERROR then
		self.m_bg:AddStyleClass("bg_error")
		self.m_text:AddStyleClass("fg_error")
		self.m_bar:AddStyleClass("fg_error")
	end
end
function InfoBox:GetType()
	return self.m_type
end
function InfoBox:SetText(text)
	self.m_text:SetText(text)
	self:SizeToContents()
end
function InfoBox:GetTextElement()
	return self.m_text
end
function InfoBox:SizeToContents()
	self.m_text:UpdateSubLines()
	self:SetHeight(self.m_contents:GetTop() * 2 + self.m_text:GetTextHeight())
end
gui.register("InfoBox", InfoBox)

gui.create_info_box = function(parent, text, type)
	local infoBox = gui.create("InfoBox", parent)
	infoBox:SetType(type or gui.InfoBox.TYPE_INFO)
	infoBox:SetText(text)
	infoBox:SizeToContents()
	parent:AddCallback("SetSize", function()
		if infoBox:IsValid() then
			infoBox:SizeToContents()
		end
	end)
	return infoBox
end

gui.register_default_skin(
	[[{}]],
	[[{
	["infobox"] = {
		children = {
			["bg_info"] = {
				Initialize = function(GUI,pElement)
					pElement:SetColor(Color(0,21,32))
				end
			},
			["bg_warning"] = {
				Initialize = function(GUI,pElement)
					pElement:SetColor(Color(26,32,0))
				end
			},
			["bg_error"] = {
				Initialize = function(GUI,pElement)
					pElement:SetColor(Color(26,10,0))
				end
			},
			["fg_info"] = {
				Initialize = function(GUI,pElement)
					pElement:SetColor(Color(2,136,209))
				end
			},
			["fg_warning"] = {
				Initialize = function(GUI,pElement)
					pElement:SetColor(Color(207,212,0))
				end
			},
			["fg_error"] = {
				Initialize = function(GUI,pElement)
					pElement:SetColor(Color(207,77,3))
				end
			}
		}
	}
}]]
)
