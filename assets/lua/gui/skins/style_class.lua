-- SPDX-FileCopyrightText: (c) 2026 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

log = log or {}
log.info = function(...)
	print("[GUI] ", ...)
end
log.warn = function(...)
	console.print_warning("[GUI] ", ...)
end
log.err = function(...)
	console.print_error("[GUI] ", ...)
end

local function unpack_spacing(val)
	if type(val) == "number" then
		return val, val, val, val
	elseif type(val) == "table" then
		return val.left or val[1] or 0, val.top or val[2] or 0, val.right or val[3] or 0, val.bottom or val[4] or 0
	end
	return 0, 0, 0, 0
end

local function parse_dimension(val)
	if type(val) == "string" then
		if val == "auto" then return nil, false, true end
		if #val > 0 and val:sub(-1) == "%" then
			return tonumber(val:sub(1, -2)) / 100.0, true, false
		end
	end
	if val ~= nil then
		return tonumber(val), false, false
	end
	return nil, false, false
end

local gradient_cache = {}
local function get_cached_gradient(grad_style)
	local w = grad_style.width or 128
	local h = grad_style.height or 64
	local dirX = grad_style.dir and grad_style.dir[1] or 0.0
	local dirY = grad_style.dir and grad_style.dir[2] or -1.0
	
	-- Build cache key
	local key = string.format("%d_%d_%f_%f", w, h, dirX, dirY)
	local nodes = {}
	
	if grad_style.nodes then
		for _, node in ipairs(grad_style.nodes) do
			key = key .. "_" .. tostring(node.offset) .. "_" .. tostring(node.color)
			
			table.insert(nodes, {
				offset = node.offset,
				color = Color.CreateFromHexColor(node.color:sub(2))
			})
		end
	else
		table.insert(nodes, { offset = 0.0, color = Color.CreateFromHexColor("ffffff") })
		table.insert(nodes, { offset = 1.0, color = Color.CreateFromHexColor("000000") })
		key = key .. "_default"
	end
	
	if gradient_cache[key] then
		return gradient_cache[key]
	end
	
	local tex = prosper.create_gradient_texture(
		w, h, 
		prosper.FORMAT_R8G8B8A8_UNORM, 
		Vector2(dirX, dirY), 
		nodes
	)
	gradient_cache[key] = tex
	return tex
end

local function find_matching_children(parent, selector)
	local results = {}
	if not parent then return results end

	local isDirectOnly = false
	local matchName = selector

	-- ">" can be used to target direct child only
	if matchName:sub(1, 1) == ">" then
		isDirectOnly = true
		-- Strip '>' and trim
		matchName = matchName:sub(2):match("^%s*(.-)%s*$") 
	end

	local function is_match(child)
		if child:HasStyleClass(matchName) then return true end

		local childName = child:GetName()
		local childClass = child:GetClass()

		return (childName == matchName)
			or (childClass:lower() == matchName:lower())
	end

	local function collect_children(currentParent)
		local children = currentParent:GetChildren()
		for _, child in ipairs(children) do
			if is_match(child) then
				table.insert(results, child)
			end

			if not isDirectOnly then
				collect_children(child)
			end
		end
	end

	collect_children(parent)
	return results
end

