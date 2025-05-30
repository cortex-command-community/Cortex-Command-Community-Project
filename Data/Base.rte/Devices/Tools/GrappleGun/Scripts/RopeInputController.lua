-- Grapple Gun Input Controller Module
-- Handles all user input related to grapple rope control

local RopeInputController = {}

-- Handle direct rope length control with Shift+Mousewheel 
function RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    -- Only process shift+mousewheel when holding shift key (jump or crouch)
    local shiftHeld = controller:IsState(Controller.BODY_JUMPSTART) or controller:IsState(Controller.BODY_CROUCH)
    if not shiftHeld then return end
    
    local scrollAmount = 0
    
    if controller:IsState(Controller.SCROLL_UP) then
        -- Scroll up - shorten rope
        scrollAmount = -grappleInstance.shiftScrollSpeed
    elseif controller:IsState(Controller.SCROLL_DOWN) then
        -- Scroll down - lengthen rope
        scrollAmount = grappleInstance.shiftScrollSpeed
    end
    
    if scrollAmount ~= 0 then
        -- Apply length change
        local newLength = grappleInstance.currentLineLength + scrollAmount
        -- Clamp to valid range
        newLength = math.max(10, math.min(newLength, grappleInstance.maxLineLength)) 
        
        -- Update rope length
        grappleInstance.currentLineLength = newLength
        grappleInstance.setLineLength = newLength
    end
end

-- Handle R key (reload) press to unhook grapple
function RopeInputController.handleReloadKeyUnhook(grappleInstance, controller)
    if not controller then return false end
    
    -- Check for reload key press (R key)
    if controller:IsState(Controller.WEAPON_RELOAD) then
        -- Only unhook if holding the Grapple Gun
        if grappleInstance.parent.EquippedItem and 
           grappleInstance.parentGun and 
           grappleInstance.parent.EquippedItem.ID == grappleInstance.parentGun.ID then
            return true -- Signal to delete the hook
        end
    end
    
    return false
end

-- Handle double-tap detection for retrieving grapple
function RopeInputController.handleTapDetection(grappleInstance, controller)
    if not controller then return false end

    local proneState = controller:IsState(Controller.BODY_PRONE)
    local isHoldingGrappleGun = false
    
    -- Check if player is holding grapple gun
    if grappleInstance.parent and grappleInstance.parent.EquippedItem and 
       grappleInstance.parentGun and grappleInstance.parent.EquippedItem.ID == grappleInstance.parentGun.ID then
        isHoldingGrappleGun = true
    end
    
    local shouldUnhook = false
    
    -- Handle tap state changes
    if proneState then
        if not isHoldingGrappleGun then -- Only process tap for unhook if NOT holding grapple gun
            grappleInstance.pieSelection = 0
            if grappleInstance.canTap then
                controller:SetState(Controller.BODY_PRONE, false)
                grappleInstance.climb = 0
                
                if grappleInstance.parentGun and grappleInstance.parentGun.ID ~= rte.NoMOID then
                    grappleInstance.parentGun:RemoveNumberValue("GrappleMode")
                end
                
                grappleInstance.tapTimer:Reset()
                grappleInstance.didTap = true
                grappleInstance.canTap = false
                grappleInstance.tapCounter = grappleInstance.tapCounter + 1
            end
        else
            grappleInstance.canTap = true
        end
    else
        grappleInstance.canTap = true
    end
    
    -- Check if we've reached enough taps in time to unhook
    if grappleInstance.tapTimer:IsPastSimMS(grappleInstance.tapTime) then
        grappleInstance.tapCounter = 0
    else
        if grappleInstance.tapCounter >= grappleInstance.tapAmount then
            if not isHoldingGrappleGun then -- Only unhook via double tap if NOT holding grapple gun
                shouldUnhook = true
            else
                grappleInstance.tapCounter = 0 -- If holding gun, reset counter to prevent unhook
            end
        end
    end
    
    return shouldUnhook
end

