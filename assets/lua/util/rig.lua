--[[
    Copyright (C) 2019  Florian Weischer

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

rig = rig or {}

function rig.determine_head_bones(mdl)
	local skeleton = mdl:GetSkeleton()

	local headBoneId
	local headParentBoneId

	local min = Vector()
	local max = Vector()
	local hitboxBones = mdl:GetHitboxBones()
	for _,boneId in ipairs(hitboxBones) do
		local hbGroup = mdl:GetHitboxGroup(boneId)
		if(hbGroup == game.HITGROUP_HEAD) then
			headBoneId = boneId
			break
		end
	end

	if(headBoneId == nil) then
		if(#hitboxBones > 0) then
			local hitboxBoneMap = {}
			for _,boneId in ipairs(hitboxBones) do hitboxBoneMap[boneId] = true end
			local firstHeadBoneId
			local function findHeadBone(bone,isNeckBranch)
				local boneId = bone:GetID()
				if(hitboxBoneMap[boneId] ~= nil) then
					if(bone:GetName():lower():find("head")) then
						firstHeadBoneId = firstHeadBoneId or bone:GetID()
						if(isNeckBranch) then
							headBoneId = boneId
							return true
						end
					end
				end
				if(isNeckBranch == false) then
					local name = bone:GetName():lower()
					if(name:find("neck")) then
						isNeckBranch = true
					end
				end
				for boneId,child in pairs(bone:GetChildren()) do
					if(findHeadBone(child,isNeckBranch)) then break end
				end
				return headBoneId ~= nil
			end
			for boneId,bone in pairs(skeleton:GetRootBones()) do
				findHeadBone(bone,false)
			end
			headBoneId = headBoneId or firstHeadBoneId
		end
		if(headBoneId == nil) then
			-- TODO: Try something else
			-- Search for head +neck?
			-- If all fails, use full body
			return
		end
	end

	local bone = skeleton:GetBone(headBoneId)
	if(headParentBoneId == nil) then
		local parent = bone:GetParent()
		if(parent ~= nil) then headParentBoneId = parent:GetID() end
	end

	min,max = mdl:GetHitboxBounds(headBoneId)

	local headBones = {}
	local function iterate_hierarchy(bone,head)
		if(head) then headBones[bone:GetID()] = true end
		for boneId,child in pairs(bone:GetChildren()) do
			local isHead = (head or boneId == headBoneId)
			iterate_hierarchy(child,isHead)
		end
	end
	for boneId,bone in pairs(skeleton:GetRootBones()) do
		iterate_hierarchy(bone,false)
	end
	return {
		headBounds = {min,max},
		headBoneId = headBoneId,
		headParentBoneId = headParentBoneId,
		headBones = headBones
	}
end
