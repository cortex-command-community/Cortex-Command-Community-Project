---@diagnostic disable: undefined-global
-- filepath: /home/cretin/git/Cortex-Command-Community-Project/Data/Base.rte/Devices/Tools/GrappleGun/Scripts/RopePhysics.lua
--[[ 
  RopePhysics.lua - Advanced Rope Physics Module
  
  Implements Verlet integration for rope physics with position-based constraints.
  Aims for a rigid rope behavior with high durability.
--]]

local RopeStateManager = require("Devices.Tools.GrappleGun.Scripts.RopeStateManager") -- Added this line

local RopePhysics = {}

-- Constants for physics behavior
local GRAVITY_Y = 0.1 -- Simulate normal gravity for the rope segments.
local NUDGE_DISTANCE = 0.5 -- Increased from 0.3 to help prevent phasing through terrain.
local BOUNCE_STRENGTH = 0.3 -- How much velocity is retained perpendicular to a collision surface.
local CONSTRAINT_STRENGTH = 1.0 -- Full strength for rigid rope constraints.
local DEFAULT_PHYSICS_ITERATIONS = 32 -- Default number of constraint iterations. User request.

--[[
  Resolves collisions for a single rope segment using raycasting.
  @param self  The grapple instance.
  @param segmentIdx The index of the segment point to process.
  @param nextX The potential next X position (delta from current).
  @param nextY The potential next Y position (delta from current).
]]
function RopePhysics.verletCollide(self, segmentIdx, nextX, nextY)
    local currentPosX = self.apx[segmentIdx]
    local currentPosY = self.apy[segmentIdx]

    local movementRay = Vector(nextX, nextY)
    
    -- Optimization: Skip collision check for very small movements.
    if movementRay:MagnitudeIsLessThan(0.01) then -- Reduced threshold
        self.apx[segmentIdx] = currentPosX + nextX
        self.apy[segmentIdx] = currentPosY + nextY
        return
    end

    local collisionPoint = Vector()   -- Stores the collision point if one occurs.
    local surfaceNormal = Vector()  -- Stores the normal of the collided surface.

    -- Cast a ray to detect obstacles (terrain and other MOs).
    -- Uses parent's ID to avoid self-collision with the firing actor.
    local collisionDist = SceneMan:CastObstacleRay(Vector(currentPosX, currentPosY), movementRay, 
                                                 collisionPoint, surfaceNormal, 
                                                 (self.parent and self.parent.ID or 0), 
                                                 self.Team, rte.airID, 0)

    if type(collisionDist) == "number" and collisionDist >= 0 and collisionDist <= movementRay.Magnitude then
        -- Collision detected.
        if surfaceNormal:MagnitudeIsGreaterThan(0.001) then
            surfaceNormal:SetMagnitude(1) -- Ensure normal is normalized.
        else
            surfaceNormal = Vector(0, -1) -- Default to an upward normal if it's invalid.
        end

        -- Move the point to the collision surface and nudge it slightly along the normal.
        self.apx[segmentIdx] = collisionPoint.X + surfaceNormal.X * NUDGE_DISTANCE
        self.apy[segmentIdx] = collisionPoint.Y + surfaceNormal.Y * NUDGE_DISTANCE
        
        -- Update the 'last' position to simulate a bounce, reducing phasing.
        self.lastX[segmentIdx] = self.apx[segmentIdx] - surfaceNormal.X * BOUNCE_STRENGTH
        self.lastY[segmentIdx] = self.apy[segmentIdx] - surfaceNormal.Y * BOUNCE_STRENGTH
        
        -- If an anchor point (player or hook end) collides, it should ideally stop completely against the surface.
        if segmentIdx == 0 or segmentIdx == self.currentSegments then
            self.lastX[segmentIdx] = self.apx[segmentIdx] -- Effectively zero velocity for next frame at this point.
            self.lastY[segmentIdx] = self.apy[segmentIdx]
        end
    else
        -- No collision, apply the full displacement.
        self.apx[segmentIdx] = currentPosX + nextX
        self.apy[segmentIdx] = currentPosY + nextY
    end
end

