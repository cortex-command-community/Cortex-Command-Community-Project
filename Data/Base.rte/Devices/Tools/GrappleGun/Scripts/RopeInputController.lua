-- Grapple Gun Input Controller Module
-- Handles user input for rope control.
-- Translates raw input into actions for the main grapple logic.

local Logger = require("Devices.Tools.GrappleGun.Scripts.Logger")
local RopeInputController = {}

-- Check if player is currently holding the grapple gun (equipped in main or background hand)
local function isCurrentlyEquipped(grappleInstance)
    if not grappleInstance.parent then 
        Logger.debug("RopeInputController.isCurrentlyEquipped() - No parent")
        return false 
    end
    
    local parent = grappleInstance.parent
    
    -- Check main equipped item
    local mainEquipped = false
    if parent.EquippedItem and parent.EquippedItem.PresetName == "Grapple Gun" then
        mainEquipped = true
        -- Always update our reference when we find the gun equipped
        grappleInstance.parentGun = ToHDFirearm(parent.EquippedItem)
        Logger.debug("RopeInputController.isCurrentlyEquipped() - Updated parentGun reference from main hand (ID: %d)", grappleInstance.parentGun.ID)
    end
    
    -- Check background equipped item
    local bgEquipped = false
    if parent.EquippedBGItem and parent.EquippedBGItem.PresetName == "Grapple Gun" then
        bgEquipped = true
        -- Always update our reference when we find the gun equipped
        grappleInstance.parentGun = ToHDFirearm(parent.EquippedBGItem)
        Logger.debug("RopeInputController.isCurrentlyEquipped() - Updated parentGun reference from BG hand (ID: %d)", grappleInstance.parentGun.ID)
    end
    
    -- Additional check: see if gun's RootID matches parent (if we have a valid parentGun)
    local rootEquipped = false
    if grappleInstance.parentGun and grappleInstance.parentGun.ID ~= rte.NoMOID then
        if grappleInstance.parentGun.RootID == parent.ID then
            rootEquipped = true
            Logger.debug("RopeInputController.isCurrentlyEquipped() - Gun root matches parent ID")
        else
            Logger.debug("RopeInputController.isCurrentlyEquipped() - Gun root mismatch: gun RootID=%d, parent ID=%d", 
                         grappleInstance.parentGun.RootID, parent.ID)
        end
    end
    
    local isEquipped = mainEquipped or bgEquipped or rootEquipped
    
    Logger.debug("RopeInputController.isCurrentlyEquipped() - Equipment check: main=%s, bg=%s, root=%s, final=%s", 
                 tostring(mainEquipped), tostring(bgEquipped), tostring(rootEquipped), tostring(isEquipped))
    
    -- Debug additional info about current equipment state
    if parent.EquippedItem then
        Logger.debug("RopeInputController.isCurrentlyEquipped() - Main equipped: %s (ID: %d)", 
                     parent.EquippedItem.PresetName or "Unknown", parent.EquippedItem.ID)
    else
        Logger.debug("RopeInputController.isCurrentlyEquipped() - No main equipped item")
    end
    
    if parent.EquippedBGItem then
        Logger.debug("RopeInputController.isCurrentlyEquipped() - BG equipped: %s (ID: %d)", 
                     parent.EquippedBGItem.PresetName or "Unknown", parent.EquippedBGItem.ID)
    else
        Logger.debug("RopeInputController.isCurrentlyEquipped() - No BG equipped item")
    end
    
    if grappleInstance.parentGun then
        Logger.debug("RopeInputController.isCurrentlyEquipped() - Parent gun: %s (ID: %d, RootID: %d)", 
                     grappleInstance.parentGun.PresetName or "Unknown", grappleInstance.parentGun.ID, grappleInstance.parentGun.RootID)
    else
        Logger.debug("RopeInputController.isCurrentlyEquipped() - No parent gun reference")
    end
    
    return isEquipped
end