-- Handle mouse wheel scrolling for rope length control
function RopeInputController.handleMouseWheelControl(grappleInstance, controller)
    if not controller:IsMouseControlled() then return end
    
    controller:SetState(Controller.WEAPON_CHANGE_NEXT, false)
    controller:SetState(Controller.WEAPON_CHANGE_PREV, false)
    
    -- Handle Shift+Mousewheel for rope control
    if controller:IsState(Controller.BODY_JUMPSTART) or controller:IsState(Controller.BODY_CROUCH) then
        -- Call our enhanced Shift+Mousewheel handler function
        RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    else
        -- Normal mousewheel behavior (without shift)
        if controller:IsState(Controller.SCROLL_UP) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.climb = 3
        end
        
        if controller:IsState(Controller.SCROLL_DOWN) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.climb = 4
        end
    end
end

-- Process standard directional controls for climbing
function RopeInputController.handleDirectionalControl(grappleInstance, controller, terrCheck)
    if not controller then return end
    
    if controller:IsMouseControlled() == false then
        if controller:IsState(Controller.HOLD_UP) then
            if grappleInstance.currentLineLength > grappleInstance.climbInterval and terrCheck == false then
                grappleInstance.climb = 1
            elseif terrCheck ~= false then
                -- Try to nudge past terrain
                local nudge = math.sqrt(grappleInstance.lineLength + grappleInstance.parent.Radius) / 
                             (10 + grappleInstance.parent.Vel.Magnitude)
                local aimvec = Vector(grappleInstance.lineVec.Magnitude, 0)
                               :SetMagnitude(nudge)
                               :RadRotate((grappleInstance.lineVec.AbsRadAngle + 
                                         grappleInstance.parent:GetAimAngle(true))/2 + 
                                         grappleInstance.parent.FlipFactor * 0.7)
                grappleInstance.parent.Vel = grappleInstance.parent.Vel + aimvec
            end
        end

        if controller:IsState(Controller.HOLD_DOWN) and 
           grappleInstance.currentLineLength < (grappleInstance.maxLineLength-grappleInstance.climbInterval) then
            grappleInstance.climb = 2
        end
    end
    
    controller:SetState(Controller.AIM_UP, false)
    controller:SetState(Controller.AIM_DOWN, false)
end

-- Handle rope pulling actions from gun activation
function RopeInputController.handleRopePulling(grappleInstance)
    local controller = grappleInstance.parent:GetController()
    local parentForces = 1 + (grappleInstance.parent.Vel.Magnitude * 10 + 
                             grappleInstance.parent.Mass)/(1 + grappleInstance.lineLength)
                             
    -- Check for terrain between player and hook to avoid auto-pulling through walls
    local terrCheck = false
    if grappleInstance.parentRadius ~= nil then
        local terrVector = Vector()
        terrCheck = SceneMan:CastStrengthRay(grappleInstance.parent.Pos, 
                                            grappleInstance.lineVec:SetMagnitude(grappleInstance.parentRadius), 
                                            0, terrVector, 2, rte.airID, grappleInstance.mapWrapsX)
    end
    
    -- Handle climbing timer for manual rope control
    if grappleInstance.climb ~= 0 and 
       grappleInstance.pieSelection == 0 and 
       grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
        
        grappleInstance.climbTimer:Reset()
        
        -- Process up/down movement
        if grappleInstance.climb == 1 then
            -- Retract - pull player up
            grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.climbInterval/parentForces)
            grappleInstance.setLineLength = grappleInstance.currentLineLength
        elseif grappleInstance.climb == 2 then
            -- Extend - let player down
            grappleInstance.currentLineLength = grappleInstance.currentLineLength + grappleInstance.climbInterval
            grappleInstance.setLineLength = grappleInstance.currentLineLength
        end
        
        -- Reset climb state
        grappleInstance.climb = 0
    end
    
    -- Handle mouse-based climbing
    if (grappleInstance.climb == 3 or grappleInstance.climb == 4) then
        if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.mouseClimbLength) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.mouseClimbTimer:Reset()
            grappleInstance.climb = 0
        else
            -- Handle mouse wheel based climbing
            if grappleInstance.mouseClimbTimer:IsPastSimMS(grappleInstance.climbDelay) then
                grappleInstance.mouseClimbTimer:Reset()
                
                if grappleInstance.climb == 3 and grappleInstance.currentLineLength > grappleInstance.climbInterval then
                    -- Mouse wheel up - retract rope
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.climbInterval/parentForces)
                    grappleInstance.setLineLength = grappleInstance.currentLineLength
                elseif grappleInstance.climb == 4 and grappleInstance.currentLineLength < (grappleInstance.maxLineLength-grappleInstance.climbInterval) then
                    -- Mouse wheel down - extend rope
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength + grappleInstance.climbInterval
                    grappleInstance.setLineLength = grappleInstance.currentLineLength
                end
            end
        end
    end
    
    -- Process directional controls
    RopeInputController.handleDirectionalControl(grappleInstance, controller, terrCheck)
    
    -- Process mouse wheel controls
    RopeInputController.handleMouseWheelControl(grappleInstance, controller)
    
    return terrCheck