--[[
  Calculates the optimal number of segments based on the current rope length.
  Aims to balance visual fidelity with performance.
  @param self       The grapple instance.
  @param ropeLength The current length of the rope.
  @return           The optimal number of segments.
]]
function RopePhysics.calculateOptimalSegments(self, ropeLength)
    if ropeLength <= 0 then return self.minSegments end

    local baseSegments = math.ceil(ropeLength / self.segmentLength)
    
    -- Apply a scaling factor for very long ropes to use fewer segments per unit length.
    local scalingFactor = 1.0
    if ropeLength > 200 then -- Example threshold for when scaling starts
        -- Reduce segments more gradually for longer ropes.
        scalingFactor = 1.0 - math.min(0.3, (ropeLength - 200) / 800) -- Adjusted scaling
    end
    
    local desiredSegments = math.ceil(baseSegments * scalingFactor)
    
    return math.max(self.minSegments, math.min(desiredSegments, self.maxSegments))
end

--[[
  Determines the number of physics iterations.
  Currently fixed as per user request in original comments.
  @param self The grapple instance.
  @return     The number of physics iterations.
]]
function RopePhysics.optimizePhysicsIterations(self)
    return DEFAULT_PHYSICS_ITERATIONS
end

--[[
  Resizes the rope's segment arrays when the optimal number of segments changes.
  Interpolates positions for new segments to maintain a smooth transition.
  @param self     The grapple instance.
  @param newNumSegments The new total number of segments.
]]
function RopePhysics.resizeRopeSegments(self, newNumSegments)
    if newNumSegments == self.currentSegments then return end

    local oldNumSegments = self.currentSegments
    local tempOldAPX = {}
    local tempOldAPY = {}
    local tempOldLastX = {}
    local tempOldLastY = {}

    -- Store current segment positions and velocities
    for i = 0, oldNumSegments do
        tempOldAPX[i] = self.apx[i]
        tempOldAPY[i] = self.apy[i]
        tempOldLastX[i] = self.lastX[i]
        tempOldLastY[i] = self.lastY[i]
    end
    
    -- Initialize new arrays (or re-initialize if maxSegments was pre-allocated)
    -- self.apx, self.apy, self.lastX, self.lastY should already be tables up to maxSegments.

    -- Player anchor (segment 0)
    if self.parent and self.parent.Pos then
        self.apx[0] = self.parent.Pos.X
        self.apy[0] = self.parent.Pos.Y
        self.lastX[0] = self.parent.Pos.X - (self.parent.Vel.X or 0)
        self.lastY[0] = self.parent.Pos.Y - (self.parent.Vel.Y or 0)
    elseif tempOldAPX[0] then -- Fallback to old anchor if parent is briefly invalid
        self.apx[0] = tempOldAPX[0]
        self.apy[0] = tempOldAPY[0]
        self.lastX[0] = tempOldLastX[0]
        self.lastY[0] = tempOldLastY[0]
    end

    -- Hook anchor (segment newNumSegments)
    -- The hook's current position (self.Pos) is the primary source for the end anchor.
    self.apx[newNumSegments] = self.Pos.X
    self.apy[newNumSegments] = self.Pos.Y
    -- Estimate velocity for the hook end based on its last movement or current self.Vel
    local hookVelX = self.Vel and self.Vel.X or (tempOldAPX[oldNumSegments] and (tempOldAPX[oldNumSegments] - tempOldLastX[oldNumSegments])) or 0
    local hookVelY = self.Vel and self.Vel.Y or (tempOldAPY[oldNumSegments] and (tempOldAPY[oldNumSegments] - tempOldLastY[oldNumSegments])) or 0
    self.lastX[newNumSegments] = self.Pos.X - hookVelX
    self.lastY[newNumSegments] = self.Pos.Y - hookVelY
    
    -- Interpolate intermediate segments
    if newNumSegments > 1 then
        for i = 1, newNumSegments - 1 do
            local t = i / newNumSegments -- Ratio along the new rope length

            -- Find corresponding point(s) on the old rope structure for interpolation
            local old_t = t * oldNumSegments
            local old_idx_prev = math.floor(old_t)
            local old_idx_next = math.ceil(old_t)
            local interp_factor = old_t - old_idx_prev

            old_idx_prev = math.max(0, math.min(old_idx_prev, oldNumSegments))
            old_idx_next = math.max(0, math.min(old_idx_next, oldNumSegments))

            if tempOldAPX[old_idx_prev] and tempOldAPX[old_idx_next] then -- Ensure old indices are valid
                self.apx[i] = tempOldAPX[old_idx_prev] * (1 - interp_factor) + tempOldAPX[old_idx_next] * interp_factor
                self.apy[i] = tempOldAPY[old_idx_prev] * (1 - interp_factor) + tempOldAPY[old_idx_next] * interp_factor
                self.lastX[i] = tempOldLastX[old_idx_prev] * (1 - interp_factor) + tempOldLastX[old_idx_next] * interp_factor
                self.lastY[i] = tempOldLastY[old_idx_prev] * (1 - interp_factor) + tempOldLastY[old_idx_next] * interp_factor
            else 
                -- Fallback: linear interpolation between new start and end if old points are problematic
                local overall_t = i / newNumSegments
                self.apx[i] = self.apx[0] * (1 - overall_t) + self.apx[newNumSegments] * overall_t
                self.apy[i] = self.apy[0] * (1 - overall_t) + self.apy[newNumSegments] * overall_t
                self.lastX[i] = self.apx[i] -- Initialize with no velocity
                self.lastY[i] = self.apy[i]
            end
        end
    end
    
    self.currentSegments = newNumSegments
