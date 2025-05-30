-- Grapple Gun State Manager Module
-- Handles state transitions and physics effects based on grapple state

local RopeStateManager = {}

-- Initialize rope state (called from Create function)
function RopeStateManager.initState(grappleInstance)
    grappleInstance.actionMode = 0    -- 0 = start, 1 = flying, 2 = grab terrain, 3 = grab MO
    grappleInstance.limitReached = false
    grappleInstance.canRelease = false
    grappleInstance.currentLineLength = 0
    grappleInstance.longestLineLength = 0
    grappleInstance.setLineLength = 0
    
    -- Ensure parent and parent gun are initialized properly
    -- This is typically called separately in the Create function
end

-- Handle state changes from flight to attached state
function RopeStateManager.checkAttachmentCollisions(grappleInstance)
    -- Only process in flight state
    if grappleInstance.actionMode ~= 1 then return false end
    
    local stateChanged = false
    local length = math.sqrt(grappleInstance.Diameter + grappleInstance.Vel.Magnitude)
    -- Detect terrain and stick if found
    local ray = Vector(length, 0):RadRotate(grappleInstance.Vel.AbsRadAngle)
    grappleInstance.rayVec = Vector()
    
    if SceneMan:CastStrengthRay(grappleInstance.Pos, ray, 0, grappleInstance.rayVec, 0, rte.airID, grappleInstance.mapWrapsX) then
        grappleInstance.actionMode = 2
        stateChanged = true
    else
        -- Detect MOs and stick if found
        local moRay = SceneMan:CastMORay(grappleInstance.Pos, ray, grappleInstance.parent.ID, -2, rte.airID, false, 0)
        if moRay ~= rte.NoMOID then
            grappleInstance.target = MovableMan:GetMOFromID(moRay)
            -- Treat pinned MOs as terrain
            if grappleInstance.target.PinStrength > 0 then
                grappleInstance.actionMode = 2
                stateChanged = true
            else
                -- Store the offset from the object so we can maintain it when the object moves/rotates
                grappleInstance.stickPosition = SceneMan:ShortestDistance(grappleInstance.target.Pos, grappleInstance.Pos, grappleInstance.mapWrapsX)
                grappleInstance.stickRotation = grappleInstance.target.RotAngle
                grappleInstance.stickDirection = grappleInstance.RotAngle
                grappleInstance.actionMode = 3
                stateChanged = true
            end
            
            -- Inflict damage on the target
            local part = CreateMOPixel("Grapple Gun Damage Particle")
            part.Pos = grappleInstance.Pos
            part.Vel = SceneMan:ShortestDistance(grappleInstance.Pos, grappleInstance.target.Pos, grappleInstance.mapWrapsX):SetMagnitude(grappleInstance.Vel.Magnitude)
            MovableMan:AddParticle(part)
        end
    end
    
    -- Handle state change initialization
    if stateChanged then
        grappleInstance.stickSound:Play(grappleInstance.Pos)
        grappleInstance.currentLineLength = math.floor(grappleInstance.lineLength)
        grappleInstance.setLineLength = grappleInstance.currentLineLength
        grappleInstance.Vel = Vector() -- Stop the hook
        grappleInstance.PinStrength = 1000
        grappleInstance.Frame = 1 -- Change appearance
    end
    
    return stateChanged
end

-- Handle exceeding maximum length
function RopeStateManager.checkLengthLimit(grappleInstance)
    if grappleInstance.lineLength > grappleInstance.maxLineLength then
        if grappleInstance.limitReached == false then
            grappleInstance.limitReached = true
            grappleInstance.clickSound:Play(grappleInstance.parent.Pos)
        end
        
        -- Handle position limiting
        local movetopos = grappleInstance.parent.Pos + (grappleInstance.lineVec):SetMagnitude(grappleInstance.maxLineLength)
        if grappleInstance.mapWrapsX == true then
            if movetopos.X > SceneMan.SceneWidth then
                movetopos = Vector(movetopos.X - SceneMan.SceneWidth, movetopos.Y)
            elseif movetopos.X < 0 then
                movetopos = Vector(SceneMan.SceneWidth + movetopos.X, movetopos.Y)
            end
        end
        grappleInstance.Pos = movetopos
        
        -- Reduce velocity in direction of rope
        local pullamountnumber = math.abs(-grappleInstance.lineVec.AbsRadAngle + grappleInstance.Vel.AbsRadAngle)/6.28
        grappleInstance.Vel = grappleInstance.Vel - grappleInstance.lineVec:SetMagnitude(grappleInstance.Vel.Magnitude * pullamountnumber)
        
        return true
    end
    
    return false