end

-- Process pie menu selections
function RopeInputController.handlePieMenuSelection(grappleInstance)
    if not grappleInstance.parentGun then return end
    
    local mode = grappleInstance.parentGun:GetNumberValue("GrappleMode")
    
    if mode ~= 0 then
        if mode == 3 then -- Unhook via Pie Menu
            grappleInstance.parentGun:RemoveNumberValue("GrappleMode")
            return true -- Signal to delete the hook
        else
            grappleInstance.pieSelection = mode
            grappleInstance.parentGun:RemoveNumberValue("GrappleMode")
        end
    end
    
    return false
end

-- Handle auto retraction via held fire button
function RopeInputController.handleAutoRetraction(grappleInstance, terrCheck)
    if not grappleInstance.parentGun then return end
    
    local parentForces = 1 + (grappleInstance.parent.Vel.Magnitude * 10 + 
                             grappleInstance.parent.Mass)/(1 + grappleInstance.lineLength)
    
    -- Retract automatically by holding fire or control the rope through the pie menu
    if grappleInstance.parentGun:IsActivated() and grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
        grappleInstance.climbTimer:Reset()
        
        if grappleInstance.pieSelection == 0 and grappleInstance.parentGun:IsActivated() then
            if grappleInstance.currentLineLength > grappleInstance.autoClimbIntervalA and terrCheck == false then
                grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.autoClimbIntervalA/parentForces)
                grappleInstance.setLineLength = grappleInstance.currentLineLength
            else
                grappleInstance.parentGun:RemoveNumberValue("GrappleMode")
                grappleInstance.pieSelection = 0
                
                if terrCheck ~= false then
                    -- Try to nudge past terrain
                    local nudge = math.sqrt(grappleInstance.lineLength + grappleInstance.parent.Radius) / 
                                 (10 + grappleInstance.parent.Vel.Magnitude)
                    local aimvec = Vector(grappleInstance.lineVec.Magnitude, 0)
                                  :SetMagnitude(nudge)
                                  :RadRotate((grappleInstance.lineVec.AbsRadAngle + 
                                            grappleInstance.parent:GetAimAngle(true))/2 + 
                                            grappleInstance.parent.FlipFactor * 0.7)
                    grappleInstance.parent.Vel = grappleInstance.parent.Vel + aimvec
                end
            end
        end
    end
    
    -- Process programmatic rope control through pie menu selection
    if grappleInstance.pieSelection ~= 0 and grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
        grappleInstance.climbTimer:Reset()
        
        if grappleInstance.pieSelection == 1 then
            -- Full retract
            if grappleInstance.currentLineLength > grappleInstance.autoClimbIntervalA and terrCheck == false then
                grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.autoClimbIntervalA/parentForces)
                grappleInstance.setLineLength = grappleInstance.currentLineLength
            else
                grappleInstance.pieSelection = 0
            end
        elseif grappleInstance.pieSelection == 2 then
            -- Partial extend
            if grappleInstance.currentLineLength < (grappleInstance.maxLineLength - grappleInstance.autoClimbIntervalB) then
                grappleInstance.currentLineLength = grappleInstance.currentLineLength + grappleInstance.autoClimbIntervalB
                grappleInstance.setLineLength = grappleInstance.currentLineLength
            else
                grappleInstance.parentGun:RemoveNumberValue("GrappleMode")
                grappleInstance.pieSelection = 0
            end
        end
    end
end

return RopeInputController
