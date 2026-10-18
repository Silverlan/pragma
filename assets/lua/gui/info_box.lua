-- SPDX-FileCopyrightText: (c) 2022 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local InfoBox = util.register_class("gui.InfoBox", gui.Base)
InfoBox.TYPE_INFO = 0
InfoBox.TYPE_WARNING = 1
InfoBox.TYPE_ERROR = 2
function InfoBox:OnInitialize()
	gui.Base.OnInitialize(self)

	self:SetSize(128, 128)
	self:SetAutoSizeToContents(true)

	self.m_btClose = gui.PFMButton.create(self, "gui/pfm/icon_clear", "gui/pfm/icon_clear_activated", function()
		self:CallCallbacks("OnClose")
		if self.m_removeOnClose then
			self:RemoveSafely()
			return
		end
		self:SetVisible(false)
	end)

	local contents = gui.create("vbox", self)
	contents:AddStyleClass("info-box")
	self.m_contents = contents

	local text = gui.create("WIText", contents)
	text:SetAutoBreakMode(gui.Text.AUTO_BREAK_WHITESPACE)
	text:SetTagsEnabled(true)
	self.m_text = text
	contents:AddCallback("OnSizeChanged", function()
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

	self.m_contents:RemoveStyleClass("info-box--info")
	self.m_contents:RemoveStyleClass("info-box--warning")
	self.m_contents:RemoveStyleClass("info-box--error")

	if type == InfoBox.TYPE_INFO then
		self.m_contents:AddStyleClass("info-box--info")
	elseif type == InfoBox.TYPE_WARNING then
		self.m_contents:AddStyleClass("info-box--warning")
	elseif type == InfoBox.TYPE_ERROR then
		self.m_contents:AddStyleClass("info-box--error")
	end
	self.m_contents:RefreshSkin()
end
function InfoBox:GetType()
	return self.m_type
end
function InfoBox:GetText()
	return self.m_text:GetText()
end
function InfoBox:SetText(text)
	self.m_text:SetText(text)
end
function InfoBox:GetTextElement()
	return self.m_text
end
function InfoBox:SizeToContents()
	self.m_text:UpdateSubLines()
end
gui.register("InfoBox", InfoBox)

gui.create_info_box = function(parent, text, type)
	local infoBox = gui.create("InfoBox", parent)
	infoBox:SetType(type or gui.InfoBox.TYPE_INFO)
	infoBox:SetText(text)
	infoBox:SizeToContents()
	parent:AddCallback("OnSizeChanged", function()
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