end


--[[
  Updates the rope physics using Verlet integration.
  @param grappleInstance The grapple instance.
  @param startPos        Position vector of the start anchor (player/gun).
  @param endPos          Position vector of the end anchor (hook).
  @param cableLength     Current maximum allowed length of the cable (physics length).
]]
function RopePhysics.updateRopePhysics(grappleInstance, startPos, endPos, cableLength)
    local segments = grappleInstance.currentSegments
    if segments < 1 or not grappleInstance.apx then return end -- Ensure segments and arrays are valid.

    -- Initialize lastX/Y for any new segments if not already done (e.g., after resize).
    for i = 0, segments do
        if grappleInstance.lastX[i] == nil then -- Check specifically for nil
            grappleInstance.lastX[i] = grappleInstance.apx[i] or startPos.X -- Fallback if apx[i] is also nil
            grappleInstance.lastY[i] = grappleInstance.apy[i] or startPos.Y
        end
    end

    -- Verlet integration for interior points (not the main anchors).
    -- Anchors (0 and segments) are handled separately.
    for i = 1, segments - 1 do
        if grappleInstance.apx[i] and grappleInstance.lastX[i] then -- Ensure points are valid
            local current_x = grappleInstance.apx[i]
            local current_y = grappleInstance.apy[i]
            local prev_x = grappleInstance.lastX[i]
            local prev_y = grappleInstance.lastY[i]

            local vel_x = current_x - prev_x
            local vel_y = current_y - prev_y

            grappleInstance.lastX[i] = current_x
            grappleInstance.lastY[i] = current_y

            -- Apply Verlet integration with gravity. No explicit dampening here for "rigid" feel.
            local next_integrated_x = current_x + vel_x
            local next_integrated_y = current_y + vel_y + GRAVITY_Y
            
            -- Perform collision detection for this segment's new position
            RopePhysics.verletCollide(grappleInstance, i, next_integrated_x - current_x, next_integrated_y - current_y)
        end
    end

    -- Update anchor positions (player and hook ends).
    -- Player anchor (segment 0)
    if startPos then
        grappleInstance.apx[0] = startPos.X
        grappleInstance.apy[0] = startPos.Y
        -- lastX/Y for player anchor are updated in Grapple.lua based on parent's velocity.
    end

    -- Hook anchor (segment 'segments')
    if endPos then
        if grappleInstance.actionMode == 1 then -- Flying hook
            -- For a flying hook, its own physics (self.Vel, self.Pos) dictate its movement.
            -- The end anchor point of the rope simply follows self.Pos.
            grappleInstance.apx[segments] = grappleInstance.Pos.X
            grappleInstance.apy[segments] = grappleInstance.Pos.Y
            grappleInstance.lastX[segments] = grappleInstance.Pos.X - (grappleInstance.Vel.X or 0)
            grappleInstance.lastY[segments] = grappleInstance.Pos.Y - (grappleInstance.Vel.Y or 0)
        elseif grappleInstance.actionMode == 2 then -- Hook stuck in terrain
            -- Position is fixed. Velocity is zero.
            grappleInstance.apx[segments] = grappleInstance.apx[segments] -- Should already be set
            grappleInstance.apy[segments] = grappleInstance.apy[segments]
            grappleInstance.lastX[segments] = grappleInstance.apx[segments]
            grappleInstance.lastY[segments] = grappleInstance.apy[segments]
        elseif grappleInstance.actionMode == 3 and grappleInstance.target and grappleInstance.target.ID ~= rte.NoMOID then -- Hook on MO
            local effective_target = RopeStateManager.getEffectiveTarget(grappleInstance)
            if effective_target and effective_target.Pos and effective_target.Vel then
                grappleInstance.apx[segments] = effective_target.Pos.X
                grappleInstance.apy[segments] = effective_target.Pos.Y
                grappleInstance.lastX[segments] = effective_target.Pos.X - (effective_target.Vel.X or 0)
                grappleInstance.lastY[segments] = effective_target.Pos.Y - (effective_target.Vel.Y or 0)
            else
                 -- Fallback if target becomes invalid, keep last known position
                grappleInstance.lastX[segments] = grappleInstance.apx[segments]
                grappleInstance.lastY[segments] = grappleInstance.apy[segments]
            end
        else -- Default or unknown state, try to hold position
            if grappleInstance.apx[segments] then
                grappleInstance.lastX[segments] = grappleInstance.apx[segments]
                grappleInstance.lastY[segments] = grappleInstance.apy[segments]
            end
        end
    end