-- Check if gun exists in player's inventory
local function isInInventory(grappleInstance)
    if not grappleInstance.parent or not grappleInstance.parent.Inventory then 
        Logger.debug("RopeInputController.isInInventory() - Missing parent or inventory")
        return false 
    end
    
    local inventoryCount = 0
    for item in grappleInstance.parent.Inventory do
        inventoryCount = inventoryCount + 1
        if item then
            Logger.debug("RopeInputController.isInInventory() - Inventory item %d: %s (ID: %d)", 
                         inventoryCount, item.PresetName or "Unknown", item.ID)
            if item.PresetName == "Grapple Gun" then
                -- Always update our reference when we find the gun in inventory
                grappleInstance.parentGun = ToHDFirearm(item)
                Logger.debug("RopeInputController.isInInventory() - Updated parentGun reference from inventory (ID: %d)", grappleInstance.parentGun.ID)
                return true
            end
        else
            Logger.debug("RopeInputController.isInInventory() - Inventory item %d: nil", inventoryCount)
        end
    end
    
    Logger.debug("RopeInputController.isInInventory() - Gun not found in inventory (%d items checked)", inventoryCount)
    return false
end

-- Handle gun persistence - ensure grapple stays active even when gun changes hands/inventory
function RopeInputController.handleGunPersistence(grappleInstance)
    if not grappleInstance.parent or not grappleInstance.parentGun then 
        Logger.warn("RopeInputController.handleGunPersistence() - Missing parent or parentGun")
        return false 
    end
    
    Logger.debug("RopeInputController.handleGunPersistence() - Checking gun persistence")
    
    -- Check if gun still exists and is accessible to the player
    local gunIsAccessible = isCurrentlyEquipped(grappleInstance) or 
                           isInInventory(grappleInstance) or
                           (grappleInstance.parentGun.RootID == rte.NoMOID and 
                            SceneMan:ShortestDistance(grappleInstance.parent.Pos, grappleInstance.parentGun.Pos, SceneMan.SceneWrapsX).Magnitude < 100)
    
    if not gunIsAccessible then
        -- Gun was completely removed or taken by someone else
        Logger.warn("RopeInputController.handleGunPersistence() - Gun no longer accessible, grapple will remain but controls limited")
        return false
    end
    
    Logger.debug("RopeInputController.handleGunPersistence() - Gun still accessible, updating magazine state")
    
    -- Gun still exists somewhere - keep grapple active
    -- Update magazine state regardless of where gun is
    if grappleInstance.parentGun.Magazine and MovableMan:IsParticle(grappleInstance.parentGun.Magazine) then
        local mag = ToMOSParticle(grappleInstance.parentGun.Magazine)
        mag.RoundCount = 0 -- Keep showing as "fired"
        mag.Scale = 0 -- Keep hidden while grapple is active
        Logger.debug("RopeInputController.handleGunPersistence() - Magazine state updated (hidden, empty)")
    end
    
    return true
end

-- Handle R key unhooking (only when gun is equipped)
function RopeInputController.handleReloadKeyUnhook(grappleInstance, controller)
    if not controller then 
        Logger.debug("RopeInputController.handleReloadKeyUnhook() - No controller provided")
        return false 
    end
    
    Logger.debug("RopeInputController.handleReloadKeyUnhook() - Checking reload key state")
    
    if isCurrentlyEquipped(grappleInstance) and controller:IsState(Controller.WEAPON_RELOAD) then
        Logger.info("RopeInputController.handleReloadKeyUnhook() - R key pressed while holding grapple gun - unhooking!")
        return true
    end
    
    Logger.debug("RopeInputController.handleReloadKeyUnhook() - No unhook condition met")
    return false
end