local function apply_dimensions(element, style)
	local x_val, x_is_pct = parse_dimension(style.x)
	local y_val, y_is_pct = parse_dimension(style.y)
	local w_val, w_is_pct, w_is_auto = parse_dimension(style.width)
	local h_val, h_is_pct, h_is_auto = parse_dimension(style.height)
	local origSize
	if w_is_auto or h_is_auto then origSize = element:GetSize() end

	-- x / width
	if x_is_pct then
		element:ApplyX(0)
		element:SetAnchor(gui.ANCHOR_EDGE_LEFT, x_val)
	elseif x_val then
		element:ApplyX(toint(x_val))
	end

	if w_is_pct then
		-- To make % width work dynamically, we need a left anchor to base the right anchor on
		local base_left = 0.0
		if x_is_pct then 
			base_left = x_val
		elseif style.anchor then 
			base_left = style.anchor[1] 
		end
		local parent = element:GetParent()
		if parent then
			element:ApplyWidth(toint(parent:GetWidth() *w_val))
		end
		
		if not x_is_pct and not style.anchor then
			element:SetAnchor(gui.ANCHOR_EDGE_LEFT, base_left)
		end
		-- Set Right anchor dynamically
		element:SetAnchor(gui.ANCHOR_EDGE_RIGHT, base_left + w_val)
	elseif w_val then
		element:ApplyWidth(toint(w_val))
	end

	-- y / height
	if y_is_pct then
		element:ApplyY(0)
		element:SetAnchor(gui.ANCHOR_EDGE_TOP, y_val)
	elseif y_val then
		element:ApplyY(toint(y_val))
	end

	if h_is_pct then
		-- To make % height work dynamically, we need a top anchor to base the bottom anchor on
		local base_top = 0.0
		if y_is_pct then 
			base_top = y_val
		elseif style.anchor then 
			base_top = style.anchor[2] 
		end
		local parent = element:GetParent()
		if parent then
			element:ApplyHeight(toint(element:GetParent():GetHeight() *h_val))
		end
		
		if not y_is_pct and not style.anchor then
			element:SetAnchor(gui.ANCHOR_EDGE_TOP, base_top)
		end
		-- Set Bottom anchor dynamically
		element:SetAnchor(gui.ANCHOR_EDGE_BOTTOM, base_top + h_val)
	elseif h_val then
		element:ApplyHeight(toint(h_val))
	end

	if w_is_auto ~= h_is_auto then
		local aspectRatio = (origSize.y > 0) and (origSize.x /origSize.y) or 0.0
		if w_is_auto then
			local w = element:GetHeight() *aspectRatio
			element:ApplyWidth(w)
		else
			local h = (aspectRatio > 0.0001) and (element:GetWidth() /aspectRatio) or 0.0
			element:ApplyHeight(h)
		end
	end
end

local function to_alignment_enum(align)
	local alignment = gui.ALIGNMENT_NONE
	if align == "start" then alignment = gui.ALIGNMENT_START
	elseif align == "center" then alignment = gui.ALIGNMENT_CENTER
	elseif align == "end" then alignment = gui.ALIGNMENT_END
	elseif align == "fill" then alignment = gui.ALIGNMENT_FILL end
	return alignment
end

local function to_flex_align_enum(align)
	local flexAlign = gui.FlexBox.FLEX_ALIGN_START
	if align == "start" then flexAlign = gui.FlexBox.FLEX_ALIGN_START
	elseif align == "center" then flexAlign = gui.FlexBox.FLEX_ALIGN_CENTER
	elseif align == "end" then flexAlign = gui.FlexBox.FLEX_ALIGN_END
	elseif align == "stretch" then flexAlign = gui.FlexBox.FLEX_ALIGN_STRETCH end
	return flexAlign
end

local function to_flex_justify_enum(align)
	local flexJustify = gui.FlexBox.FLEX_JUSTIFY_START
	if align == "start" then flexJustify = gui.FlexBox.FLEX_JUSTIFY_START
	elseif align == "center" then flexJustify = gui.FlexBox.FLEX_JUSTIFY_CENTER
	elseif align == "end" then flexJustify = gui.FlexBox.FLEX_JUSTIFY_END
	elseif align == "spaceBetween" then flexJustify = gui.FlexBox.FLEX_JUSTIFY_SPACE_BETWEEN
	elseif align == "spaceEvenly" then flexJustify = gui.FlexBox.FLEX_JUSTIFY_SPACE_EVENLY end
	return flexJustify
end

