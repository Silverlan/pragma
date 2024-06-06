--[[
    Copyright (C) 2023 Silverlan

    This Source Code Form is subject to the terms of the Mozilla Public
    License, v. 2.0. If a copy of the MPL was not distributed with this
    file, You can obtain one at http://mozilla.org/MPL/2.0/.
]]

util = util or {}
util.rig = util.rig or {}

function util.rig.determine_mirrored_bone(mdl, boneName)
	local skeleton = mdl:GetSkeleton()
	local lnameToActualName = {}
	for _, bone in ipairs(skeleton:GetBones()) do
		lnameToActualName[bone:GetName():lower()] = bone:GetName()
	end

	local lname = boneName:lower()
	local function check_candidate(identifier, identifierOther)
		local pos = lname:find(identifier)
		if pos ~= nil then
			local otherName = lname:sub(0, pos - 1) .. identifierOther .. lname:sub(pos + #identifier)
			if lnameToActualName[otherName] ~= nil then
				return lnameToActualName[otherName]
			end
		end
	end
	local candidate = check_candidate("left", "right")
	candidate = candidate or check_candidate("right", "left")
	candidate = candidate or check_candidate("_l_", "_r_")
	candidate = candidate or check_candidate("_r_", "_l_")
	candidate = candidate or check_candidate("_l", "_r")
	candidate = candidate or check_candidate("r_", "l_")
	return candidate
end

function util.rig.determine_mirrored_bone_flip_factors(mdl, boneLeft, boneRight)
	local skeleton = mdl:GetSkeleton()
	local boneIdLeft = skeleton:LookupBone(boneLeft)
	local boneIdRight = skeleton:LookupBone(boneRight)

	local ref = mdl:GetReferencePose()
	local poseLeft = ref:GetBonePose(boneIdLeft)
	local poseRight = ref:GetBonePose(boneIdRight)
	local angLeft = poseLeft:GetRotation():ToEulerAngles()
	local angRight = poseRight:GetRotation():ToEulerAngles()
	local pdiff = math.abs(math.get_angle_difference(angRight.p, angLeft.p))
	local ydiff = math.abs(math.get_angle_difference(angRight.y, angLeft.y))
	local rdiff = math.abs(math.get_angle_difference(angRight.r, angLeft.r))

	local rollAxis = math.AXIS_Z
	rollAxis = mdl:FindBoneTwistAxis(boneIdLeft) or rollAxis

	-- TODO: This may not be correct
	local pitchAxis
	if rollAxis == math.AXIS_X or rollAxis == math.AXIS_SIGNED_X then
		pitchAxis = math.AXIS_Y
	elseif rollAxis == math.AXIS_Y or rollAxis == math.AXIS_SIGNED_Y then
		pitchAxis = math.AXIS_Z
	else
		pitchAxis = math.AXIS_X
	end

	local flipFactors = Vector(-1, -1, -1)
	flipFactors:Set(pitchAxis, 1)
	return flipFactors
end

function util.rig.determine_head_bones(mdl)
	local skeleton = mdl:GetSkeleton()
	local function getHeadBones(headBoneId)
		local headBones = {}
		local function iterate_hierarchy(bone, head)
			if head then
				headBones[bone:GetID()] = true
			end
			for boneId, child in pairs(bone:GetChildren()) do
				local isHead = (head or boneId == headBoneId)
				iterate_hierarchy(child, isHead)
			end
		end
		for boneId, bone in pairs(skeleton:GetRootBones()) do
			iterate_hierarchy(bone, false)
		end
		return headBones
	end
	local metaRig = mdl:GetMetaRig()
	if metaRig ~= nil then
		local boneHead = metaRig:GetBone(game.Model.MetaRig.BONE_TYPE_HEAD)
		if boneHead ~= nil then
			local bone = skeleton:GetBone(boneHead.boneId)
			local headParentBoneId
			if headParentBoneId == nil then
				local parent = bone:GetParent()
				if parent ~= nil then
					headParentBoneId = parent:GetID()
				end
			end
			return {
				headBounds = { boneHead.min, boneHead.max },
				headBoneId = boneHead.boneId,
				headParentBoneId = headParentBoneId,
				headBones = getHeadBones(boneHead.boneId),
			}
		end
	end

	local headBoneId
	local headParentBoneId

	local min = Vector()
	local max = Vector()
	local hitboxBones = mdl:GetHitboxBones()
	for _, boneId in ipairs(hitboxBones) do
		local hbGroup = mdl:GetHitboxGroup(boneId)
		if hbGroup == game.HITGROUP_HEAD then
			headBoneId = boneId
			break
		end
	end

	if headBoneId == nil then
		if #hitboxBones > 0 then
			local hitboxBoneMap = {}
			for _, boneId in ipairs(hitboxBones) do
				hitboxBoneMap[boneId] = true
			end
			local firstHeadBoneId
			local function findHeadBone(bone, isNeckBranch)
				local boneId = bone:GetID()
				if hitboxBoneMap[boneId] ~= nil then
					if bone:GetName():lower():find("head") then
						firstHeadBoneId = firstHeadBoneId or bone:GetID()
						if isNeckBranch then
							headBoneId = boneId
							return true
						end
					end
				end
				if isNeckBranch == false then
					local name = bone:GetName():lower()
					if name:find("neck") then
						isNeckBranch = true
					end
				end
				for boneId, child in pairs(bone:GetChildren()) do
					if findHeadBone(child, isNeckBranch) then
						break
					end
				end
				return headBoneId ~= nil
			end
			for boneId, bone in pairs(skeleton:GetRootBones()) do
				findHeadBone(bone, false)
			end
			headBoneId = headBoneId or firstHeadBoneId
		end
		if headBoneId == nil then
			local headBoneName
			for boneId, bone in pairs(skeleton:GetBones()) do
				local name = bone:GetName()
				local lname = name:lower()
				if lname:find("head") then
					if headBoneName == nil or #name < #headBoneName then
						headBoneName = name
					end
				end
			end
			if headBoneName == nil then
				return
			end
			headBoneId = skeleton:LookupBone(headBoneName)
		end
	end

	local bone = skeleton:GetBone(headBoneId)
	if headParentBoneId == nil then
		local parent = bone:GetParent()
		if parent ~= nil then
			headParentBoneId = parent:GetID()
		end
	end

	min, max = mdl:GetHitboxBounds(headBoneId)

	return {
		headBounds = { min, max },
		headBoneId = headBoneId,
		headParentBoneId = headParentBoneId,
		headBones = getHeadBones(headBoneId),
	}
end
