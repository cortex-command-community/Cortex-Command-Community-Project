-- Grapple Gun Input Controller Module
-- Handles user input for rope control.
-- Translates raw input into actions for the main grapple logic.

local RopeInputController = {}

-- Helper to check if the player is currently holding the specific grapple gun instance.
local function isHoldingGrappleGun(grappleInstance)
    if grappleInstance and grappleInstance.parent and grappleInstance.parentGun then
        if grappleInstance.parent.EquippedItem and grappleInstance.parent.EquippedItem.ID == grappleInstance.parentGun.ID then
            return true
        end
        if grappleInstance.parent.EquippedBGItem and grappleInstance.parent.EquippedBGItem.ID == grappleInstance.parentGun.ID then
            return true
        end
    end
    return false
end

-- Handle direct rope length control with Shift+Mousewheel.
function RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    if not controller then return end

    -- Only process if Shift (Jump or Crouch in this context) is held.
    local shiftHeld = controller:IsState(Controller.BODY_JUMPSTART)
    if not shiftHeld then return end
    
    local scrollAmount = 0
    if controller:IsState(Controller.SCROLL_UP) then
        scrollAmount = -grappleInstance.shiftScrollSpeed
    elseif controller:IsState(Controller.SCROLL_DOWN) then
        scrollAmount = grappleInstance.shiftScrollSpeed
    end
    
    if scrollAmount ~= 0 then
        local newLength = grappleInstance.currentLineLength + scrollAmount
        -- Clamp to valid range (e.g., min 10, max defined by maxLineLength).
        newLength = math.max(10, math.min(newLength, grappleInstance.maxLineLength)) 
        
        grappleInstance.currentLineLength = newLength
        grappleInstance.setLineLength = newLength -- Ensure setLineLength is also updated.
        grappleInstance.climbTimer:Reset() -- Reset climb timer to reflect manual adjustment.
    end
end

-- Handle R key (reload) press to unhook the grapple.
function RopeInputController.handleReloadKeyUnhook(grappleInstance, controller)
    if not controller then return false end
    
    if controller:IsState(Controller.WEAPON_RELOAD) then
        -- Only unhook if the player is actually holding this grapple gun.
        if isHoldingGrappleGun(grappleInstance) then
            return true -- Signal to Grapple.lua to delete the hook.
        end
    end
    return false
end


-- Handle double-tap detection (e.g., crouch key) for retrieving the grapple.
-- This is typically used when *not* holding the grapple gun.
function RopeInputController.handleTapDetection(grappleInstance, controller)
    if not controller or not grappleInstance.parent then return false end

    local proneState = controller:IsState(Controller.BODY_PRONE)
    
    -- This tap detection is for recalling the hook when *NOT* holding the gun.
    if isHoldingGrappleGun(grappleInstance) then
        grappleInstance.tapCounter = 0 -- Reset tap if player is holding the gun.
        grappleInstance.canTap = true -- Allow tapping if they switch away.
        return false
    end
        
    if proneState then
        if grappleInstance.canTap then
            controller:SetState(Controller.BODY_PRONE, false) -- Prevent continuous prone state.
            
            -- Reset pie selection and climb state if a tap occurs.
            grappleInstance.pieSelection = 0
            grappleInstance.climb = 0
            if grappleInstance.parentGun and grappleInstance.parentGun.ID ~= rte.NoMOID then
                grappleInstance.parentGun:RemoveNumberValue("GrappleMode")
            end
            
            grappleInstance.tapTimer:Reset()
            -- grappleInstance.didTap = true -- If used for anything.
            grappleInstance.canTap = false -- Prevent immediate re-tap.
            grappleInstance.tapCounter = grappleInstance.tapCounter + 1
        end
    else
        grappleInstance.canTap = true -- Ready for the first tap.
    end
    
    if grappleInstance.tapTimer:IsPastSimMS(grappleInstance.tapTime) then
        grappleInstance.tapCounter = 0 -- Reset if too much time passed.
    else
        if grappleInstance.tapCounter >= grappleInstance.tapAmount then
            grappleInstance.tapCounter = 0 -- Reset after successful multi-tap.
            return true -- Signal to Grapple.lua to delete the hook.
        end
    end
    return false
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

    if grappleInstance.actionMode <= 1 then -- Not attached, or flying. No pulling.
        grappleInstance.climb = 0
        return
    end

    local parentForces = 1.0
    if grappleInstance.parent.Vel and grappleInstance.parent.Mass and grappleInstance.lineLength > 0 then
         parentForces = 1 + (grappleInstance.parent.Vel.Magnitude * 10 + grappleInstance.parent.Mass) / (1 + grappleInstance.lineLength)
         parentForces = math.max(0.1, parentForces) 
    end
                             
    if grappleInstance.climb ~= 0 and grappleInstance.pieSelection == 0 then 
        if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
            grappleInstance.climbTimer:Reset()
            
            if grappleInstance.climb == 1 then 
                grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.climbInterval / parentForces)
            elseif grappleInstance.climb == 2 then 
                grappleInstance.currentLineLength = grappleInstance.currentLineLength + grappleInstance.climbInterval 
            end
            grappleInstance.setLineLength = grappleInstance.currentLineLength
            grappleInstance.climb = 0 
        end
        
        if grappleInstance.climb == 3 or grappleInstance.climb == 4 then 
            if grappleInstance.mouseClimbTimer:IsPastSimMS(grappleInstance.climbDelay) then 
                grappleInstance.mouseClimbTimer:Reset()
                if grappleInstance.climb == 3 and grappleInstance.currentLineLength > grappleInstance.climbInterval then
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.climbInterval / parentForces)
                elseif grappleInstance.climb == 4 and grappleInstance.currentLineLength < (grappleInstance.maxLineLength - grappleInstance.climbInterval) then
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength + grappleInstance.climbInterval
                end
                grappleInstance.setLineLength = grappleInstance.currentLineLength
            end
            if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.mouseClimbLength) then
                 grappleInstance.climb = 0 
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