end

-- Apply elastic stretch dynamics when in stretch mode
function RopeStateManager.applyStretchMode(grappleInstance)
    if not grappleInstance.stretchMode then return end
    
    if grappleInstance.actionMode == 1 then
        -- Stretch mode: gradually retract the hook for a return hit
        grappleInstance.Vel = grappleInstance.Vel - 
                            Vector(grappleInstance.lineVec.X, grappleInstance.lineVec.Y)
                            :SetMagnitude(math.sqrt(grappleInstance.lineLength) * 
                                        grappleInstance.stretchPullRatio/2)
    end
end

-- Apply terrain pull physics
function RopeStateManager.applyTerrainPullPhysics(grappleInstance)
    if grappleInstance.actionMode != 2 then return end
    
    if grappleInstance.stretchMode then
        local pullVec = grappleInstance.lineVec:SetMagnitude(0.15 * math.sqrt(grappleInstance.lineLength)/
                                                          grappleInstance.parentForces)
        grappleInstance.parent.Vel = grappleInstance.parent.Vel + pullVec
    elseif grappleInstance.lineLength > grappleInstance.currentLineLength then
        local hookVel = SceneMan:ShortestDistance(Vector(grappleInstance.PrevPos.X, grappleInstance.PrevPos.Y), 
                                                Vector(grappleInstance.Pos.X, grappleInstance.PrevPos.Y), 
                                                grappleInstance.mapWrapsX)

        local pullAmountNumber = grappleInstance.lineVec.AbsRadAngle - grappleInstance.parent.Vel.AbsRadAngle
        if pullAmountNumber < 0 then
            pullAmountNumber = pullAmountNumber * -1
        end

        pullAmountNumber = pullAmountNumber/6.28
        
        -- Apply force to parent based on rope physics
        grappleInstance.parent:AddAbsForce(grappleInstance.lineVec:SetMagnitude(
                                        ((grappleInstance.lineLength - grappleInstance.currentLineLength)^3) * 
                                        pullAmountNumber) + 
                                        hookVel:SetMagnitude(math.pow(grappleInstance.lineLength - 
                                                                   grappleInstance.currentLineLength, 2) * 0.8), 
                                        grappleInstance.parent.Pos)

        -- Instead of direct path, use the rope path to pull the player
        -- Calculate rope force along the first segment direction
        local segmentVec = Vector(grappleInstance.apx[1] - grappleInstance.apx[0], 
                                grappleInstance.apy[1] - grappleInstance.apy[0])
        local pullDirection = segmentVec:SetMagnitude(1)
        
        -- Apply force along the rope path rather than direct line
        local tensionForce = (grappleInstance.lineLength - grappleInstance.currentLineLength) * 2
        grappleInstance.parent:AddForce(pullDirection * tensionForce, grappleInstance.parent.Pos)
        
        -- Add rope tension feedback to the player via camera shake
        if tensionForce > 15 and grappleInstance.parent:IsPlayerControlled() then
            local screenShake = math.min(tensionForce * 0.05, 2.0)
            FrameMan:SetScreenScrollSpeed(screenShake)
        end
        
        -- Break the rope if the forces are too high
        local pullAmountNumber = math.abs(grappleInstance.lineVec.AbsRadAngle - grappleInstance.parent.Vel.AbsRadAngle)/6.28
        if (grappleInstance.parent.Vel - grappleInstance.lineVec:SetMagnitude(
                                      grappleInstance.parent.Vel.Magnitude * pullAmountNumber))
                                      :MagnitudeIsGreaterThan(grappleInstance.lineStrength) then
            return true -- Signal to delete the hook due to excessive force
        end
        
        grappleInstance.parent.Vel = grappleInstance.parent.Vel + grappleInstance.lineVec
    end
    
    return false
end