-- Handle double-tap crouch unhooking (only when gun is NOT equipped but in inventory)
function RopeInputController.handleTapDetection(grappleInstance, controller)
    if not controller or not grappleInstance.parent then 
        Logger.debug("RopeInputController.handleTapDetection() - No controller or parent")
        return false 
    end

    Logger.debug("RopeInputController.handleTapDetection() - Processing tap detection, counter: %d", grappleInstance.tapCounter)

    -- Only allow tap unhooking when gun is NOT equipped but IS in inventory
    if isCurrentlyEquipped(grappleInstance) then
        -- Reset tap state when gun is equipped
        if grappleInstance.tapCounter > 0 then
            Logger.debug("RopeInputController.handleTapDetection() - Gun equipped, resetting tap counter")
        end
        grappleInstance.tapCounter = 0
        grappleInstance.canTap = true
        return false
    end
    
    if not isInInventory(grappleInstance) then
        Logger.debug("RopeInputController.handleTapDetection() - Gun not in inventory")
        return false -- Gun not in inventory at all
    end

    -- Process tap detection
    local proneState = controller:IsState(Controller.BODY_PRONE)
    
    if proneState then
        if grappleInstance.canTap then
            controller:SetState(Controller.BODY_PRONE, false) -- Clear prone state
            
            grappleInstance.tapCounter = grappleInstance.tapCounter + 1
            grappleInstance.canTap = false
            grappleInstance.tapTimer:Reset()
            
            Logger.info("RopeInputController.handleTapDetection() - Crouch tap %d detected (gun not equipped)", grappleInstance.tapCounter)
        else
            Logger.debug("RopeInputController.handleTapDetection() - Crouch held but can't tap yet")
        end
    else
        if not grappleInstance.canTap then
            Logger.debug("RopeInputController.handleTapDetection() - Crouch released, can tap again")
        end
        grappleInstance.canTap = true
    end
    
    -- Check for successful double-tap
    if grappleInstance.tapTimer:IsPastSimMS(grappleInstance.tapTime) then
        if grappleInstance.tapCounter > 0 then
            Logger.debug("RopeInputController.handleTapDetection() - Tap timeout, resetting counter")
        end
        grappleInstance.tapCounter = 0 -- Reset if too much time passed
    elseif grappleInstance.tapCounter >= grappleInstance.tapAmount then
        grappleInstance.tapCounter = 0
        Logger.info("RopeInputController.handleTapDetection() - Double crouch-tap while gun not equipped - unhooking!")
        return true
    end
    
    return false
end

-- Handle precise rope control with Shift+Mousewheel
function RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    if not controller or not grappleInstance.parent then 
        return false 
    end
    
    print("[SHIFT+WHEEL DEBUG] Starting shift mousewheel check")
    
    -- Only allow when gun is equipped and grapple is attached
    if grappleInstance.actionMode <= 1 then
        print("[SHIFT+WHEEL DEBUG] Action mode is " .. grappleInstance.actionMode .. " (not attached)")
        return false
    end
    
    if not isCurrentlyEquipped(grappleInstance) then
        print("[SHIFT+WHEEL DEBUG] Gun not currently equipped")
        return false
    end
    
    print("[SHIFT+WHEEL DEBUG] Equipment and attachment checks passed")
    
    -- Check for actual keyboard SHIFT key
    local shiftHeld = controller:IsState(Controller.KEYBOARD_SHIFT)
    print("[SHIFT+WHEEL DEBUG] Keyboard SHIFT held (KEYBOARD_SHIFT): " .. tostring(shiftHeld))
    
    if not shiftHeld then
        return false
    end
    
    -- Check for mouse wheel input
    local scrollUp = controller:IsState(Controller.SCROLL_UP)
    local scrollDown = controller:IsState(Controller.SCROLL_DOWN)
    
    print("[SHIFT+WHEEL DEBUG] Scroll up: " .. tostring(scrollUp) .. ", Scroll down: " .. tostring(scrollDown))
    
    if not scrollUp and not scrollDown then
        return false
    end
    
    print("[SHIFT+WHEEL DEBUG] SHIFT + Mousewheel detected!")
    
    -- IMPORTANT: Clear the scroll states to prevent weapon switching
    controller:SetState(Controller.SCROLL_UP, false)
    controller:SetState(Controller.SCROLL_DOWN, false)
    controller:SetState(Controller.WEAPON_CHANGE_NEXT, false)
    controller:SetState(Controller.WEAPON_CHANGE_PREV, false)
    
    -- Apply precise rope length control
    local preciseScrollSpeed = grappleInstance.shiftScrollSpeed or 1.0
    local lengthChange = 0
    
    if scrollUp then
        lengthChange = -preciseScrollSpeed -- Shorten rope
        print("[SHIFT+WHEEL DEBUG] Shortening rope by " .. preciseScrollSpeed)
    elseif scrollDown then
        lengthChange = preciseScrollSpeed -- Lengthen rope
        print("[SHIFT+WHEEL DEBUG] Lengthening rope by " .. preciseScrollSpeed)
    end
    
    -- Update rope length
    local oldLength = grappleInstance.currentLineLength
    grappleInstance.currentLineLength = math.max(10, math.min(grappleInstance.currentLineLength + lengthChange, grappleInstance.maxLineLength))
    grappleInstance.setLineLength = grappleInstance.currentLineLength
    
    print("[SHIFT+WHEEL DEBUG] Rope length changed from " .. oldLength .. " to " .. grappleInstance.currentLineLength)
    
    -- Clear any automatic selections since user is manually controlling
    grappleInstance.pieSelection = 0
    
    return true
