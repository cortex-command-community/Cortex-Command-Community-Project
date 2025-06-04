-- Grapple Gun Input Controller Module
-- Handles user input for rope control.
-- Translates raw input into actions for the main grapple logic.

local RopeInputController = {}

-- Check if player is currently holding the grapple gun (equipped in main or background hand)
local function isCurrentlyEquipped(grappleInstance)
    if not grappleInstance.parent or not grappleInstance.parentGun then return false end
    
    local parent = grappleInstance.parent
    return (parent.EquippedItem and parent.EquippedItem.ID == grappleInstance.parentGun.ID) or
           (parent.EquippedBGItem and parent.EquippedBGItem.ID == grappleInstance.parentGun.ID)
end

-- Check if gun exists in player's inventory
local function isInInventory(grappleInstance)
    if not grappleInstance.parent or not grappleInstance.parentGun or not grappleInstance.parent.Inventory then 
        return false 
    end
    
    for item in grappleInstance.parent.Inventory do
        if item and item.ID == grappleInstance.parentGun.ID then
            return true
        end
    end
    return false
end

-- Handle gun persistence - ensure grapple stays active even when gun changes hands/inventory
function RopeInputController.handleGunPersistence(grappleInstance)
    if not grappleInstance.parent or not grappleInstance.parentGun then return false end
    
    -- Check if gun still exists in any form (equipped or in inventory)
    local gunStillExists = isCurrentlyEquipped(grappleInstance) or isInInventory(grappleInstance)
    
    if not gunStillExists then
        -- Gun was completely removed from player (dropped, etc.)
        print("Grapple gun removed from player - maintaining hook but no new controls")
        return false -- This will eventually lead to unhook when hook hits terrain
    end
    
    -- Gun still exists somewhere - keep grapple active
    -- Update magazine state regardless of where gun is
    if grappleInstance.parentGun.Magazine and MovableMan:IsParticle(grappleInstance.parentGun.Magazine) then
        local mag = ToMOSParticle(grappleInstance.parentGun.Magazine)
        mag.RoundCount = 0 -- Keep showing as "fired"
        mag.Scale = 0 -- Keep hidden while grapple is active
    end
    
    return true
end

-- Handle R key unhooking (only when gun is equipped)
function RopeInputController.handleReloadKeyUnhook(grappleInstance, controller)
    if not controller then return false end
    
    if isCurrentlyEquipped(grappleInstance) and controller:IsState(Controller.WEAPON_RELOAD) then
        print("R key pressed while holding grapple gun - unhooking!")
        return true
    end
    
    return false
end

-- Handle double-tap crouch unhooking (only when gun is NOT equipped but in inventory)
function RopeInputController.handleTapDetection(grappleInstance, controller)
    if not controller or not grappleInstance.parent then return false end

    -- Only allow tap unhooking when gun is NOT equipped but IS in inventory
    if isCurrentlyEquipped(grappleInstance) then
        -- Reset tap state when gun is equipped
        grappleInstance.tapCounter = 0
        grappleInstance.canTap = true
        return false
    end
    
    if not isInInventory(grappleInstance) then
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
            
            print("Crouch tap " .. grappleInstance.tapCounter .. " detected (gun not equipped)")
        end
    else
        grappleInstance.canTap = true
    end
    
    -- Check for successful double-tap
    if grappleInstance.tapTimer:IsPastSimMS(grappleInstance.tapTime) then
        grappleInstance.tapCounter = 0 -- Reset if too much time passed
    elseif grappleInstance.tapCounter >= grappleInstance.tapAmount then
        grappleInstance.tapCounter = 0
        print("Double crouch-tap while gun not equipped - unhooking!")
        return true
    end
    
    return false
end

-- Handle precise rope control with Shift+Mousewheel
function RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    if not controller or grappleInstance.actionMode <= 1 then return end

    -- Only allow rope controls if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then return end

    local shiftHeld = controller:IsState(Controller.BODY_JUMPSTART) or controller:IsState(Controller.BODY_CROUCH)
    if not shiftHeld then return end
    
    local scrollAmount = 0
    local preciseScrollSpeed = (grappleInstance.shiftScrollSpeed or 1.0) * 0.25
    
    if controller:IsState(Controller.SCROLL_UP) then
        scrollAmount = -preciseScrollSpeed
    elseif controller:IsState(Controller.SCROLL_DOWN) then
        scrollAmount = preciseScrollSpeed
    end
    
    if scrollAmount ~= 0 then
        local newLength = math.max(10, math.min(
            grappleInstance.currentLineLength + scrollAmount, 
            grappleInstance.maxLineLength
        ))
        
        grappleInstance.currentLineLength = newLength
        grappleInstance.setLineLength = newLength
        grappleInstance.climbTimer:Reset()
        
        -- Clear automatic selections
        grappleInstance.pieSelection = 0
        grappleInstance.climb = 0
    end
end

-- Handle mouse wheel scrolling for rope control
function RopeInputController.handleMouseWheelControl(grappleInstance, controller)
    if not controller or not controller:IsMouseControlled() then return end
    
    -- Only allow rope controls if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then return end
    
    -- Clear weapon change states
    controller:SetState(Controller.WEAPON_CHANGE_NEXT, false)
    controller:SetState(Controller.WEAPON_CHANGE_PREV, false)
    
    -- Check if shift is held for precise control
    local shiftHeld = controller:IsState(Controller.BODY_JUMPSTART) or controller:IsState(Controller.BODY_CROUCH)
    if shiftHeld then
        RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    else
        -- Normal mousewheel behavior
        if controller:IsState(Controller.SCROLL_UP) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.climb = 3 -- Mouse retract
        elseif controller:IsState(Controller.SCROLL_DOWN) then
            grappleInstance.climbTimer:Reset()
            grappleInstance.climb = 4 -- Mouse extend
        end
    end
