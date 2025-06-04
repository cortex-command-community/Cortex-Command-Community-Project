-- Grapple Gun Input Controller Module
-- Handles user input for rope control.
-- Translates raw input into actions for the main grapple logic.

local RopeInputController = {}

-- Helper function to check if the player is holding the grapple gun
local function isHoldingGrappleGun(grappleInstance)
    if not grappleInstance.parent or not grappleInstance.parentGun then return false end
    
    -- Check if holding in main hand
    if grappleInstance.parent.EquippedItem and grappleInstance.parent.EquippedItem.ID == grappleInstance.parentGun.ID then
        return true
    end
    
    -- Check if holding in background hand
    if grappleInstance.parent.EquippedBGItem and grappleInstance.parent.EquippedBGItem.ID == grappleInstance.parentGun.ID then
        return true
    end
    
    -- NEW: Also check if the gun exists in inventory (allowing control even when not equipped)
    if grappleInstance.parent.Inventory then
        for item in grappleInstance.parent.Inventory do
            if item and item.ID == grappleInstance.parentGun.ID then
                return true
            end
        end
    end
    
    return false
end

-- Handle direct rope length control with Shift+Mousewheel.
function RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    if not controller then return end

    -- Only process if Shift (Jump or Crouch in this context) is held.
    local shiftHeld = controller:IsState(Controller.BODY_JUMPSTART) or controller:IsState(Controller.BODY_CROUCH)
    if not shiftHeld then return end
    
    -- Only allow when attached (not flying)
    if grappleInstance.actionMode <= 1 then return end
    
    local scrollAmount = 0
    local scrollDetected = false
    
    -- Use even smaller increment for ultra-precise control
    local preciseScrollSpeed = (grappleInstance.shiftScrollSpeed or 1.0) * 0.25 -- Quarter speed for ultra precision
    
    if controller:IsState(Controller.SCROLL_UP) then
        scrollAmount = -preciseScrollSpeed -- Negative for retraction
        scrollDetected = true
        print("Shift+Scroll UP detected, retracting by: " .. preciseScrollSpeed) -- Debug
    elseif controller:IsState(Controller.SCROLL_DOWN) then
        scrollAmount = preciseScrollSpeed -- Positive for extension
        scrollDetected = true
        print("Shift+Scroll DOWN detected, extending by: " .. preciseScrollSpeed) -- Debug
    end
    
    -- Only apply changes while actively scrolling (no automatic behavior)
    if scrollDetected and scrollAmount ~= 0 then
        local oldLength = grappleInstance.currentLineLength
        local newLength = grappleInstance.currentLineLength + scrollAmount
        -- Clamp to valid range (e.g., min 10, max defined by maxLineLength).
        newLength = math.max(10, math.min(newLength, grappleInstance.maxLineLength)) 
        
        grappleInstance.currentLineLength = newLength
        grappleInstance.setLineLength = newLength -- Ensure setLineLength is also updated.
        grappleInstance.climbTimer:Reset() -- Reset climb timer to reflect manual adjustment.
        
        -- Clear any automatic pie menu selections when manually controlling
        grappleInstance.pieSelection = 0
        grappleInstance.climb = 0
        
        print("Rope length changed from " .. oldLength .. " to " .. newLength) -- Debug
    end
end

-- Handle R key (reload) press to unhook the grapple.
function RopeInputController.handleReloadKeyUnhook(grappleInstance, controller)
    if not controller then return false end
    
    local isCurrentlyHolding = false
    if grappleInstance.parent.EquippedItem and grappleInstance.parent.EquippedItem.ID == grappleInstance.parentGun.ID then
        isCurrentlyHolding = true
    elseif grappleInstance.parent.EquippedBGItem and grappleInstance.parent.EquippedBGItem.ID == grappleInstance.parentGun.ID then
        isCurrentlyHolding = true
    end
    
    -- If currently holding the gun, use R key to unhook
    if isCurrentlyHolding and controller:IsState(Controller.WEAPON_RELOAD) then
        print("R key pressed while holding grapple gun - unhooking!") -- Debug
        return true -- Signal unhook
    end
    
    return false
end