end

-- Handle mouse wheel scrolling for rope control
function RopeInputController.handleMouseWheelControl(grappleInstance, controller)
    if not controller or not controller:IsMouseControlled() then 
        Logger.debug("RopeInputController.handleMouseWheelControl() - No controller or not mouse controlled")
        return 
    end
    
    -- Only allow rope controls if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then 
        Logger.debug("RopeInputController.handleMouseWheelControl() - Gun not equipped")
        return 
    end
    
    Logger.debug("RopeInputController.handleMouseWheelControl() - Processing mouse wheel input")
    
    -- Clear weapon change states
    controller:SetState(Controller.WEAPON_CHANGE_NEXT, false)
    controller:SetState(Controller.WEAPON_CHANGE_PREV, false)
    
    -- Check if shift is held for precise control
    local shiftHeld = controller:IsState(Controller.BODY_JUMPSTART) or controller:IsState(Controller.BODY_CROUCH)
    if shiftHeld then
        Logger.debug("RopeInputController.handleMouseWheelControl() - Shift held, using precise controls")
        RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    else
        -- Normal mousewheel behavior
        if controller:IsState(Controller.SCROLL_UP) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.climb = 3 -- Mouse retract
            Logger.info("RopeInputController.handleMouseWheelControl() - Mouse wheel up - retracting rope")
        elseif controller:IsState(Controller.SCROLL_DOWN) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.climb = 4 -- Mouse extend
            Logger.info("RopeInputController.handleMouseWheelControl() - Mouse wheel down - extending rope")
        end
    end
end

-- Handle directional key controls for climbing
function RopeInputController.handleDirectionalControl(grappleInstance, controller)
    if not controller or controller:IsMouseControlled() or grappleInstance.actionMode <= 1 then 
        Logger.debug("RopeInputController.handleDirectionalControl() - Invalid state for directional control")
        return 
    end
    
    -- Only allow rope controls if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then 
        Logger.debug("RopeInputController.handleDirectionalControl() - Gun not equipped")
        return 
    end
    
    Logger.debug("RopeInputController.handleDirectionalControl() - Checking directional input")
    
    if controller:IsState(Controller.HOLD_UP) then
        if grappleInstance.currentLineLength > grappleInstance.climbInterval then
            grappleInstance.climb = 1 -- Key retract
            Logger.info("RopeInputController.handleDirectionalControl() - Up key held - retracting rope")
        else
            Logger.debug("RopeInputController.handleDirectionalControl() - Up key held but rope too short to retract")
        end
    elseif controller:IsState(Controller.HOLD_DOWN) then
        if grappleInstance.currentLineLength < (grappleInstance.maxLineLength - grappleInstance.climbInterval) then
            grappleInstance.climb = 2 -- Key extend
            Logger.info("RopeInputController.handleDirectionalControl() - Down key held - extending rope")
        else
            Logger.debug("RopeInputController.handleDirectionalControl() - Down key held but rope at max length")
        end
    end
    
    -- Clear aim states if directional keys are used
    if controller:IsState(Controller.HOLD_UP) or controller:IsState(Controller.HOLD_DOWN) then
        controller:SetState(Controller.AIM_UP, false)
        controller:SetState(Controller.AIM_DOWN, false)
        Logger.debug("RopeInputController.handleDirectionalControl() - Cleared aim states")
    end
