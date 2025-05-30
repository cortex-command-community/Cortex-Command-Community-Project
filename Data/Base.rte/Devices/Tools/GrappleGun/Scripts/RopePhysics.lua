-- Grapple Gun Physics Module
-- Handles the physics simulation for the rope

local RopePhysics = {}

-- Verlet collision resolution (optimized for many segments)
function RopePhysics.verletCollide(self, h, nextX, nextY)
    --APPLY FRICTION TO INDIVIDUAL JOINTS
    local ray = Vector(nextX, nextY)
    local startpos = Vector(self.apx[h], self.apy[h])
    local rayvec = Vector()
    local rayvec2 = Vector()

    -- Skip collision check for very short movements to optimize performance
    if ray:MagnitudeIsLessThan(0.2) then
        self.apx[h] = self.apx[h] + nextX
        self.apy[h] = self.apy[h] + nextY
        return
    end

    -- Adaptive ray casting - use faster/simpler method for mid-segments
    local rayl
    if h > 1 and h < self.currentSegments - 1 then
        -- Mid segments use simplified collision
        rayl = SceneMan:CastStrengthRay(startpos, ray, 1, rayvec, 0, rte.airID, self.mapWrapsX)
    else
        -- End segments near player/target use more precise collision
        rayl = SceneMan:CastObstacleRay(startpos, ray, rayvec, rayvec2, self.parent.ID, self.Team, rte.airID, 0)
    end

    if rayl >= 0 then
        local ud = SceneMan:ShortestDistance(rayvec, startpos, true)
        local angle2 = ud.AbsRadAngle
        local usemag = math.min(ray.Magnitude*0.8, 0.8) -- Reduced bounce response
        local changevect = Vector(usemag, 0):RadRotate(angle2)
        self.apx[h] = self.apx[h] + changevect.X
        self.apy[h] = self.apy[h] + changevect.Y
        -- Apply damping to velocity after collision
        self.lastX[h] = rayvec.X * 0.7
        self.lastY[h] = rayvec.Y * 0.7
    else
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
    local baseIterations = 3
    
    -- For very long ropes or many segments, reduce iterations to maintain performance
    if self.currentSegments > 30 or self.currentLineLength > 300 then
        return 2
    -- For very short ropes, increase iterations for stability
    elseif self.currentSegments < 8 and self.currentLineLength < 100 then
        return 4
    end
    
    return baseIterations
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
    
    -- Calculate the direct path
    local startPos = self.parent.Pos
    local endPos = self.Pos
    local distance = SceneMan:ShortestDistance(startPos, endPos, self.mapWrapsX).Magnitude
    
    -- Update segment positions
    for i = 0, self.currentSegments do
        local t = i / self.currentSegments
        local segmentPos = SceneMan:ShortestDistance(startPos, endPos, self.mapWrapsX)
        segmentPos:SetMagnitude(distance * t)
        segmentPos = startPos + segmentPos
        
        self.apx[i] = segmentPos.X
        self.apy[i] = segmentPos.Y
        self.lastX[i] = segmentPos.X
        self.lastY[i] = segmentPos.Y
    end
end

return RopePhysics