-- Handle double-tap detection (e.g., crouch key) for retrieving the grapple.
-- This is typically used when *not* holding the grapple gun.
function RopeInputController.handleTapDetection(grappleInstance, controller)
    if not controller or not grappleInstance.parent then return false end

    -- This tap detection is for recalling the hook when *NOT* holding the gun.
    local isHolding = isHoldingGrappleGun(grappleInstance)
    if isHolding then
        -- IS holding gun - don't process crouch-tap for unhook, reset counters
        grappleInstance.tapCounter = 0
        grappleInstance.canTap = true
        return false
    end

    -- NOT holding gun - process crouch-tap for unhook
    local proneState = controller:IsState(Controller.BODY_PRONE)
    
    if proneState then
        if grappleInstance.canTap then
            -- Clear the prone state immediately to prevent interference
            controller:SetState(Controller.BODY_PRONE, false)
            
            grappleInstance.tapCounter = grappleInstance.tapCounter + 1
            grappleInstance.canTap = false
            grappleInstance.tapTimer:Reset()
            
            print("Crouch tap " .. grappleInstance.tapCounter .. " detected (not holding gun)") -- Debug
        end
    else
        grappleInstance.canTap = true -- Ready for the next tap when crouch is released.
    end
    
    -- Check if enough taps occurred within time limit
    if grappleInstance.tapTimer:IsPastSimMS(grappleInstance.tapTime) then
        grappleInstance.tapCounter = 0 -- Reset if too much time passed
    else
        if grappleInstance.tapCounter >= grappleInstance.tapAmount then
            grappleInstance.tapCounter = 0
            print("Double crouch-tap while NOT holding gun - unhooking!") -- Debug
            return true -- Signal unhook
        end
    end
    
    return false
end

-- Add a new function for handling crouch controls when holding the gun
function RopeInputController.handleCrouchControls(grappleInstance, controller)
    if not controller or not grappleInstance.parent then return end
    
    -- Only process if holding the grapple gun
    local isHolding = isHoldingGrappleGun(grappleInstance)
    if not isHolding then return end
    
    -- When holding gun, crouch can be used for rope control
    -- This ensures crouch works normally for rope length control
    -- without interfering with unhook tap detection
    
    -- Reset tap counters when holding gun to prevent accidental unhooks
    grappleInstance.tapCounter = 0
    grappleInstance.canTap = true
end

-- Handle mouse wheel scrolling for rope length control (when not holding Shift).
function RopeInputController.handleMouseWheelControl(grappleInstance, controller)
    if not controller or not controller:IsMouseControlled() then return end
    
    -- Clear weapon change states if mouse wheel is used for grapple control.
    controller:SetState(Controller.WEAPON_CHANGE_NEXT, false)
    controller:SetState(Controller.WEAPON_CHANGE_PREV, false)
    
    -- If Shift is held, it's handled by handleShiftMousewheelControls.
    local shiftHeld = controller:IsState(Controller.BODY_JUMPSTART) or controller:IsState(Controller.BODY_CROUCH)
    if shiftHeld then
        RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    else
        -- Normal mousewheel behavior (without Shift) for quick retract/extend.
        if controller:IsState(Controller.SCROLL_UP) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.climb = 3 -- Signal mouse retract.
        elseif controller:IsState(Controller.SCROLL_DOWN) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.climb = 4 -- Signal mouse extend.
        end
    end
end

-- Process standard directional controls (Up/Down keys) for climbing.
function RopeInputController.handleDirectionalControl(grappleInstance, controller)
    if not controller or controller:IsMouseControlled() then return end -- Only for keyboard/gamepad.
    if grappleInstance.actionMode <= 1 then return end -- Only allow climbing when attached
    
    if grappleInstance.actionMode <= 1 then -- Not attached, or flying. No pulling.
        grappleInstance.climb = 0
        return
    end
    -- Using HOLD_UP/HOLD_DOWN for continuous climbing.
    if controller:IsState(Controller.HOLD_UP) then
        if grappleInstance.currentLineLength > grappleInstance.climbInterval then -- Check if can retract further.
            grappleInstance.climb = 1 -- Signal key retract.
        end
    elseif controller:IsState(Controller.HOLD_DOWN) then -- Use elseif to prevent retract & extend same frame.
        if grappleInstance.currentLineLength < (grappleInstance.maxLineLength - grappleInstance.climbInterval) then -- Check if can extend further.
            grappleInstance.climb = 2 -- Signal key extend.
        end
    end
    
    -- Clear aim states if directional keys are used for climbing.
    controller:SetState(Controller.AIM_UP, false)
    controller:SetState(Controller.AIM_DOWN, false)
end

