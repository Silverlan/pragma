-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

util.register_class("util.PageControl")
function util.PageControl:__init(maxItemCount)
	self.m_maxItemCount = maxItemCount or 6
	self.m_itemCount = 0
	self:SetPage(0)
end
function util.PageControl:SetItemCount(n)
	self.m_itemCount = n
	self:UpdatePage()
end
function util.PageControl:SetMaxItemCount(n)
	self.m_maxItemCount = n
	self:UpdatePage()
end
function util.PageControl:UpdatePage(page)
	page = page or self:GetPage()
	local numPages = self:GetPageCount()
	if numPages == 0 then
		self.m_page = 0
		return
	end
	self.m_page = math.min(page, numPages - 1)
end
function util.PageControl:SetPage(page)
	self:UpdatePage(page)
end
function util.PageControl:GetItemCount()
	return self.m_itemCount
end
function util.PageControl:GetMaxItemCount()
	return self.m_maxItemCount
end
function util.PageControl:GetPage()
	return self.m_page
end
function util.PageControl:GetItemOffset()
	return self.m_page * self:GetMaxItemCount()
end
function util.PageControl:GetPageCount()
	return math.ceil((self:GetItemCount() - 1) / self:GetMaxItemCount())
end
function util.PageControl:IsItemVisible(id)
	local offset = self:GetItemOffset()
	return id >= offset and id < offset + self:GetMaxItemCount()
end
function util.PageControl:GoToNextPage()
	local numPages = self:GetPageCount()
	if numPages == 0 then
		self:SetPage(0)
		return
	end
	self:SetPage((self.m_page + 1) % self:GetPageCount())
end
function util.PageControl:GoToPreviousPage()
	local numPages = self:GetPageCount()
	if numPages == 0 then
		self:SetPage(0)
		return
	end
	local page = self.m_page - 1
	if page < 0 then
		page = self:GetPageCount() - 1
	end
	self:SetPage(page)
end