end


--[[
  Applies constraints to the rope segments to maintain their lengths and overall rope length.
  This is the core of the rigid rope behavior.
  @param grappleInstance        The grapple instance.
  @param currentPhysicsLength   The target physics length of the rope.
  @return                       True if the rope should break due to extreme stretch, false otherwise.
]]
function RopePhysics.applyRopeConstraints(grappleInstance, currentPhysicsLength)
    local segments = grappleInstance.currentSegments
    if segments == 0 or not grappleInstance.apx or not grappleInstance.parent then return false end

    local maxAllowedRopeLength = currentPhysicsLength -- This is the length the rope tries to adhere to.
    
    -- Ensure anchor points are up-to-date before constraint solving.
    -- Player anchor:
    grappleInstance.apx[0] = grappleInstance.parent.Pos.X
    grappleInstance.apy[0] = grappleInstance.parent.Pos.Y
    -- Hook anchor is updated based on its state (flying, terrain, MO) in updateRopePhysics or Grapple.lua

    -- Store current tension as a ratio for feedback/other systems.
    -- This will be updated after constraints.
    grappleInstance.currentTension = 0 

    -- Iteratively satisfy segment length constraints.
    local targetSegmentLength = maxAllowedRopeLength / math.max(1, segments)
    local iterations = RopePhysics.optimizePhysicsIterations(grappleInstance)

    for iter = 1, iterations do
        -- First, constrain the overall length between the two main anchors (player and hook).
        -- This helps prevent the whole rope from overstretching significantly.
        local p_start_x, p_start_y = grappleInstance.apx[0], grappleInstance.apy[0]
        local p_end_x, p_end_y = grappleInstance.apx[segments], grappleInstance.apy[segments]
        
        local dx_total = p_end_x - p_start_x
        local dy_total = p_end_y - p_start_y
        local dist_total = math.sqrt(dx_total*dx_total + dy_total*dy_total)

        if dist_total > maxAllowedRopeLength and dist_total > 0.001 then
            local diff_total = maxAllowedRopeLength - dist_total
            local percent_total = (diff_total / dist_total) * CONSTRAINT_STRENGTH * 0.5 -- Apply half to each end's controller

            -- Determine how to apply correction based on actionMode
            if grappleInstance.actionMode == 2 then -- Hook on terrain, player swings
                -- Correct player position and velocity (primary correction)
                local vec_from_hook_to_player = Vector(p_start_x - p_end_x, p_start_y - p_end_y)
                local correctedPlayerPos = Vector(p_end_x, p_end_y) + vec_from_hook_to_player:SetMagnitude(maxAllowedRopeLength)
                
                grappleInstance.parent.Pos = correctedPlayerPos
                grappleInstance.apx[0] = correctedPlayerPos.X
                grappleInstance.apy[0] = correctedPlayerPos.Y
                
                -- Correct player velocity to be tangential
                local ropeDirFromPlayerToHook = (Vector(p_end_x, p_end_y) - correctedPlayerPos):SetMagnitude(1)
                local radialVelScalar = grappleInstance.parent.Vel:Dot(ropeDirFromPlayerToHook)
                grappleInstance.parent.Vel = grappleInstance.parent.Vel - (ropeDirFromPlayerToHook * radialVelScalar)
                
                -- Store tension feedback
                if -radialVelScalar > 0.01 then
                    grappleInstance.ropeTensionForce = -radialVelScalar * 0.5 -- Simplified tension magnitude
                    grappleInstance.ropeTensionDirection = ropeDirFromPlayerToHook
                else
                    grappleInstance.ropeTensionForce = nil
                end

            elseif grappleInstance.actionMode == 1 or grappleInstance.actionMode == 3 then -- Hook flying or on MO, player is "fixed" anchor
                -- Correct hook position
                grappleInstance.apx[segments] = p_end_x + dx_total * percent_total
                grappleInstance.apy[segments] = p_end_y + dy_total * percent_total
                -- Also update the grapple MO's actual position if it's the one being moved
                if grappleInstance.actionMode == 1 then -- Flying hook's position is its anchor
                    grappleInstance.Pos.X = grappleInstance.apx[segments]
                    grappleInstance.Pos.Y = grappleInstance.apy[segments]
                end
                 grappleInstance.ropeTensionForce = nil -- No direct tension feedback to player in this case from this global constraint
            end
        else
             grappleInstance.ropeTensionForce = nil -- No global overstretch
        end


        -- Then, iterate through individual segments.
        for i = 0, segments - 1 do
            local p1_idx, p2_idx = i, i + 1
            local x1, y1 = grappleInstance.apx[p1_idx], grappleInstance.apy[p1_idx]
            local x2, y2 = grappleInstance.apx[p2_idx], grappleInstance.apy[p2_idx]
            
            local dx_seg = x2 - x1
            local dy_seg = y2 - y1
            local dist_seg = math.sqrt(dx_seg*dx_seg + dy_seg*dy_seg)
            
            if dist_seg > targetSegmentLength and dist_seg > 0.001 then -- Only correct if overstretched
                local diff_seg = targetSegmentLength - dist_seg
                local percent_seg = (diff_seg / dist_seg) * CONSTRAINT_STRENGTH * 0.5 -- 0.5 because applied to two points

                local offsetX = dx_seg * percent_seg
                local offsetY = dy_seg * percent_seg

                local p1_is_player_anchor = (p1_idx == 0)
                local p2_is_hook_anchor = (p2_idx == segments)
                
                if not p1_is_player_anchor then
                    grappleInstance.apx[p1_idx] = x1 - offsetX
                    grappleInstance.apy[p1_idx] = y1 - offsetY
                end
                if not p2_is_hook_anchor then
                    grappleInstance.apx[p2_idx] = x2 + offsetX
                    grappleInstance.apy[p2_idx] = y2 + offsetY
                end

                -- If one end is an anchor, the other point takes full correction.
                if p1_is_player_anchor and not p2_is_hook_anchor then
                    grappleInstance.apx[p2_idx] = grappleInstance.apx[p2_idx] + offsetX -- Additional correction for p2
                    grappleInstance.apy[p2_idx] = grappleInstance.apy[p2_idx] + offsetY
                elseif p2_is_hook_anchor and not p1_is_player_anchor then
                    grappleInstance.apx[p1_idx] = grappleInstance.apx[p1_idx] - offsetX -- Additional correction for p1
                    grappleInstance.apy[p1_idx] = grappleInstance.apy[p1_idx] - offsetY
                end
            end
        end
    end

    -- Calculate final actual rope length and check for breaking condition.
    local finalRopeVisualLength = 0
    for i = 0, segments - 1 do
        local dx = grappleInstance.apx[i+1] - grappleInstance.apx[i]
        local dy = grappleInstance.apy[i+1] - grappleInstance.apy[i]
        finalRopeVisualLength = finalRopeVisualLength + math.sqrt(dx*dx + dy*dy)
    end
    grappleInstance.actualRopeLength = finalRopeVisualLength -- For debug/renderer

    -- Update tension based on final visual length vs physics target length
    if maxAllowedRopeLength > 0 then
        grappleInstance.currentTension = math.max(0, (finalRopeVisualLength - maxAllowedRopeLength) / maxAllowedRopeLength)
    else
        grappleInstance.currentTension = 0
    end
    
    -- Rope breaking condition: Extremely high stretch (e.g., 5x target length).
    if maxAllowedRopeLength > 0 and finalRopeVisualLength > maxAllowedRopeLength * 5.0 then
        grappleInstance.shouldBreak = true -- Signal to Grapple.lua
        return true
    end

    return false -- Rope did not break.