end

-- Main rope pulling handler
function RopeInputController.handleRopePulling(grappleInstance)
    if not grappleInstance.parent then 
        return 
    end
    
    local controller = grappleInstance.parent:GetController()
    if not controller then 
        return 
    end
    
    print("[ROPE PULLING DEBUG] Starting rope pulling handler")
    
    -- Handle SHIFT + Mousewheel for precise control first
    if RopeInputController.handleShiftMousewheelControls(grappleInstance, controller) then
        print("[ROPE PULLING DEBUG] SHIFT + Mousewheel handled, returning")
        return -- If shift+mousewheel was handled, don't process other inputs
    end
    
    -- Handle regular mouse wheel control
    RopeInputController.handleMouseWheelControl(grappleInstance, controller)
    
    -- Handle directional key controls
    RopeInputController.handleDirectionalControl(grappleInstance, controller)
end

-- Handle pie menu selections
function RopeInputController.handlePieMenuSelection(grappleInstance)
    if not grappleInstance.parentGun or grappleInstance.parentGun.ID == rte.NoMOID then 
        Logger.debug("RopeInputController.handlePieMenuSelection() - No parent gun")
        return false 
    end
    
    -- Only allow pie menu controls if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then 
        Logger.debug("RopeInputController.handlePieMenuSelection() - Gun not equipped")
        return false 
    end
    
    Logger.debug("RopeInputController.handlePieMenuSelection() - Checking for pie menu commands")
    
    local mode = grappleInstance.parentGun:GetNumberValue("GrappleMode")
    
    if mode and mode ~= 0 then
        grappleInstance.parentGun:RemoveNumberValue("GrappleMode")
        Logger.info("RopeInputController.handlePieMenuSelection() - Pie menu mode %d selected", mode)
        
        if mode == 3 then
            Logger.info("RopeInputController.handlePieMenuSelection() - Unhook command from pie menu")
            return true -- Unhook via pie menu
        elseif grappleInstance.actionMode > 1 then
            grappleInstance.pieSelection = mode
            grappleInstance.climb = 0
            Logger.info("RopeInputController.handlePieMenuSelection() - Pie selection set to %d", mode)
        end
    end
    return false
end

