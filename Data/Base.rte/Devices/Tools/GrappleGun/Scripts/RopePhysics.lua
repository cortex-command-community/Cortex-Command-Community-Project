-- Rope Physics Module
-- Ultra-rigid Verlet rope physics with pure position-based constraints
-- No dampening, no force accumulation, no direct player manipulation
-- EXTREMELY HARD TO BREAK: Rope only breaks at 500% stretch (5x original length)

local RopePhysics = {}

-- Verlet collision resolution (optimized for many segments)
function RopePhysics.verletCollide(self, h, nextX, nextY)
    --APPLY FRICTION TO INDIVIDUAL JOINTS
    local ray = Vector(nextX, nextY)
    local startpos = Vector(self.apx[h], self.apy[h])
    local rayvec = Vector()
    local rayvec2 = Vector() -- This will store the surface normal

    -- Skip collision check for very short movements to optimize performance
    if ray:MagnitudeIsLessThan(0.05) then -- Further reduced threshold
        self.apx[h] = self.apx[h] + nextX
        self.apy[h] = self.apy[h] + nextY
        return
    end

    rayl = SceneMan:CastObstacleRay(startpos, ray, rayvec, rayvec2, (self.parent and self.parent.ID or 0), self.Team, rte.airID, 0)

    if type(rayl) == "number" and rayl >= 0 then
        -- Collision detected at rayvec
        -- Move point to collision surface, then nudge it slightly along the normal
        local nudgeDistance = 1.0 -- Increased nudge to prevent phasing (was 0.3)
        
        -- Ensure normal is normalized (it should be, but good practice)
        if rayvec2:MagnitudeIsGreaterThan(0.001) then
            rayvec2:SetMagnitude(1)
        else
            -- If normal is zero (should not happen for valid surface), don't nudge or use a default upward nudge
            rayvec2 = Vector(0, -1) -- Default to pushing upwards if normal is bad
        end

        local collisionPointX = rayvec.X + rayvec2.X * nudgeDistance
        local collisionPointY = rayvec.Y + rayvec2.Y * nudgeDistance
        
        self.apx[h] = collisionPointX
        self.apy[h] = collisionPointY
        
        -- Update lastX, lastY so that the velocity for the next frame has a strong rebound
        -- This helps prevent phasing by giving a bounce away from the collision surface
        local bounceStrength = 0.5 -- Velocity component for the bounce
        -- The velocity for the next frame (apx[h] - lastX[h]) should be along the normal (rayvec2)
        -- So, lastX[h] = apx[h] - (rayvec2.X * bounceStrength)
        self.lastX[h] = collisionPointX - rayvec2.X * bounceStrength
        self.lastY[h] = collisionPointY - rayvec2.Y * bounceStrength
        
        -- For the segments at the endpoints (anchors), if they collide, they should stop.
        if h == 0 or h == self.currentSegments then
            -- Set velocity to zero to prevent further movement on next frame
            self.lastX[h] = self.apx[h]
            self.lastY[h] = self.apy[h]
        end
    else
        -- No collision, apply the full displacement.
        self.apx[h] = self.apx[h] + nextX
        self.apy[h] = self.apy[h] + nextY
    end
end

-- Calculate optimal segment count for a given rope length
function RopePhysics.calculateOptimalSegments(self, ropeLength)
    -- Base calculation
    local baseSegments = math.ceil(ropeLength / self.segmentLength)
    
    -- Apply scaling factor for longer ropes (fewer segments per length for very long ropes)
    local scalingFactor = 1.0
    if ropeLength > 200 then
        scalingFactor = 1.0 - math.min(0.5, (ropeLength - 200) / 600)
    end
    
    local desiredSegments = math.ceil(baseSegments * scalingFactor)
    
    -- Ensure within limits
    return math.max(self.minSegments, math.min(desiredSegments, self.maxSegments))
end

-- Determine appropriate physics iterations based on segment count and distance
function RopePhysics.optimizePhysicsIterations(self)
    -- Base iteration count
    if self.currentSegments < 15 and self.currentLineLength < 150 then
        return 5 -- More iterations for shorter, more active ropes
    elseif self.currentSegments > 30 or self.currentLineLength > 300 then
        return 3 -- Fewer for very long ropes to save performance
    end
    
    return 4 -- Default
end

