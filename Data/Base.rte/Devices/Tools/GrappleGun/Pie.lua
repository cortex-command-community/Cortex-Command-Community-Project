-- Load required modules
local RopeStateManager = require("Base.rte.Devices.Tools.GrappleGun.Scripts.RopeStateManager")

function GrapplePieRetract(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun then
		ToMOSRotating(gun):SetNumberValue("GrappleMode", 1);
	end
end

function GrapplePieExtend(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun then
		ToMOSRotating(gun):SetNumberValue("GrappleMode", 2);
	end
end

-- Helper function to safely check if a table has an attribute/property
function HasProperty(obj, prop)
    local status, result = pcall(function() return obj[prop] ~= nil end)
    return status and result
end

function GrapplePieUnhook(pieMenuOwner, pieMenu, pieSlice)
	local gun = pieMenuOwner.EquippedItem;
	if gun and IsMOSRotating(gun) then -- Ensure gun is valid
		local grappleMO = nil
		-- Find the active grapple claw associated with this gun
		for mo_instance in MovableMan:GetMOsByPreset("Grapple Gun Claw") do
			if mo_instance and mo_instance:IsScriptActor() and 
               HasProperty(mo_instance, "parentGun") and 
               mo_instance.parentGun and 
               mo_instance.parentGun.ID == gun.ID then
				grappleMO = mo_instance;
				break;
			end
		end

		local allowUnhook = true;
		-- Use RopeStateManager if available, otherwise fall back to direct property check
		if grappleMO then
			if HasProperty(RopeStateManager, "canReleaseGrapple") then
				allowUnhook = RopeStateManager.canReleaseGrapple(grappleMO)
			elseif HasProperty(grappleMO, "canRelease") then
				allowUnhook = grappleMO.canRelease ~= false
			end
		end

		if allowUnhook then
			ToMOSRotating(gun):SetNumberValue("GrappleMode", 3); -- 3 will signify Unhook
		else
			-- Play a denial sound
			local denySound = CreateSoundContainer("Grapple Gun Click", "Base.rte");
			denySound:Play(gun.Pos);
		end
	end
end