-- Main function to handle all rope pulling/climbing inputs.
function RopeInputController.handleRopePulling(grappleInstance)
    if not grappleInstance.parent then return end
    
    local controller = grappleInstance.parent:GetController()
    if not controller then return end
    
    local oldLength = grappleInstance.setLineLength
    local lengthChanged = false
    
    -- Handle directional controls for rope length
    if controller:IsState(Controller.MOVE_UP) then
        grappleInstance.setLineLength = math.max(grappleInstance.setLineLength - grappleInstance.climbInterval, 50)
        lengthChanged = true
    elseif controller:IsState(Controller.MOVE_DOWN) then
        grappleInstance.setLineLength = math.min(grappleInstance.setLineLength + grappleInstance.climbInterval, grappleInstance.maxLineLength)
        lengthChanged = true
    end
    
    -- Handle shift+mousewheel controls
    RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    
    -- Play sound if length changed significantly
    if lengthChanged and math.abs(grappleInstance.setLineLength - oldLength) > 5 then
        if grappleInstance.setLineLength < oldLength then
            -- Retracting - play crank sound
            if grappleInstance.crankSoundInstance and not grappleInstance.crankSoundInstance.ToDelete then
                grappleInstance.crankSoundInstance.ToDelete = true
            end
            grappleInstance.crankSoundInstance = CreateSoundContainer("Grapple Gun Crank", "Base.rte")
            if grappleInstance.crankSoundInstance then
                grappleInstance.crankSoundInstance:Play(grappleInstance.parent.Pos)
            end
        else
            -- Extending - play click sound
            if grappleInstance.clickSound then
                grappleInstance.clickSound:Play(grappleInstance.parent.Pos)
            end
        end
    end
    
    grappleInstance.currentLineLength = math.max(10, math.min(grappleInstance.currentLineLength, grappleInstance.maxLineLength))
    
    RopeInputController.handleDirectionalControl(grappleInstance, controller)
    RopeInputController.handleMouseWheelControl(grappleInstance, controller)
end

-- Process pie menu selections made by the player.
-- Process pie menu selections made by the player.
function RopeInputController.handlePieMenuSelection(grappleInstance)
    if not grappleInstance.parentGun or grappleInstance.parentGun.ID == rte.NoMOID then return false end
    
    local mode = grappleInstance.parentGun:GetNumberValue("GrappleMode") 
    
    if mode and mode ~= 0 then
        grappleInstance.parentGun:RemoveNumberValue("GrappleMode") 
        if mode == 3 then -- Unhook via Pie Menu.
            return true 
        else
            if grappleInstance.actionMode > 1 then -- Only allow pie retract/extend if attached
                grappleInstance.pieSelection = mode 
                grappleInstance.climb = 0 
            end
        end
    end
    return false 
end

-- Handle automatic retraction (e.g., when holding fire button or from pie menu).
function RopeInputController.handleAutoRetraction(grappleInstance, terrCheck)
    if not grappleInstance.parentGun or grappleInstance.parentGun.ID == rte.NoMOID then return end
    if grappleInstance.actionMode <= 1 then -- No auto retraction if not attached.
        grappleInstance.pieSelection = 0
        return
    end
    
    local parentForces = 1.0
    if grappleInstance.parent and grappleInstance.parent.Vel and grappleInstance.parent.Mass and grappleInstance.lineLength > 0 then
         parentForces = 1 + (grappleInstance.parent.Vel.Magnitude * 10 + grappleInstance.parent.Mass) / (1 + grappleInstance.lineLength)
         parentForces = math.max(0.1, parentForces)
    end
    
    if grappleInstance.parentGun:IsActivated() and grappleInstance.pieSelection == 0 then
        if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then 
            grappleInstance.climbTimer:Reset()
            if grappleInstance.currentLineLength > grappleInstance.autoClimbIntervalA then
                grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.autoClimbIntervalA / parentForces)
                grappleInstance.setLineLength = grappleInstance.currentLineLength
            end
        end
    end
    
    if grappleInstance.pieSelection ~= 0 then
        if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
            grappleInstance.climbTimer:Reset()
            local actionTaken = false
            if grappleInstance.pieSelection == 1 then 
                if grappleInstance.currentLineLength > grappleInstance.autoClimbIntervalA then
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.autoClimbIntervalA / parentForces)
                    actionTaken = true
                end
            elseif grappleInstance.pieSelection == 2 then 
                if grappleInstance.currentLineLength < (grappleInstance.maxLineLength - grappleInstance.autoClimbIntervalB) then
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength + grappleInstance.autoClimbIntervalB
                    actionTaken = true
                end
            end
            
            grappleInstance.setLineLength = grappleInstance.currentLineLength
            if not actionTaken then
                grappleInstance.pieSelection = 0 
            end
        end
    end
    grappleInstance.currentLineLength = math.max(10, math.min(grappleInstance.currentLineLength, grappleInstance.maxLineLength))
end

return RopeInputController
