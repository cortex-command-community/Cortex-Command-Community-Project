-- Load required modules
-- RopeStateManager might not be directly needed here if we only set GrappleMode on the gun.
-- local RopeStateManager = require("Devices.Tools.GrappleGun.Scripts.RopeStateManager")

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

-- Helper function to validate grapple gun
local function ValidateGrappleGun(pieMenuOwner)
    if not pieMenuOwner or not pieMenuOwner.EquippedItem then
        return nil
    end
    
    local gun = ToMOSRotating(pieMenuOwner.EquippedItem)
    if gun and gun.PresetName == "Grapple Gun" then
        return gun
    end
    
    return nil
end

-- Action for Retract slice in the pie menu.
function GrapplePieRetract(pieMenuOwner, pieMenu, pieSlice)
    local gun = ValidateGrappleGun(pieMenuOwner)
    if gun then
        gun:SetNumberValue("GrappleMode", 1) -- 1 signifies Retract
    end
end

-- Action for Extend slice in the pie menu.
function GrapplePieExtend(pieMenuOwner, pieMenu, pieSlice)
    local gun = ValidateGrappleGun(pieMenuOwner)
    if gun then
        gun:SetNumberValue("GrappleMode", 2) -- 2 signifies Extend
    end
end

-- Action for Unhook slice in the pie menu.
function GrapplePieUnhook(pieMenuOwner, pieMenu, pieSlice)
    local gun = ValidateGrappleGun(pieMenuOwner)
    if gun then
        gun:SetNumberValue("GrappleMode", 3) -- 3 signifies Unhook
    end
end