-- Resize the rope segments (add/remove/reposition)
function RopePhysics.resizeRopeSegments(self, segments)
    -- Get current positions to interpolate from
    local startPos = self.parent and self.parent.Pos or Vector(self.apx[0] or self.Pos.X, self.apy[0] or self.Pos.Y)
    local endPos = self.Pos
    
    -- Keep previous end points if they exist
    local prevStart = {x = startPos.X, y = startPos.Y}
    local prevEnd = {x = endPos.X, y = endPos.Y}
    local prevEndVel = {x = 0, y = 0}
    
    if self.apx[0] then
        prevStart = {x = self.apx[0], y = self.apy[0]}
    end
    
    if self.apx[self.currentSegments] then
        prevEnd = {x = self.apx[self.currentSegments], y = self.apy[self.currentSegments]}
        if self.lastX[self.currentSegments] then
            prevEndVel.x = self.apx[self.currentSegments] - self.lastX[self.currentSegments]
            prevEndVel.y = self.apy[self.currentSegments] - self.lastY[self.currentSegments]
        end
    end
    
    -- Initialize arrays with appropriate number of segments
    for i = 0, segments do
        -- Interpolate positions between start and end points
        local t = i / math.max(1, segments)
        self.apx[i] = prevStart.x * (1-t) + prevEnd.x * t
        self.apy[i] = prevStart.y * (1-t) + prevEnd.y * t
        self.lastX[i] = self.apx[i]
        self.lastY[i] = self.apy[i]
    end
    
    -- Special handling for anchor points
    if self.parent then
        -- Point 0 is anchored to player
        self.apx[0] = self.parent.Pos.X
        self.apy[0] = self.parent.Pos.Y
    end
    
    -- Point segments is anchored to hook
    self.apx[segments] = self.Pos.X
    self.apy[segments] = self.Pos.Y
    
    -- Update velocity for end point if we have it
    if prevEndVel.x ~= 0 or prevEndVel.y ~= 0 then
        self.lastX[segments] = self.apx[segments] - prevEndVel.x
        self.lastY[segments] = self.apy[segments] - prevEndVel.y
    end
    
    self.currentSegments = segments
end

-- Update rope segments to form a straight line during flight
function RopePhysics.updateRopeFlightPath(self)
    if not (self.parent and self.apx and self.currentSegments > 0) then
        return
    end
    
    -- Calculate the direct path from player to hook
    local startPos = self.parent.Pos
    local endPos = self.Pos
    local ropeVector = SceneMan:ShortestDistance(startPos, endPos, self.mapWrapsX)
    local distance = ropeVector.Magnitude
    
    -- Update total rope length to match the straight-line distance
    self.currentLineLength = distance
    self.lineLength = distance
    
    -- Create perfectly straight rope segments
    for i = 0, self.currentSegments do
        local t = i / math.max(1, self.currentSegments)
        
        -- Calculate position along the straight line
        local segmentPos = startPos + Vector(ropeVector.X * t, ropeVector.Y * t)
        
        self.apx[i] = segmentPos.X
        self.apy[i] = segmentPos.Y
        
        -- Set previous positions to current positions to prevent velocity
        self.lastX[i] = segmentPos.X
        self.lastY[i] = segmentPos.Y
    end
    
    -- Ensure exact anchor positions
    self.apx[0] = startPos.X
    self.apy[0] = startPos.Y
    self.apx[self.currentSegments] = endPos.X
    self.apy[self.currentSegments] = endPos.Y
end