local function apply_anchor_align(element, align, idStart, idEnd, anchor0, anchor1, margin0, margin1, offset, axis)
	if align == idStart then
		margin0 = margin0 or 0
		element:SetAnchor(anchor0, 0.0)
		element:SetAnchor(anchor1, 0.0)
		element:SetAnchorEdgeEnabled(anchor0, true)
		element:SetAnchorOffset(anchor0, margin0)
	elseif align == idEnd then
		margin1 = margin1 or 0
		element:SetAnchor(anchor0, 1.0)
		element:SetAnchor(anchor1, 1.0)
		element:SetAnchorEdgeEnabled(anchor1, true)
		element:SetAnchorOffset(anchor1, -margin1)
	elseif align == "center" then
		if axis == math.AXIS_X then element:SetAnchorHorizontalCenter(offset or 0)
		else element:SetAnchorVerticalCenter(offset or 0) end
	elseif align == "stretch" then
		margin0 = margin0 or 0
		margin1 = margin1 or 0
		element:SetAnchor(anchor0, 0.0)
		element:SetAnchor(anchor1, 1.0)
		element:SetAnchorEdgeEnabled(anchor0, true)
		element:SetAnchorEdgeEnabled(anchor1, true)
		element:SetAnchorOffset(anchor0, margin0)
		element:SetAnchorOffset(anchor1, -margin1)
	end
end

