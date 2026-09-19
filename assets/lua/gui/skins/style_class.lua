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
		element:SetX(0)
		element:SetAnchor(gui.ANCHOR_EDGE_LEFT, x_val)
	elseif x_val then
		element:SetX(toint(x_val))
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
			element:SetWidth(toint(parent:GetWidth() *w_val))
		end
		
		if not x_is_pct and not style.anchor then
			element:SetAnchor(gui.ANCHOR_EDGE_LEFT, base_left)
		end
		-- Set Right anchor dynamically
		element:SetAnchor(gui.ANCHOR_EDGE_RIGHT, base_left + w_val)
	elseif w_val then
		element:SetWidth(toint(w_val))
	end

	-- y / height
	if y_is_pct then
		element:SetY(0)
		element:SetAnchor(gui.ANCHOR_EDGE_TOP, y_val)
	elseif y_val then
		element:SetY(toint(y_val))
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
			element:SetHeight(toint(element:GetParent():GetHeight() *h_val))
		end
		
		if not y_is_pct and not style.anchor then
			element:SetAnchor(gui.ANCHOR_EDGE_TOP, base_top)
		end
		-- Set Bottom anchor dynamically
		element:SetAnchor(gui.ANCHOR_EDGE_BOTTOM, base_top + h_val)
	elseif h_val then
		element:SetHeight(toint(h_val))
	end

	if w_is_auto ~= h_is_auto then
		local aspectRatio = (origSize.y > 0) and (origSize.x /origSize.y) or 0.0
		if w_is_auto then
			local w = element:GetHeight() *aspectRatio
			element:SetWidth(w)
		else
			local h = (aspectRatio > 0.0001) and (element:GetWidth() /aspectRatio) or 0.0
			element:SetHeight(h)
		end
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

    apply_dimensions(element, style)

	if style.alignItems and element.SetAlignItems then element:SetAlignItems(style.alignItems) end

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

	if style.margin then
		local parent = element:GetParent()
		if parent ~= nil and parent.SetChildMargin ~= nil then
			local l, t, r, b = unpack_spacing(style.margin)
			parent:SetChildMargin(element, l, t, r, b)
		end
	end
	
	if style.spacing and element.SetSpacing then
		element:SetSpacing(style.spacing)
	end

	local bg = element:GetFirstChildByName("skin_background")
	if style.backgroundColor or style.backgroundGradient then
		if not bg then
			local className = style.backgroundGradient and "WITexturedRect" or "WIRect"
			bg = gui.create(className, element)
			bg:SetName("skin_background")
			bg:SetZPos(100)
			bg:SetBackgroundElement(true)
			bg:SetAlignment(gui.ALIGNMENT_FILL)
		end
		bg:SetVisible(true) 
		
		if style.backgroundGradient then
            -- Test
			style.backgroundGradient = {
				dir = Vector2(0, -1),
				width = 128,
				height = 64,
				nodes = {
					{
						offset = 0.0,
						color = "#ff0000"
					},
					{
						offset = 1.0,
						color = "#00ff00"
					}
				}
			}

			local tex = get_cached_gradient(style.backgroundGradient)
			if bg.SetTexture then
				bg:SetTexture(tex)
			end
            -- Disable base color
			bg:SetColor(Color.CreateFromHexColor("ffffff"))
			
		elseif style.backgroundColor then
			if bg.SetTexture then 
				bg:SetTexture(nil) 
			end
			bg:SetColor(Color.CreateFromHexColor(style.backgroundColor:sub(2)))
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
			ol:SetZPos(-90) 
			ol:SetBackgroundElement(true)
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
                    decEl:SetBackgroundElement(true)
                    decEl:SetZPos(-80)
				else
					log.err("Failed to create decorator type: " .. tostring(elementType))
				end
			end
			
			if decEl then
				decEl:SetVisible(true)
				gui.apply_style_class(decEl, decStyle)
			end
		end
	end
end
