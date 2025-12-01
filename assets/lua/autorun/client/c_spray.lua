-- SPDX-FileCopyrightText: (c) 2021 Silverlan <opensource@pragma-engine.com>
-- SPDX-License-Identifier: MIT

local entSpray
local function spray()
	local matSprayLogo = console.get_convar_string("cl_logofile")

	local pl = ents.get_local_player()
	local charComponent = (pl ~= nil) and pl:GetEntity():GetComponent(ents.COMPONENT_CHARACTER) or nil
	if charComponent == nil then
		return
	end
	local rayData = charComponent:GetAimRayData(80.0)
	local ray = phys.raycast(rayData)
	if ray == false then
		return
	end

	if util.is_valid(entSpray) then
		entSpray:Remove()
	end

	local sprayPos = ray.position
	local decalPos = sprayPos + ray.normal * 1.0

	local forward = vector.UP
	if math.abs(ray.normal:DotProduct(forward)) > 0.99 then
		forward = vector.FORWARD
	end
	local rot = Quaternion(-ray.normal, forward)

	entSpray = ents.create("env_decal")
	entSpray:SetKeyValue("size", "32")
	entSpray:SetKeyValue("material", matSprayLogo)
	entSpray:SetPos(decalPos)
	entSpray:SetRotation(rot)
	entSpray:Spawn()

	local playInfo = sound.PlayInfo()
	playInfo.flags = bit.bor(playInfo.flags, sound.FCREATE_MONO)
	playInfo.origin = decalPos
	sound.play("player/sprayer.wav", sound.TYPE_EFFECT, playInfo)
end

console.register_command("spray", function(pl, joystickAxisMagnitude)
	spray()
end)

console.register_variable(
	"cl_logofile",
	udm.TYPE_STRING,
	"logo/test_spray",
	console.FLAG_BIT_ARCHIVE,
	"Material name for the spraylogo."
)