function gui.apply_style_class(element, style)
	-- General properties
	if style.color then element:SetColor(Color.CreateFromHexColor(style.color:sub(2))) end
	if style.zPos then element:SetZPos(style.zPos) end
	if style.fixedSize then element:SetFixedSize(true) end
	if (style.fixedWidth or style.width) and element.SetFixedWidth then element:SetFixedWidth(true) end
	if (style.fixedHeight or style.height) and element.SetFixedHeight then element:SetFixedHeight(true) end
	if style.autoCenter then element:SetAlignment(gui.ALIGNMENT_CENTER) end
	if style.autoAlign then element:SetAlignment(gui.ALIGNMENT_FILL) end
	if style.visible ~= nil then element:SetVisible(style.visible) end
	if style.zIndex then element:SetZPos(style.zIndex) end

	if style.alignment then element:SetAlignment(to_alignment_enum(style.alignment)) end
	if style.alignmentX then element:SetHorizontalAlignment(to_alignment_enum(style.alignmentX)) end
	if style.alignmentY then element:SetVerticalAlignment(to_alignment_enum(style.alignmentY)) end
	if style.horizontalAlignment then element:SetHorizontalAlignment(to_alignment_enum(style.horizontalAlignment)) end
	if style.verticalAlignment then element:SetVerticalAlignment(to_alignment_enum(style.verticalAlignment)) end

	-- Text properties
	if style.font and element.SetFont then element:SetFont(style.font) end
	if style.textTransform and element.SetText then
		if style.textTransform == "uppercase" then
			element:SetText(element:GetText():upper())
		elseif style.textTransform == "lowercase" then
			element:SetText(element:GetText():lower())
		elseif style.textTransform == "capitalize" then
			local text = element:GetText()
			local capitalizeNext = true
			for i, c in ipairs(text) do
				if c == ' ' or c == '\t' then
					capitalizeNext = true
				else
					capitalizeNext = false
					text = text:sub(1, i -1) .. text:sub(i, i):upper() .. text:sub(i +1)
				end
			end
			element:SetText(text)
		end
	end

	if style.shadowOffset and element.EnableShadow then
		if style.shadowColor then
			element:SetShadowColor(Color.CreateFromHexColor(style.shadowColor:sub(2)))
		end
		element:SetShadowOffset(Vector2(style.shadowOffset[1], style.shadowOffset[2]))
		element:EnableShadow(true)
	end

	if style.material and element.SetMaterial then
		element:SetMaterial(style.material)
		element:SizeToTexture()
	end

	if style.anchor then element:SetAnchor(style.anchor[1], style.anchor[2], style.anchor[3], style.anchor[4]) end

	if style.anchorEdges then
		for edgeName, value in pairs(style.anchorEdges) do
			local edgeEnum = gui["ANCHOR_EDGE_" .. edgeName:upper()]
			if edgeEnum and element.SetAnchor then
				element:SetAnchor(edgeEnum, value)
				element:SetAnchorEdgeEnabled(edgeEnum, true)
			end
		end
	end

	apply_dimensions(element, style)

	if style.minWidth and element.SetMinWidth then
		element:SetMinWidth(style.minWidth)
	end
	if style.maxWidth and element.SetMaxWidth then
		element:SetMaxWidth(style.maxWidth)
	end
	if style.minHeight and element.SetMinHeight then
		element:SetMinHeight(style.minHeight)
	end
	if style.maxHeight and element.SetMaxHeight then
		element:SetMaxHeight(style.maxHeight)
	end

	if style.rowHeight and element.SetRowHeight then
		element:SetRowHeight(style.rowHeight)
	end

	if style.alignX or style.alignY then
		element:ClearAnchor()
		apply_anchor_align(
			element, style.alignX, "left", "right", gui.ANCHOR_EDGE_LEFT, gui.ANCHOR_EDGE_RIGHT,
			-- style.marginLeft, style.marginRight,
			0, 0,
			style.offsetX, math.AXIS_X
		)
		apply_anchor_align(
			element, style.alignY, "top", "bottom", gui.ANCHOR_EDGE_TOP, gui.ANCHOR_EDGE_BOTTOM,
			-- style.marginTop, style.marginBottom,
			0, 0,
			style.offsetY, math.AXIS_Y
		)
		element:UpdateAnchorTransform()
	end

	if style.alignItems and element.SetAlignItems then element:SetAlignItems(to_flex_align_enum(style.alignItems)) end
	if style.justifyContent and element.SetJustifyContent then element:SetJustifyContent(to_flex_justify_enum(style.justifyContent)) end

	if style.flex then
		local parent = element:GetParent()
		if parent and parent.SetChildFlex then
			parent:SetChildFlex(element, style.flex)
		end
	end

	if style.padding and element.SetPadding then
		local l, t, r, b = unpack_spacing(style.padding)
		element:SetPadding(l, t, r, b)
	end

	local parent = element:GetParent()
	if parent ~= nil and parent.SetChildMargin ~= nil then
		if style.margin or style.marginLeft or style.marginRight or style.marginTop or style.marginBottom then
			local l, t, r, b = 0, 0, 0, 0
			if style.margin then l, t, r, b = unpack_spacing(style.margin) end
			if style.marginLeft then l = tonumber(style.marginLeft) end
			if style.marginRight then r = tonumber(style.marginRight) end
			if style.marginTop then t = tonumber(style.marginTop) end
			if style.marginBottom then b = tonumber(style.marginBottom) end
			parent:SetChildMargin(element, l, t, r, b)
		end
	end
	
	if style.spacing and element.SetSpacing then
		element:SetSpacing(style.spacing)
	end

	local background = style.background
	if style.backgroundColor then
		background = background or {}
		background.color = style.backgroundColor
	end

	local bg = element:GetFirstChildByName("skin_background")
	if background then
		local simpleBackground = false
		if background.borderThickness == nil and background.cornerRadii == nil and background.gradient == nil and background.color ~= nil then
			simpleBackground = true
		end

		local bgClass = simpleBackground and "wirect" or "styled_rect"

		if bg and bg:GetClass():lower() ~= bgClass then
			util.remove(bg)
			bg = nil
		end

		if not bg then
			bg = gui.create(bgClass, element)
			bg:SetName("skin_background")
			bg:SetBackgroundElement(true)
			bg:SetZPos(-100)
			bg:SetAlignment(gui.ALIGNMENT_FILL)
		end
		bg:SetVisible(true) 

		if background.color then
			bg:SetColor(Color.CreateFromHexColor(background.color:sub(2)))
		end

		if not simpleBackground then
			if background.borderThickness then
				bg:SetBorderThickness(background.borderThickness)
				if background.borderColor then bg:SetBorderColor(Color.CreateFromHexColor(background.borderColor:sub(2))) end
			end

			if background.cornerRadii then
				bg:SetCornerRadii(Vector4(background.cornerRadii, background.cornerRadii, background.cornerRadii, background.cornerRadii))
			end

			bg:ClearGradient()
			if background.gradient then
				if background.gradient.type then
					local type = gui.StyledRect.GRADIENT_TYPE_LINEAR
					if background.gradient.type == "radial" then
						type = gui.StyledRect.GRADIENT_TYPE_RADIAL
					end
					bg:SetGradientType(type)
				end

				if background.gradient.start then
					bg:SetGradientStart(Vector2(background.gradient.start[1], background.gradient.start[2]))
				end
				if background.gradient["end"] then
					bg:SetGradientEnd(Vector2(background.gradient["end"][1], background.gradient["end"][2]))
				end

				if background.gradient.stops then
					local positions = {}
					local colors = {}
					for _, stop in ipairs(background.gradient.stops) do
						table.insert(positions, stop.pos)
						table.insert(colors, Color.CreateFromHexColor(stop.color:sub(2)))
					end
					bg:SetGradientStops(positions, colors)
				end
			end
		end
	elseif bg then
		bg:SetVisible(false)
	end

	local ol = element:GetFirstChildByName("skin_outline")
	local outlineWidth = style.outlineWidth and toint(style.outlineWidth) or 0
	if outlineWidth > 0.0 then
		if not ol then
			ol = gui.create("WIOutlinedRect", element)
			ol:SetName("skin_outline")
			ol:SetBackgroundElement(true)
			ol:SetZPos(10) 
			ol:SetAlignment(gui.ALIGNMENT_FILL)
		end
		ol:SetOutlineWidth(outlineWidth)
		if style.outlineColor ~= nil then
			ol:SetColor(Color.CreateFromHexColor(style.outlineColor:sub(2)))
		end
		ol:SetVisible(true)
	elseif ol then
		ol:SetVisible(false)
	end

	if style.sound then
		-- Ensure we only play the sound once
		if element.__lastSkinSound ~= style.sound then
			
			local playInfo = sound.PlayInfo()
			playInfo.pitch = 0.8 -- TODO: Let user define via skin
			sound.play(style.sound, sound.TYPE_GUI, playInfo)
			
			element.__lastSkinSound = style.sound
		end
	else
		element.__lastSkinSound = nil
	end

	if style.children then
		for selector, childStyle in pairs(style.children) do
			local matchingChildren = find_matching_children(element, selector)
			for _, child in ipairs(matchingChildren) do
				-- TODO: Do we need this here?
				if childStyle.margin then
					local immediateParent = child:GetParent()
					if immediateParent and immediateParent.SetChildMargin then
						local l, t, r, b = unpack_spacing(childStyle.margin)
						immediateParent:SetChildMargin(child, l, t, r, b)
					end
				end

				gui.apply_style_class(child, childStyle)
			end
		end
	end

	if style.decorators then
		for decName, decStyle in pairs(style.decorators) do
			local internalName = "skin_dec_" .. decName
			local decEl = element:GetFirstChildByName(internalName)
			
			if not decEl then
				local elementType = decStyle.type
				decEl = gui.create(elementType, element)
				
				if decEl then
					decEl:SetName(internalName)
					decEl:SetBackgroundElement(true, false)
					decEl:SetZPos(-80)
				else
					log.err("Failed to create decorator type: " .. tostring(elementType))
				end
			end
			
			if decEl then
				decEl:SetVisible(true)
				--decEl:Update()
				--decEl:RefreshSkin()
				gui.apply_style_class(decEl, decStyle)
			end
		end
	end
end