end

--[[
  Smooths the rope visually using weighted averaging.
  Applied sparingly to avoid significantly altering physics.
  @param grappleInstance The grapple instance.
]]
function RopePhysics.smoothRope(grappleInstance)
    local segments = grappleInstance.currentSegments
    if segments < 3 or not grappleInstance.apx then return end -- Need at least 3 points (2 segments) to smooth.
    
    local smoothing_strength = 0.05 -- Very light smoothing.
    
    local smoothedX, smoothedY = {}, {}
    for i = 0, segments do -- Copy current points.
        smoothedX[i] = grappleInstance.apx[i]
        smoothedY[i] = grappleInstance.apy[i]
    end
    
    -- Apply smoothing to intermediate points only.
    for i = 1, segments - 1 do
        local avgX = (grappleInstance.apx[i-1] + grappleInstance.apx[i] + grappleInstance.apx[i+1]) / 3
        local avgY = (grappleInstance.apy[i-1] + grappleInstance.apy[i] + grappleInstance.apy[i+1]) / 3
        
        smoothedX[i] = grappleInstance.apx[i] * (1 - smoothing_strength) + avgX * smoothing_strength
        smoothedY[i] = grappleInstance.apy[i] * (1 - smoothing_strength) + avgY * smoothing_strength
    end
    
    -- Apply smoothed positions back (excluding anchors, which are controlled).
    for i = 1, segments - 1 do
        grappleInstance.apx[i] = smoothedX[i]
        grappleInstance.apy[i] = smoothedY[i]
    end
