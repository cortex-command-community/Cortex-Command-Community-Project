-- Rope Physics Module
-- Handles the physics simulation for the grapple rope

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

-- Update the rope physics using Verlet integration
-- Includes improved damping and spring behavior for better actor safety
function RopePhysics.updateRopePhysics(grappleInstance, startPos, endPos, cablelength)
    local segments = grappleInstance.currentSegments
    if segments < 1 then return end

    local gravity_y = 0.04 -- Slightly reduced gravity for gentler behavior

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

        -- Calculate velocity from position history
        local vel_x = current_x - prev_x
        local vel_y = current_y - prev_y

        -- Apply progressive damping - stronger for faster movements
        local velocity_magnitude = math.sqrt(vel_x*vel_x + vel_y*vel_y)
        local base_damping = 0.98
        local extra_damping = math.min(velocity_magnitude * 0.01, 0.05) -- Additional damping for high velocities
        local damping = base_damping - extra_damping
        
        vel_x = vel_x * damping
        vel_y = vel_y * damping

        -- Store current position as previous for next frame
        grappleInstance.lastX[i] = current_x
        grappleInstance.lastY[i] = current_y

        -- Apply Verlet integration with gravity
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

-- Apply constraints to keep rope segments connected and within length limits
-- Now includes spring damping to prevent actor obliteration from tension
function RopePhysics.applyRopeConstraints(grappleInstance, currentTotalCableLength)
    local segments = grappleInstance.currentSegments
    if segments == 0 then return end

    local targetSegmentLength = currentTotalCableLength / segments
    if targetSegmentLength <= 0 then
        return
    end

    local iterations = 3 -- Fewer iterations for more stability

    for iter = 1, iterations do
        for i = 0, segments - 1 do
            local p1_idx = i
            local p2_idx = i + 1

            local dx = grappleInstance.apx[p2_idx] - grappleInstance.apx[p1_idx]
            local dy = grappleInstance.apy[p2_idx] - grappleInstance.apy[p1_idx]
            
            local distance = math.sqrt(dx*dx + dy*dy)
            
            if distance > 0.001 then
                local difference = targetSegmentLength - distance
                
                -- Spring-like behavior: stronger correction for larger deviations but with limits
                local stretch_ratio = distance / targetSegmentLength
                local correction_strength = 0.2 -- Reduced from 0.3 for gentler corrections
                
                -- Apply spring damping to prevent extreme tension forces
                if stretch_ratio > 1.5 then
                    -- Very stretched - apply gentle restoration to prevent snap-back
                    correction_strength = 0.1
                elseif stretch_ratio > 1.2 then
                    -- Moderately stretched - normal spring force
                    correction_strength = 0.15
                elseif stretch_ratio < 0.7 then
                    -- Very compressed - allow some slack, gentle restoration
                    correction_strength = 0.1
                end
                
                local percent = (difference / distance) * correction_strength
                
                local offsetX = dx * percent * 0.5
                local offsetY = dy * percent * 0.5

                -- Apply corrections based on which points are moveable
                local p1_is_anchor = (p1_idx == 0 or p1_idx == segments)
                local p2_is_anchor = (p2_idx == 0 or p2_idx == segments)
                
                if not p1_is_anchor and not p2_is_anchor then
                    -- Both points are free - move both equally
                    grappleInstance.apx[p1_idx] = grappleInstance.apx[p1_idx] - offsetX
                    grappleInstance.apy[p1_idx] = grappleInstance.apy[p1_idx] - offsetY
                    grappleInstance.apx[p2_idx] = grappleInstance.apx[p2_idx] + offsetX
                    grappleInstance.apy[p2_idx] = grappleInstance.apy[p2_idx] + offsetY
                elseif p1_is_anchor and not p2_is_anchor then
                    -- Only p2 can move - apply spring damping to prevent actor obliteration
                    local force_multiplier = 1.5 -- Reduced from 2 for gentler force
                    grappleInstance.apx[p2_idx] = grappleInstance.apx[p2_idx] + offsetX * force_multiplier
                    grappleInstance.apy[p2_idx] = grappleInstance.apy[p2_idx] + offsetY * force_multiplier
                elseif not p1_is_anchor and p2_is_anchor then
                    -- Only p1 can move - apply spring damping to prevent actor obliteration
                    local force_multiplier = 1.5 -- Reduced from 2 for gentler force
                    grappleInstance.apx[p1_idx] = grappleInstance.apx[p1_idx] - offsetX * force_multiplier
                    grappleInstance.apy[p1_idx] = grappleInstance.apy[p1_idx] - offsetY * force_multiplier
                end
                -- If both are anchors, do nothing
            end
        end
    end
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
    
    -- Break the rope if tension exceeds a threshold
    if tension > grappleInstance.maxTension then
        grappleInstance:Break()
    end
end

return RopePhysics