-- Update the rope physics using Verlet integration with no dampening
function RopePhysics.updateRopePhysics(grappleInstance, startPos, endPos, cablelength)
    local segments = grappleInstance.currentSegments
    if segments < 1 then return end

    local gravity_y = 0.1 -- Normal gravity for realistic rope behavior

    -- Initialize previous positions for new points
    for i = 0, segments do
        if grappleInstance.lastX[i] == nil then
            grappleInstance.lastX[i] = grappleInstance.apx[i]
            grappleInstance.lastY[i] = grappleInstance.apy[i]
        end
    end

    -- Verlet integration for interior points only (not anchor points)
    for i = 1, segments - 1 do
        local current_x = grappleInstance.apx[i]
        local current_y = grappleInstance.apy[i]
        local prev_x = grappleInstance.lastX[i]
        local prev_y = grappleInstance.lastY[i]

        -- Calculate velocity from position history (no dampening applied)
        local vel_x = current_x - prev_x
        local vel_y = current_y - prev_y

        -- Store current position as previous for next frame
        grappleInstance.lastX[i] = current_x
        grappleInstance.lastY[i] = current_y

        -- Apply Verlet integration with gravity (no dampening)
        grappleInstance.apx[i] = current_x + vel_x
        grappleInstance.apy[i] = current_y + vel_y + gravity_y
    end

    -- Set anchor positions AFTER physics update with velocity tracking
    -- Update anchor positions and track their velocity for wave propagation
    if grappleInstance.anchorVelX == nil then 
        grappleInstance.anchorVelX = {[0] = 0, [segments] = 0}
        grappleInstance.anchorVelY = {[0] = 0, [segments] = 0}
    end
    
    -- Track start anchor (player) velocity
    local startVelX = startPos.X - grappleInstance.apx[0]
    local startVelY = startPos.Y - grappleInstance.apy[0]
    grappleInstance.anchorVelX[0] = startVelX
    grappleInstance.anchorVelY[0] = startVelY
    
    -- Track end anchor (hook) velocity
    local endVelX = endPos.X - grappleInstance.apx[segments]
    local endVelY = endPos.Y - grappleInstance.apy[segments]
    grappleInstance.anchorVelX[segments] = endVelX
    grappleInstance.anchorVelY[segments] = endVelY
    
    -- Set anchor positions
    grappleInstance.apx[0] = startPos.X
    grappleInstance.apy[0] = startPos.Y
    grappleInstance.lastX[0] = startPos.X - startVelX
    grappleInstance.lastY[0] = startPos.Y - startVelY

    grappleInstance.apx[segments] = endPos.X
    grappleInstance.apy[segments] = endPos.Y
    grappleInstance.lastX[segments] = endPos.X - endVelX
    grappleInstance.lastY[segments] = endPos.Y - endVelY
    
    -- Propagate anchor movement to adjacent segments for wave effects
    if math.abs(startVelX) > 0.1 or math.abs(startVelY) > 0.1 then
        -- Player anchor moved - affect first segment
        if segments > 1 then
            grappleInstance.apx[1] = grappleInstance.apx[1] + startVelX * 0.3
            grappleInstance.apy[1] = grappleInstance.apy[1] + startVelY * 0.3
        end
    end
    
    if math.abs(endVelX) > 0.1 or math.abs(endVelY) > 0.1 then
        -- Hook anchor moved - affect last segment
        if segments > 1 then
            grappleInstance.apx[segments-1] = grappleInstance.apx[segments-1] + endVelX * 0.3
            grappleInstance.apy[segments-1] = grappleInstance.apy[segments-1] + endVelY * 0.3
        end
    end
end

-- Advanced force protection system for preventing actor death from rope forces
function RopePhysics.calculateActorProtection(grappleInstance, force_magnitude, force_direction)
    -- Simplified - just return reduced values, no complex calculations
    local safe_force_magnitude = math.min(force_magnitude, 5.0) -- Hard cap at 5
    local safe_force_vector = force_direction * safe_force_magnitude
    return safe_force_magnitude, safe_force_vector
end

-- Apply stored forces gradually over time
function RopePhysics.applyStoredForces(grappleInstance)
    -- Disabled - no stored forces in pure Verlet implementation
end