-- Handle automatic retraction
function RopeInputController.handleAutoRetraction(grappleInstance, terrCheck)
    if not grappleInstance.parentGun or grappleInstance.parentGun.ID == rte.NoMOID or grappleInstance.actionMode <= 1 then
        if grappleInstance.pieSelection ~= 0 then
            Logger.debug("RopeInputController.handleAutoRetraction() - Clearing pie selection (invalid state)")
        end
        grappleInstance.pieSelection = 0
        return
    end
    
    -- Only allow auto retraction if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then 
        if grappleInstance.pieSelection ~= 0 then
            Logger.debug("RopeInputController.handleAutoRetraction() - Clearing pie selection (gun not equipped)")
        end
        grappleInstance.pieSelection = 0
        return 
    end
    
    Logger.debug("RopeInputController.handleAutoRetraction() - Processing auto retraction, pieSelection: %d", grappleInstance.pieSelection)
    
    local parentForces = 1.0
    if grappleInstance.parent and grappleInstance.parent.Vel and grappleInstance.parent.Mass and grappleInstance.lineLength > 0 then
        parentForces = 1 + (grappleInstance.parent.Vel.Magnitude * 10 + grappleInstance.parent.Mass) / (1 + grappleInstance.lineLength)
        parentForces = math.max(0.1, parentForces)
        Logger.debug("RopeInputController.handleAutoRetraction() - Parent forces calculated: %.2f", parentForces)
    end
    
    -- Auto retraction when gun is activated
    if grappleInstance.parentGun:IsActivated() and grappleInstance.pieSelection == 0 then
        if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
            grappleInstance.climbTimer:Reset()
            if grappleInstance.currentLineLength > grappleInstance.autoClimbIntervalA then
                local oldLength = grappleInstance.currentLineLength
                grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.autoClimbIntervalA / parentForces)
                grappleInstance.setLineLength = grappleInstance.currentLineLength
                Logger.info("RopeInputController.handleAutoRetraction() - Gun activated: auto retract %.1f -> %.1f", oldLength, grappleInstance.currentLineLength)
            else
                Logger.debug("RopeInputController.handleAutoRetraction() - Gun activated but rope too short to retract")
            end
        end
    end
    
    -- Pie menu controlled retraction/extension
    if grappleInstance.pieSelection ~= 0 then
        if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
            grappleInstance.climbTimer:Reset()
            local actionTaken = false
            local oldLength = grappleInstance.currentLineLength
            
            if grappleInstance.pieSelection == 1 then -- Retract
                if grappleInstance.currentLineLength > grappleInstance.autoClimbIntervalA then
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.autoClimbIntervalA / parentForces)
                    actionTaken = true
                    Logger.info("RopeInputController.handleAutoRetraction() - Pie retract: %.1f -> %.1f", oldLength, grappleInstance.currentLineLength)
                else
                    Logger.debug("RopeInputController.handleAutoRetraction() - Pie retract: rope too short")
                end
            elseif grappleInstance.pieSelection == 2 then -- Extend
                if grappleInstance.currentLineLength < (grappleInstance.maxLineLength - grappleInstance.autoClimbIntervalB) then
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength + grappleInstance.autoClimbIntervalB
                    actionTaken = true
                    Logger.info("RopeInputController.handleAutoRetraction() - Pie extend: %.1f -> %.1f", oldLength, grappleInstance.currentLineLength)
                else
                    Logger.debug("RopeInputController.handleAutoRetraction() - Pie extend: rope at max length")
                end
            end
            
            grappleInstance.setLineLength = grappleInstance.currentLineLength
            if not actionTaken then
                Logger.info("RopeInputController.handleAutoRetraction() - Pie action complete, clearing selection")
                grappleInstance.pieSelection = 0
            end
        end
    end
    
    local clampedLength = math.max(10, math.min(grappleInstance.currentLineLength, grappleInstance.maxLineLength))
    if clampedLength ~= grappleInstance.currentLineLength then
        Logger.debug("RopeInputController.handleAutoRetraction() - Clamped rope length from %.1f to %.1f", grappleInstance.currentLineLength, clampedLength)
    end
    grappleInstance.currentLineLength = clampedLength
end