end

-- Placeholder for actor protection logic if direct forces were to be applied.
-- In a pure constraint system, this is less critical as positions are directly managed.
function RopePhysics.calculateActorProtection(grappleInstance, force_magnitude, force_direction)
    -- This function would limit forces if the system used AddForce extensively.
    -- For now, it's a conceptual placeholder.
    local safe_force_magnitude = math.min(force_magnitude, 5.0) -- Example hard cap.
    return safe_force_magnitude, force_direction * safe_force_magnitude
end


-- The following functions (handleRopePull, handleRopeExtend, checkRopeBreak) seem
-- to be remnants of a previous force-based system or conceptual helpers.
-- In the current Verlet + constraint model, their roles are largely superseded
-- by the input controller (for desired length changes) and applyRopeConstraints.
-- They are kept here for context or if parts of their logic are to be repurposed.

function RopePhysics.handleRopePull(grappleInstance, controller, terrCheck)
    -- Logic for player pulling on the rope would typically adjust 'currentLineLength'
    -- which is then enforced by applyRopeConstraints.
    -- Direct force application here would conflict with the constraint system.
end

function RopePhysics.handleRopeExtend(grappleInstance)
    -- Similar to handleRopePull, extending the rope involves changing 'currentLineLength'.
end

function RopePhysics.checkRopeBreak(grappleInstance)
    -- The primary rope breaking logic is now within applyRopeConstraints,
    -- based on excessive stretch beyond a high threshold.
    -- This function could be used for alternative breaking conditions if needed.
    -- Example: if grappleInstance.lineStrength is exceeded by some calculated tension.
    -- However, current breaking is purely stretch-based.
end

return RopePhysics