end

-- Handle directional key controls for climbing
function RopeInputController.handleDirectionalControl(grappleInstance, controller)
    if not controller or controller:IsMouseControlled() or grappleInstance.actionMode <= 1 then 
        return 
    end
    
    -- Only allow rope controls if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then return end
    
    if controller:IsState(Controller.HOLD_UP) then
        if grappleInstance.currentLineLength > grappleInstance.climbInterval then
            grappleInstance.climb = 1 -- Key retract
        end
    elseif controller:IsState(Controller.HOLD_DOWN) then
        if grappleInstance.currentLineLength < (grappleInstance.maxLineLength - grappleInstance.climbInterval) then
            grappleInstance.climb = 2 -- Key extend
        end
    end
    
    -- Clear aim states if directional keys are used
    controller:SetState(Controller.AIM_UP, false)
    controller:SetState(Controller.AIM_DOWN, false)
end

-- Main rope pulling handler
function RopeInputController.handleRopePulling(grappleInstance)
    if not grappleInstance.parent then return end
    
    local controller = grappleInstance.parent:GetController()
    if not controller then return end
    
    -- Only allow active rope control if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then return end
    
    local oldLength = grappleInstance.setLineLength
    local lengthChanged = false
    
    -- Handle directional controls
    if controller:IsState(Controller.MOVE_UP) then
        grappleInstance.setLineLength = math.max(grappleInstance.setLineLength - grappleInstance.climbInterval, 50)
        lengthChanged = true
    elseif controller:IsState(Controller.MOVE_DOWN) then
        grappleInstance.setLineLength = math.min(grappleInstance.setLineLength + grappleInstance.climbInterval, grappleInstance.maxLineLength)
        lengthChanged = true
    end
    
    -- Handle shift+mousewheel
    RopeInputController.handleShiftMousewheelControls(grappleInstance, controller)
    
    -- Play sounds for length changes
    if lengthChanged and math.abs(grappleInstance.setLineLength - oldLength) > 5 then
        if grappleInstance.setLineLength < oldLength then
            -- Retracting sound
            if grappleInstance.crankSoundInstance and not grappleInstance.crankSoundInstance.ToDelete then
                grappleInstance.crankSoundInstance.ToDelete = true
            end
            grappleInstance.crankSoundInstance = CreateSoundContainer("Grapple Gun Crank", "Base.rte")
            if grappleInstance.crankSoundInstance then
                grappleInstance.crankSoundInstance:Play(grappleInstance.parent.Pos)
            end
        else
            -- Extending sound
            if grappleInstance.clickSound then
                grappleInstance.clickSound:Play(grappleInstance.parent.Pos)
            end
        end
    end
    
    grappleInstance.currentLineLength = math.max(10, math.min(grappleInstance.currentLineLength, grappleInstance.maxLineLength))
    
    RopeInputController.handleDirectionalControl(grappleInstance, controller)
    RopeInputController.handleMouseWheelControl(grappleInstance, controller)
end

-- Handle pie menu selections
function RopeInputController.handlePieMenuSelection(grappleInstance)
    if not grappleInstance.parentGun or grappleInstance.parentGun.ID == rte.NoMOID then return false end
    
    -- Only allow pie menu controls if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then return false end
    
    local mode = grappleInstance.parentGun:GetNumberValue("GrappleMode")
    
    if mode and mode ~= 0 then
        grappleInstance.parentGun:RemoveNumberValue("GrappleMode")
        if mode == 3 then
            return true -- Unhook via pie menu
        elseif grappleInstance.actionMode > 1 then
            grappleInstance.pieSelection = mode
            grappleInstance.climb = 0
        end
    end
    return false
end

-- Handle automatic retraction
function RopeInputController.handleAutoRetraction(grappleInstance, terrCheck)
    if not grappleInstance.parentGun or grappleInstance.parentGun.ID == rte.NoMOID or grappleInstance.actionMode <= 1 then
        grappleInstance.pieSelection = 0
        return
    end
    
    -- Only allow auto retraction if gun is equipped
    if not isCurrentlyEquipped(grappleInstance) then 
        grappleInstance.pieSelection = 0
        return 
    end
    
    local parentForces = 1.0
    if grappleInstance.parent and grappleInstance.parent.Vel and grappleInstance.parent.Mass and grappleInstance.lineLength > 0 then
        parentForces = 1 + (grappleInstance.parent.Vel.Magnitude * 10 + grappleInstance.parent.Mass) / (1 + grappleInstance.lineLength)
        parentForces = math.max(0.1, parentForces)
    end
    
    -- Auto retraction when gun is activated
    if grappleInstance.parentGun:IsActivated() and grappleInstance.pieSelection == 0 then
        if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
            grappleInstance.climbTimer:Reset()
            if grappleInstance.currentLineLength > grappleInstance.autoClimbIntervalA then
                grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.autoClimbIntervalA / parentForces)
                grappleInstance.setLineLength = grappleInstance.currentLineLength
            end
        end
    end
    
    -- Pie menu controlled retraction/extension
    if grappleInstance.pieSelection ~= 0 then
        if grappleInstance.climbTimer:IsPastSimMS(grappleInstance.climbDelay) then
            grappleInstance.climbTimer:Reset()
            local actionTaken = false
            
            if grappleInstance.pieSelection == 1 then -- Retract
                if grappleInstance.currentLineLength > grappleInstance.autoClimbIntervalA then
                    grappleInstance.currentLineLength = grappleInstance.currentLineLength - (grappleInstance.autoClimbIntervalA / parentForces)
                    actionTaken = true
                end
            elseif grappleInstance.pieSelection == 2 then -- Extend
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
