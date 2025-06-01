-- Load required modules
-- RopeStateManager might not be directly needed here if we only set GrappleMode on the gun.
-- local RopeStateManager = require("Devices.Tools.GrappleGun.Scripts.RopeStateManager")

-- Action for Retract slice in the pie menu.
function GrapplePieRetract(pieMenuOwner, pieMenu, pieSlice)
    if pieMenuOwner and pieMenuOwner.EquippedItem then
        local gun = ToMOSRotating(pieMenuOwner.EquippedItem) -- Assume it's a MOSRotating
        if gun and gun.PresetName == "Grapple Gun" then -- Ensure it's the correct gun
            gun:SetNumberValue("GrappleMode", 1) -- 1 signifies Retract
        end
    end
end

-- Action for Extend slice in the pie menu.
function GrapplePieExtend(pieMenuOwner, pieMenu, pieSlice)
    if pieMenuOwner and pieMenuOwner.EquippedItem then
        local gun = ToMOSRotating(pieMenuOwner.EquippedItem)
        if gun and gun.PresetName == "Grapple Gun" then
            gun:SetNumberValue("GrappleMode", 2) -- 2 signifies Extend
        end
    end
end

-- Utility function to safely check if an object has a specific property (key) in its Lua script table.
-- This is useful for checking if a script-defined variable exists on an MO.
function HasScriptProperty(obj, propName)
    if type(obj) ~= "table" or type(propName) ~= "string" then
        return false
    end
    -- pcall to safely access potentially non-existent script members.
    -- This is more about checking Lua script-defined members rather than engine properties.
    local status, result = pcall(function() return rawget(obj, propName) ~= nil end)
    return status and result
end


-- Action for Unhook slice in the pie menu.
function GrapplePieUnhook(pieMenuOwner, pieMenu, pieSlice)
    if not pieMenuOwner or not pieMenuOwner.EquippedItem then
        return
    end

    local gun = ToMOSRotating(pieMenuOwner.EquippedItem)
    if not (gun and gun.PresetName == "Grapple Gun") then
        return -- Not the grapple gun
    end

    local activeGrappleMO = nil
    -- Find the active grapple claw associated with this specific gun instance.
    for mo_instance in MovableMan:GetMOsByPreset("Grapple Gun Claw") do
        -- Check if the instance is valid, has the parentGun property, and it matches our gun.
        if mo_instance and mo_instance.ID ~= rte.NoMOID and 
           HasScriptProperty(mo_instance, "parentGun") and -- Use HasScriptProperty for Lua members
           mo_instance.parentGun and mo_instance.parentGun.ID == gun.ID then
            activeGrappleMO = mo_instance
            break
        end
    end

    local allowUnhook = true -- Default to allowing unhook.
    if activeGrappleMO then
        -- Check the 'canRelease' property on the grapple claw instance itself.
        -- This property is set by the Grapple.lua script based on its state (e.g., after sticking).
        if HasScriptProperty(activeGrappleMO, "canRelease") then
            allowUnhook = (activeGrappleMO.canRelease == true)
        else
            -- If canRelease property doesn't exist, but grapple is active,
            -- it might imply it's in a state where it can be unhooked (e.g., already stuck).
            -- However, for safety, if 'canRelease' is the definitive flag, stick to it.
            -- If the grapple is flying (actionMode 1), 'canRelease' might be false.
            -- If it's stuck (actionMode > 1), 'canRelease' should become true.
            -- If actionMode is 1 (flying), pie menu unhook might not be desired or should just delete it.
            if HasScriptProperty(activeGrappleMO, "actionMode") and activeGrappleMO.actionMode == 1 then
                 allowUnhook = true -- Allow "unhooking" (deleting) a flying hook via pie menu
            elseif not HasScriptProperty(activeGrappleMO, "canRelease") then
                 allowUnhook = false -- If stuck and no canRelease flag, assume cannot release.
            end
        end
    else
        -- No active grapple found for this gun. Unhook action is irrelevant.
        allowUnhook = false 
    end

    if allowUnhook then
        gun:SetNumberValue("GrappleMode", 3) -- 3 signifies Unhook. Grapple.lua will handle this.
    else
        -- Play a denial sound if unhook is not allowed (e.g., hook is still flying and not releasable yet).
        local denySound = CreateSoundContainer("Grapple Gun Click", "Base.rte") -- Or a specific "deny" sound
        if denySound then
            denySound:Play(gun.Pos)
        end
    end
end