-- Apply MO pull physics when attached to a movable object
function RopeStateManager.applyMOPullPhysics(grappleInstance)
    if grappleInstance.actionMode != 3 or not grappleInstance.target then return false
    
    if grappleInstance.target.ID ~= rte.NoMOID then
        -- Update the hook position based on the object it's attached to
        grappleInstance.Pos = grappleInstance.target.Pos + 
                            Vector(grappleInstance.stickPosition.X, grappleInstance.stickPosition.Y)
                            :RadRotate(grappleInstance.target.RotAngle - grappleInstance.stickRotation)
        grappleInstance.RotAngle = grappleInstance.stickDirection + 
                                 (grappleInstance.target.RotAngle - grappleInstance.stickRotation)
        
        -- Update rope anchor point for hook position
        grappleInstance.apx[grappleInstance.currentSegments] = grappleInstance.Pos.X
        grappleInstance.apy[grappleInstance.currentSegments] = grappleInstance.Pos.Y

        local jointStiffness
        local target = grappleInstance.target
        if target.ID ~= target.RootID then
            local mo = target:GetRootParent()
            if mo.ID ~= rte.NoMOID and IsAttachable(target) then
                -- It's best to apply all the forces to the parent instead of utilizing JointStiffness
                target = mo
            end
        end

        if grappleInstance.stretchMode then
            local pullVec = grappleInstance.lineVec:SetMagnitude(grappleInstance.stretchPullRatio * 
                                                             math.sqrt(grappleInstance.lineLength)/
                                                             grappleInstance.parentForces)
            grappleInstance.parent.Vel = grappleInstance.parent.Vel + pullVec

            local targetForces = 1 + (target.Vel.Magnitude * 10 + target.Mass)/(1 + grappleInstance.lineLength)
            target.Vel = target.Vel - (pullVec) * grappleInstance.parentForces/targetForces
        elseif grappleInstance.lineLength > grappleInstance.currentLineLength then
            -- Take wrapping to account, treat all distances relative to hook
            local parentPos = target.Pos + SceneMan:ShortestDistance(target.Pos, 
                                                                  grappleInstance.parent.Pos, 
                                                                  grappleInstance.mapWrapsX)
            -- Add forces to both user and the target MO
            local hookVel = SceneMan:ShortestDistance(Vector(grappleInstance.PrevPos.X, 
                                                         grappleInstance.PrevPos.Y), 
                                                     Vector(grappleInstance.Pos.X, 
                                                         grappleInstance.Pos.Y), 
                                                     grappleInstance.mapWrapsX)

            local pullAmountNumber = grappleInstance.lineVec.AbsRadAngle - grappleInstance.parent.Vel.AbsRadAngle
            if pullAmountNumber < 0 then
                pullAmountNumber = pullAmountNumber * -1
            end
            pullAmountNumber = pullAmountNumber/6.28
            
            -- Apply forces to player
            grappleInstance.parent:AddAbsForce(grappleInstance.lineVec
                                           :SetMagnitude((grappleInstance.lineLength - 
                                                       grappleInstance.currentLineLength) * 
                                                       pullAmountNumber * 9 / grappleInstance.parentForces), 
                                            grappleInstance.parent.Pos)
            
            -- Break rope if forces too high                                
            if (grappleInstance.parent.Vel - grappleInstance.lineVec
                                          :SetMagnitude(grappleInstance.parent.Vel.Magnitude * 
                                                     pullAmountNumber))
                                          :MagnitudeIsGreaterThan(grappleInstance.lineStrength) then
                return true -- Signal to delete the hook due to excessive force
            end
            
            -- Apply forces to target                              
            local targetForces = 1 + (target.Vel.Magnitude * 10 + target.Mass)/(1 + grappleInstance.lineLength)
            target:AddForce(grappleInstance.lineVec:SetMagnitude((grappleInstance.lineLength - 
                                                              grappleInstance.currentLineLength) * 5), 
                         parentPos)
            
            -- Add some dampening for smoother motion
            target.Vel = target.Vel * 0.98
            target.AngularVel = target.AngularVel * 0.99
        end
    else
        -- Our MO has been destroyed, return hook
        return true -- Signal to delete the hook
    end
    
    return false
end

-- Update maximum line length if it needs to be capped
function RopeStateManager.checkLineLengthUpdate(grappleInstance)
    if grappleInstance.currentLineLength > grappleInstance.maxLineLength then
        grappleInstance.currentLineLength = grappleInstance.currentLineLength - 1 -- TIGHTEN ROPE
        if grappleInstance.limitReached == false then
            grappleInstance.limitReached = true
            grappleInstance.clickSound:Play(grappleInstance.parent.Pos)
        end
    end
end

-- Determine if the grapple can be released based on its current state
function RopeStateManager.canReleaseGrapple(grappleInstance)
    -- Check if the grapple is in a state where it can be released
    -- For now just return the canRelease property, but this could be expanded
    -- with additional logic in the future if needed
    return grappleInstance.canRelease
end

return RopeStateManager