-- Proper Verlet rope constraint satisfaction with rigid distance constraints
-- Implements true non-stretchy rope behavior using position-based dynamics
-- Now properly integrated with centralized length control
function RopePhysics.applyRopeConstraints(grappleInstance, currentTotalCableLength)
    local segments = grappleInstance.currentSegments
    if segments == 0 then return false end
    
    if not grappleInstance.parent then return false end

    -- Use the centrally controlled rope length as the maximum constraint
    local maxRopeLength = grappleInstance.currentLineLength or grappleInstance.maxLineLength
    
    -- FIRST: Enforce rigid maximum distance constraint through rope physics only
    -- Pure Verlet implementation - no direct player position manipulation
    local playerPos = grappleInstance.parent.Pos
    local hookPos = Vector(grappleInstance.apx[segments], grappleInstance.apy[segments])
    local ropeVector = SceneMan:ShortestDistance(playerPos, hookPos, grappleInstance.mapWrapsX)
    local totalRopeDistance = ropeVector.Magnitude
    
    -- Update anchor positions for constraint calculations
    grappleInstance.apx[0] = playerPos.X
    grappleInstance.apy[0] = playerPos.Y

    -- GLOBAL CONSTRAINT: Handle rope length constraints smoothly
    if totalRopeDistance > maxRopeLength then
        local excessDistance = totalRopeDistance - maxRopeLength
        local constraintDirection = ropeVector:SetMagnitude(1)
        
        -- Check if hook is anchored (attached to terrain or MO)
        -- if grappleInstance.actionMode >= 2 then  -- Original condition
        if grappleInstance.actionMode == 2 then -- Changed: Actor anchored to claw only in mode 2
            -- Hook is anchored - apply PROPER SWINGING CONSTRAINT
            -- This allows free tangential movement (swinging) while constraining radial movement
            
            local currentVelocity = grappleInstance.parent.Vel
            
            -- Calculate velocity component toward/away from hook
            -- constraintDirection points FROM player TO hook
            local radialVelocity = currentVelocity:Dot(constraintDirection)
            
            -- Only constrain the radial component if moving away from hook (stretching rope)
            if radialVelocity < 0 then
                -- Player is moving away from hook - remove ONLY the radial component
                -- Keep all tangential velocity for swinging motion
                local radialVelocityVector = constraintDirection * radialVelocity
                local tangentialVelocity = currentVelocity - radialVelocityVector
                
                -- Set velocity to pure tangential motion (perfect swinging)
                grappleInstance.parent.Vel = tangentialVelocity
                
                -- Set tension for physics feedback
                grappleInstance.ropeTensionForce = -radialVelocity * 0.5
                grappleInstance.ropeTensionDirection = constraintDirection
            else
                -- Player is moving toward hook or tangentially - no constraint needed
                -- This allows free movement inward and pure swinging motion
                grappleInstance.ropeTensionForce = nil
                grappleInstance.ropeTensionDirection = nil
            end
            
            -- CRITICAL: Also enforce position constraint to prevent gradual stretching
            -- After constraining velocity, ensure player doesn't drift beyond max rope length
            if totalRopeDistance > maxRopeLength then
                local correctionDistance = totalRopeDistance - maxRopeLength
                local correctionVector = constraintDirection * correctionDistance
                
                -- Move player back to exact rope radius (smooth correction)
                local correctionStrength = 0.8 -- Strong but not instant correction
                grappleInstance.parent.Pos = grappleInstance.parent.Pos + correctionVector * correctionStrength
                
                -- Update rope anchor to match corrected player position
                grappleInstance.apx[0] = grappleInstance.parent.Pos.X
                grappleInstance.apy[0] = grappleInstance.parent.Pos.Y
            end
        elseif grappleInstance.actionMode == 1 then -- Added: Claw anchored to actor in mode 1
            -- Hook is in flight, anchor it to the player
            local correctionVector = constraintDirection * excessDistance
            -- Move the player instead of the hook
            grappleInstance.parent.Pos = grappleInstance.parent.Pos + correctionVector
            -- Update rope anchor to match corrected player position
            grappleInstance.apx[0] = grappleInstance.parent.Pos.X
            grappleInstance.apy[0] = grappleInstance.parent.Pos.Y

            -- Clear any tension forces since rope is not under tension
            grappleInstance.ropeTensionForce = nil
            grappleInstance.ropeTensionDirection = nil

            -- Recalculate after constraint
            playerPos = grappleInstance.parent.Pos -- update playerPos for subsequent calculations
            ropeVector = SceneMan:ShortestDistance(playerPos, hookPos, grappleInstance.mapWrapsX)
            totalRopeDistance = ropeVector.Magnitude
        else
            -- Hook is in flight - we can move it to maintain rope length (default case)
            local correctionVector = constraintDirection * excessDistance
            grappleInstance.apx[segments] = grappleInstance.apx[segments] - correctionVector.X
            grappleInstance.apy[segments] = grappleInstance.apy[segments] - correctionVector.Y
            
            -- Clear any tension forces since rope is not under tension
            grappleInstance.ropeTensionForce = nil
            grappleInstance.ropeTensionDirection = nil
            
            -- Recalculate after constraint
            hookPos = Vector(grappleInstance.apx[segments], grappleInstance.apy[segments])
            ropeVector = SceneMan:ShortestDistance(playerPos, hookPos, grappleInstance.mapWrapsX)
            totalRopeDistance = ropeVector.Magnitude
        end
    else
        -- Rope is not at maximum length - clear tension forces
        grappleInstance.ropeTensionForce = nil
        grappleInstance.ropeTensionDirection = nil
    end

    -- SECOND: Apply smooth rope retraction if rope is being shortened
    -- This prevents "snapping" when the player retracts the rope
    local currentActualLength = 0
    for i = 0, segments - 1 do
        local dx = grappleInstance.apx[i+1] - grappleInstance.apx[i]
        local dy = grappleInstance.apy[i+1] - grappleInstance.apy[i]
        currentActualLength = currentActualLength + math.sqrt(dx*dx + dy*dy)
    end
    
    if currentActualLength > maxRopeLength then
        -- Rope needs to be shortened - apply smooth contraction
        local contractionRatio = maxRopeLength / currentActualLength
        local contractionSpeed = 0.1 -- Smooth retraction speed
        
        -- Smoothly contract each segment toward the desired length
        for i = 1, segments - 1 do
            local toHook = Vector(grappleInstance.apx[segments] - grappleInstance.apx[i], 
                                grappleInstance.apy[segments] - grappleInstance.apy[i])
            local distanceToHook = toHook.Magnitude
            
            if distanceToHook > 0.1 then
                -- Move segment gradually toward hook
                local contractionDirection = toHook:SetMagnitude(1)
                local contractionAmount = distanceToHook * (1 - contractionRatio) * contractionSpeed
                
                grappleInstance.apx[i] = grappleInstance.apx[i] + contractionDirection.X * contractionAmount
                grappleInstance.apy[i] = grappleInstance.apy[i] + contractionDirection.Y * contractionAmount
            end
        end
    end

    -- THIRD: Apply rigid Verlet constraints for rope segments using MAXIMUM ALLOWED length
    -- This prevents gradual stretching during swinging by enforcing the max rope length
    local targetSegmentLength = maxRopeLength / segments -- Use maximum allowed length, not current distance
    local iterations = 32 -- High iteration count for rigid rope behavior
    local constraint_strength = 1.0 -- Full strength for completely rigid rope

    for iter = 1, iterations do
        for i = 0, segments - 1 do
            local p1_idx = i
            local p2_idx = i + 1

            local x1, y1 = grappleInstance.apx[p1_idx], grappleInstance.apy[p1_idx]
            local x2, y2 = grappleInstance.apx[p2_idx], grappleInstance.apy[p2_idx]
            
            local dx = x2 - x1
            local dy = y2 - y1
            local distance = math.sqrt(dx*dx + dy*dy)
            
            if distance > 0.001 then -- Avoid division by zero
                -- Calculate exact constraint satisfaction
                local difference = targetSegmentLength - distance
                local percent = (difference / distance) * constraint_strength
                local offsetX = dx * percent * 0.5
                local offsetY = dy * percent * 0.5

                -- Check which points are anchors
                local p1_is_anchor = (p1_idx == 0) -- Player anchor
                local p2_is_anchor = (p2_idx == segments) -- Hook anchor
                
                if not p1_is_anchor and not p2_is_anchor then
                    -- Both points are free - move both equally
                    grappleInstance.apx[p1_idx] = x1 - offsetX
                    grappleInstance.apy[p1_idx] = y1 - offsetY
                    grappleInstance.apx[p2_idx] = x2 + offsetX
                    grappleInstance.apy[p2_idx] = y2 + offsetY
                    
                elseif p1_is_anchor and not p2_is_anchor then
                    -- P1 is player anchor - only move P2
                    -- Pure position-based constraints - no force feedback to player
                    grappleInstance.apx[p2_idx] = x2 + offsetX * 2
                    grappleInstance.apy[p2_idx] = y2 + offsetY * 2
                    
                elseif not p1_is_anchor and p2_is_anchor then
                    -- P2 is hook anchor - only move P1
                    grappleInstance.apx[p1_idx] = x1 - offsetX * 2
                    grappleInstance.apy[p1_idx] = y1 - offsetY * 2
                end
            end
        end
    end

    -- Calculate final rope distance and segment lengths for breaking check and debug info
    local finalRopeDistance = 0
    local segmentLengths = {}
    for i = 0, segments - 1 do
        local dx = grappleInstance.apx[i+1] - grappleInstance.apx[i]
        local dy = grappleInstance.apy[i+1] - grappleInstance.apy[i]
        local segmentLength = math.sqrt(dx*dx + dy*dy)
        segmentLengths[i] = segmentLength
        finalRopeDistance = finalRopeDistance + segmentLength
    end
    
    -- Store segment length data for debug display
    grappleInstance.segmentLengths = segmentLengths
    grappleInstance.actualRopeLength = finalRopeDistance
    
    -- EXTREMELY HARD TO BREAK: Only break at 500% stretch (5x original length)
    -- This makes the rope virtually indestructible under normal conditions
    if finalRopeDistance > maxRopeLength * 5.0 then -- Break at 500% stretch - extremely high threshold
        grappleInstance.shouldBreak = true
        return true
    end

    -- Store tension as stretch ratio for feedback
    grappleInstance.currentTension = math.max(0, (finalRopeDistance - maxRopeLength) / maxRopeLength)

    return false -- Rope didn't break
