include("wibasefileexplorer.lua")

local Element = util.register_class("gui.WIFileExplorer", gui.Base, gui.BaseFileExplorer)
Element.COLUMN_NAME = 1
Element.COLUMN_TYPE = 2
Element.COLUMN_FILE_SIZE = 3
Element.COLUMN_DATE_MODIFIED = 4
Element.COLUMN_COUNT = 5

local names = {
	[Element.COLUMN_NAME] = locale.get_text("name"),
	[Element.COLUMN_TYPE] = locale.get_text("type"),
	[Element.COLUMN_FILE_SIZE] = locale.get_text("file_size"),
	[Element.COLUMN_DATE_MODIFIED] = locale.get_text("date_modified"),
}

function gui.WIFileExplorer:__init()
	gui.Base.__init(self)
	gui.BaseFileExplorer.__init(self)
end

function gui.WIFileExplorer:OnUpdate()
	self:ListFiles()
end

function gui.WIFileExplorer:OnInitialize()
	gui.Base.OnInitialize(self)

	self.m_tRowInfo = {}
	local t = gui.create("WITable", self)
	t:SetAutoAlignToParent(true)
	t:SetScrollable(true)
	t:SetSortable(true)
	t:SetSelectableMode(gui.Table.SELECTABLE_MODE_MULTI)
	t:SetSortFunction(function(a, b, colIdx, ascending)
		local adata = self.m_tRowInfo[a]
		local bdata = self.m_tRowInfo[b]
		if adata ~= nil and bdata ~= nil then
			if adata.isDirectory and not bdata.isDirectory then
				return true
			end
			if not adata.isDirectory and bdata.isDirectory then
				return false
			end
		end
		-- ".." should always be first item
		if a:GetValue(0) == ".." then
			return true
		end
		if b:GetValue(0) == ".." then
			return false
		end
		local valA = a:GetValue(colIdx)
		local valB = b:GetValue(colIdx)
		local colType = self:GetColumnType(colIdx)
		if colType ~= Element.COLUMN_NAME and valA == valB then
			-- Fall-back to alphabetical sorting
			local idx = self:GetColumnIndex(Element.COLUMN_NAME)
			if idx ~= nil then
				valA = a:GetValue(idx)
				valB = b:GetValue(idx)
			end
		end
		local res = valA < valB
		if ascending then
			return res
		end
		return not res
	end)
	t:SetSelectableMode(gui.Table.SELECTABLE_MODE_SINGLE)
	t:SetRowHeight(20)
	self.m_pFileList = t

	self.m_columnStates = {}
	self.m_columIndexToType = {}
	for i = 1, Element.COLUMN_COUNT - 1 do
		self:SetColumnEnabled(i, false)
	end
	self:SetColumnEnabled(Element.COLUMN_NAME, true)
	self:SetColumnEnabled(Element.COLUMN_TYPE, true)
	self:SetColumnEnabled(Element.COLUMN_FILE_SIZE, true)
	self:UpdateColumns()

	self:SetPath("/")
end
function gui.WIFileExplorer:SetColumnEnabled(colType, enabled)
	self.m_columnStates[colType] = {
		enabled = enabled,
	}
end
function gui.WIFileExplorer:IsColumnEnabled(colType)
	return self.m_columnStates[colType].enabled or false
end
function gui.WIFileExplorer:GetColumnType(colIdx)
	return self.m_columIndexToType[colIdx]
end
function gui.WIFileExplorer:GetColumnIndex(colType)
	return self.m_columnStates[colType].columnIndex
end
function gui.WIFileExplorer:SetEnabledColumns(cols)
	local colMap = {}
	for _, col in ipairs(cols) do
		colMap[col] = true
	end
	for i = 1, Element.COLUMN_COUNT - 1 do
		self:SetColumnEnabled(i, colMap[i] or false)
	end
	self:UpdateColumns()
end
function gui.WIFileExplorer:GetEnabledColumns()
	local t = {}
	for i = 1, Element.COLUMN_COUNT - 1 do
		if self:IsColumnEnabled(i) then
			table.insert(t, i)
		end
	end
	return t
end
function gui.WIFileExplorer:UpdateColumns()
	local t = self.m_pFileList
	t:Clear(true)

	self.m_columIndexToType = {}
	for i = 1, Element.COLUMN_COUNT - 1 do
		self.m_columnStates[i].columnIndex = nil
	end
	local row = t:AddHeaderRow()
	for colIdx, colType in ipairs(self:GetEnabledColumns()) do
		row:SetValue(colIdx - 1, names[colType])
		self.m_columnStates[colType].columnIndex = colIdx - 1
		self.m_columIndexToType[colIdx - 1] = colType
	end