-- Refresh gun reference - called when gun might have changed
function RopeInputController.refreshGunReference(grappleInstance)
    -- Only refresh if we don't have a valid reference
    if grappleInstance.parentGun then
        local success, presetName = pcall(function() return grappleInstance.parentGun.PresetName end)
        local idSuccess, gunID = pcall(function() return grappleInstance.parentGun.ID end)
        if success and presetName == "Grapple Gun" and idSuccess and gunID and gunID ~= rte.NoMOID then
            Logger.debug("RopeInputController.refreshGunReference() - Current gun reference is valid, no refresh needed")
            return true -- Current reference is fine
        end
    end
    
    Logger.debug("RopeInputController.refreshGunReference() - Refreshing gun reference")
    
    if not grappleInstance.parent then 
        Logger.debug("RopeInputController.refreshGunReference() - No parent")
        return false
    end
    
    local parent = grappleInstance.parent
    local foundGun = false
    
    -- Check equipped items first
    if parent.EquippedItem and parent.EquippedItem.PresetName == "Grapple Gun" then
        grappleInstance.parentGun = ToHDFirearm(parent.EquippedItem)
        foundGun = true
        Logger.info("RopeInputController.refreshGunReference() - Gun found equipped in main hand (ID: %d)", grappleInstance.parentGun.ID)
    elseif parent.EquippedBGItem and parent.EquippedBGItem.PresetName == "Grapple Gun" then
        grappleInstance.parentGun = ToHDFirearm(parent.EquippedBGItem)
        foundGun = true
        Logger.info("RopeInputController.refreshGunReference() - Gun found equipped in BG hand (ID: %d)", grappleInstance.parentGun.ID)
    end
    
    -- If not equipped, check inventory
    if not foundGun and parent.Inventory then
        for item in parent.Inventory do
            if item and item.PresetName == "Grapple Gun" then
                grappleInstance.parentGun = ToHDFirearm(item)
                foundGun = true
                Logger.info("RopeInputController.refreshGunReference() - Gun found in inventory (ID: %d)", grappleInstance.parentGun.ID)
                break
            end
        end
    end
    
    if foundGun and grappleInstance.parentGun then
        -- Test if we can actually access the gun's properties
        local testSuccess, testID = pcall(function() return grappleInstance.parentGun.ID end)
        if testSuccess and testID and testID ~= rte.NoMOID then
            -- Update magazine state for the refreshed gun
            local magSuccess, magazine = pcall(function() return grappleInstance.parentGun.Magazine end)
            if magSuccess and magazine and MovableMan:IsParticle(magazine) then
                local mag = ToMOSParticle(magazine)
                mag.RoundCount = 0 -- Keep showing as "fired"
                mag.Scale = 0 -- Keep hidden while grapple is active
                Logger.debug("RopeInputController.refreshGunReference() - Updated magazine state for refreshed gun")
            end
            return true
        else
            Logger.warn("RopeInputController.refreshGunReference() - Found gun but cannot access its properties")
            grappleInstance.parentGun = nil
            return false
        end
    end
    
    Logger.warn("RopeInputController.refreshGunReference() - Could not find any grapple gun")
    return false
end

-- Restore magazine state when grapple is being destroyed
function RopeInputController.restoreMagazineState(grappleInstance)
    if not grappleInstance.parentGun then
        Logger.debug("RopeInputController.restoreMagazineState() - No parent gun to restore")
        -- Try to find gun one more time for restoration
        if RopeInputController.refreshGunReference(grappleInstance) then
            Logger.debug("RopeInputController.restoreMagazineState() - Found gun during restoration attempt")
        else
            return false
        end
    end
    
    -- Don't call refreshGunReference again if we already have a gun reference
    -- Test the gun reference directly
    local success, gunID = pcall(function() return grappleInstance.parentGun.ID end)
    if success and gunID and gunID ~= rte.NoMOID then
        Logger.info("RopeInputController.restoreMagazineState() - Restoring magazine state for gun (ID: %d)", gunID)
        
        -- Restore magazine visibility and ammo count
        local magSuccess, magazine = pcall(function() return grappleInstance.parentGun.Magazine end)
        if magSuccess and magazine and MovableMan:IsParticle(magazine) then
            local mag = ToMOSParticle(magazine)
            mag.RoundCount = 1 -- Restore ammo
            mag.Scale = 1 -- Make magazine visible again
            Logger.info("RopeInputController.restoreMagazineState() - Magazine restored (visible, ammo: 1)")
            return true
        else
            Logger.warn("RopeInputController.restoreMagazineState() - No magazine found to restore")
        end
    else
        Logger.warn("RopeInputController.restoreMagazineState() - Gun ID invalid or inaccessible")
    end
    
    return false
end

return RopeInputController