end

-- Smooth the rope using weighted averaging to reduce jaggedness
function RopePhysics.smoothRope(grappleInstance)
    local segments = grappleInstance.currentSegments
    if segments < 3 then return end
    
    -- Very light smoothing that doesn't interfere with physics
    local smoothing_strength = 0.1
    
    -- Create temporary arrays for smoothed positions
    local smoothedX = {}
    local smoothedY = {}
    
    -- Copy all points first
    for i = 0, segments do
        smoothedX[i] = grappleInstance.apx[i]
        smoothedY[i] = grappleInstance.apy[i]
    end
    
    -- Apply very light smoothing to intermediate points only
    for i = 1, segments - 1 do
        local avgX = (grappleInstance.apx[i-1] + grappleInstance.apx[i] + grappleInstance.apx[i+1]) / 3
        local avgY = (grappleInstance.apy[i-1] + grappleInstance.apy[i] + grappleInstance.apy[i+1]) / 3
        
        smoothedX[i] = grappleInstance.apx[i] * (1 - smoothing_strength) + avgX * smoothing_strength
        smoothedY[i] = grappleInstance.apy[i] * (1 - smoothing_strength) + avgY * smoothing_strength
    end
    
    -- Apply smoothed positions back to rope (except anchors)
    for i = 1, segments - 1 do
        grappleInstance.apx[i] = smoothedX[i]
        grappleInstance.apy[i] = smoothedY[i]
    end