end
function gui.WIFileExplorer:GetSelectedFile(relativePath)
	relativePath = relativePath or false
	local path
	if relativePath == false then
		path = self:GetAbsolutePath()
	else
		path = self:GetPath()
		if path == "/" then
			path = ""
		end
	end
	local t = self.m_pFileList
	if util.is_valid(t) == true then
		local pRow = t:GetFirstSelectedRow()
		if pRow ~= nil then
			local pCell = pRow:GetCell(0)
			if pCell ~= nil then
				local pText = pCell:GetFirstChild("WIText")
				if pText ~= nil then
					path = path .. pText:GetText()
				end
			end
		end
	end
	if #path > 0 and path:sub(1, 1) == "/" then
		path = path:sub(2, #path)
	end
	return path
end
function gui.WIFileExplorer:GetIcon(row)
	return self.m_tRowInfo[row].icon
end
function gui.WIFileExplorer:ListFiles()
	local t = self.m_pFileList
	if util.is_valid(t) == false then
		return
	end
	t:Clear()
	self.m_tRowInfo = {}

	local tFiles, tDirectories = self:FindFiles()
	local function create_icon(icon)
		local margin = 2
		local pContainer = gui.create("WIBase")
		local pIcon = gui.create("WISilkIcon", pContainer)
		pIcon:SetIcon(icon)
		local size = pIcon:GetSize()
		size.x = size.x + margin
		pContainer:SetSize(size)
		return pContainer
	end

	local cols = self:GetEnabledColumns()
	local path = self:GetPath()
	local rootPath = self:GetRootPath()
	for _, dirName in ipairs(tDirectories) do
		local pIcon = create_icon("folder")

		local fPath = dirName
		if path ~= "/" then
			fPath = rootPath .. path .. fPath
		else
			fPath = rootPath
		end

		local row = t:AddRow()
		row:InsertElement(0, pIcon)
		for colIdx, colType in ipairs(cols) do
			if colType == Element.COLUMN_NAME then
				row:SetValue(colIdx - 1, dirName)
			elseif colType == Element.COLUMN_TYPE then
				row:SetValue(colIdx - 1, locale.get_text("file_folder"))
			elseif colType == Element.COLUMN_FILE_SIZE then
				row:SetValue(colIdx - 1, "")
			elseif colType == Element.COLUMN_DATE_MODIFIED then
				row:SetValue(colIdx - 1, file.get_last_write_time(fPath) or "")
			end
		end
		row:GetCell(0):SetTooltip(path .. fPath)
		row:AddCallback("OnDoubleClick", function(pRow)
			if util.is_valid(self) == false then
				return
			end
			self:SetPath(self:GetPath() .. dirName)
			self:Update()
		end)
		self.m_tRowInfo[row] = {
			isDirectory = true,
			icon = pIcon,
		}
		self:CallCallbacks("OnDirectoryRowAdded", row, fPath)
	end
	if self.m_isDirExplorer ~= true then
		for _, fName in ipairs(tFiles) do
			local pIcon = create_icon("page")

			local fPath = fName
			if path ~= "/" then
				fPath = rootPath .. path .. fPath
			else
				fPath = rootPath
			end
			local sz = file.get_size(fPath)
			local ext = file.get_file_extension(fPath)
			local fileType = (ext ~= nil) and ext:upper() or locale.get_text("unknown")

			local row = t:AddRow()
			row:InsertElement(0, pIcon)
			for colIdx, colType in ipairs(cols) do
				if colType == Element.COLUMN_NAME then
					row:SetValue(colIdx - 1, fName)
				elseif colType == Element.COLUMN_TYPE then
					row:SetValue(colIdx - 1, fileType)
				elseif colType == Element.COLUMN_FILE_SIZE then
					row:SetValue(colIdx - 1, util.get_pretty_bytes(sz))
				elseif colType == Element.COLUMN_DATE_MODIFIED then
					row:SetValue(colIdx - 1, file.get_last_write_time(fPath) or "")
				end
			end
			row:GetCell(0):SetTooltip(path .. fPath)
			row:AddCallback("OnMouseEvent", function(pRow, button, action, mods)
				if util.is_valid(self) == false then
					return
				end
				if button == input.MOUSE_BUTTON_LEFT and action == input.STATE_PRESS then
					if self:CallCallbacks("OnFileClicked", fName) == util.EVENT_REPLY_HANDLED then
						return
					end
				end
				self:CallCallbacks("OnFileMouseEvent", pRow, path .. fPath, button, action, mods)
			end)
			row:AddCallback("OnDoubleClick", function(pRow)
				if util.is_valid(self) == false then
					return
				end
				self:CallCallbacks("OnFileSelected", fPath)
			end)
			self.m_tRowInfo[row] = {
				isDirectory = false,
				icon = pIcon,
			}
			self:CallCallbacks("OnFileRowAdded", row, fPath)
		end
	end
	t:Sort()
	t:SetSize(self:GetSize())
end
gui.register("WIFileExplorer", gui.WIFileExplorer)