end

-- Handle player pulling on the rope (manual or automatic)
function RopePhysics.handleRopePull(grappleInstance, controller, terrCheck)
    local player = grappleInstance.parent
    local segments = grappleInstance.currentSegments
    
    -- Manual pull - this would need to be handled by the calling code
    -- as we don't have access to the controller constants here
    
    -- Automatic retraction (e.g., rope not taut)
    if grappleInstance.currentLineLength < grappleInstance.maxLineLength then
        local retractVec = Vector(grappleInstance.apx[segments], grappleInstance.apy[segments]) - player.Pos
        retractVec:SetMagnitude(1)
        grappleInstance.apx[segments] = grappleInstance.apx[segments] - retractVec.X
        grappleInstance.apy[segments] = grappleInstance.apy[segments] - retractVec.Y
    end
end

-- Handle player extending the rope
function RopePhysics.handleRopeExtend(grappleInstance)
    if grappleInstance.currentLineLength < grappleInstance.maxLineLength then
        -- Placeholder for rope extension logic
        -- This might involve increasing grappleInstance.currentLineLength
        -- or allowing the hook to move further if not anchored.
    end
end

function RopePhysics.checkRopeBreak(grappleInstance)
    -- Calculate tension (simplified)
    local tension = 0
    local segments = grappleInstance.currentSegments
    
    for i = 0, segments - 1 do
        local dx = grappleInstance.apx[i+1] - grappleInstance.apx[i]
        local dy = grappleInstance.apy[i+1] - grappleInstance.apy[i]
        tension = tension + math.sqrt(dx*dx + dy*dy)
    end
    
    -- EXTREMELY HARD TO BREAK: Only break if tension exceeds 5x the line strength
    if tension > (grappleInstance.lineStrength or 10000) * 5 then
        grappleInstance.shouldBreak = true
    end
end

return RopePhysics
