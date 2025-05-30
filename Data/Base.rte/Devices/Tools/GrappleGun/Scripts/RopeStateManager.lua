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

-- Handle exceeding maximum length - SIMPLIFIED VERSION
-- Main length control is now centralized in Grapple.lua
function RopeStateManager.checkLengthLimit(grappleInstance)
    if grappleInstance.lineLength > grappleInstance.maxLineLength then
        if grappleInstance.limitReached == false then
            grappleInstance.limitReached = true
            grappleInstance.clickSound:Play(grappleInstance.parent.Pos)
        end
        return true -- Signal that limit was reached
    end
    
    grappleInstance.limitReached = false
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

-- Apply sophisticated terrain pull physics with comprehensive actor protection
function RopeStateManager.applyTerrainPullPhysics(grappleInstance)
    if grappleInstance.actionMode ~= 2 then return false end
    
    -- Check if we have rope tension from the constraint system
    if grappleInstance.ropeTensionForce and grappleInstance.ropeTensionDirection then
        -- Use the tension force calculated by the rope constraint system
        local raw_spring_force = grappleInstance.ropeTensionForce
        local force_direction = grappleInstance.ropeTensionDirection
        
        -- Apply sophisticated actor protection
        local actor = grappleInstance.parent
        local actor_mass = actor.Mass
        local actor_vel = actor.Vel.Magnitude
        local actor_health = actor.Health
        
        -- Base safety limits
        local base_force_limit = 6.0 -- Conservative limit for terrain pulls
        local mass_scaling = math.min(actor_mass / 80, 1.8)
        local velocity_penalty = 1 + math.min(actor_vel / 15, 1.0)
        local health_scaling = math.min(actor_health / 100, 1.1)
        
        local safe_force_limit = base_force_limit * mass_scaling * health_scaling / velocity_penalty
        
        -- Progressive force dampening with multiple stages
        local force_dampening = 1.0
        if raw_spring_force > safe_force_limit then
            local excess_ratio = raw_spring_force / safe_force_limit
            if excess_ratio < 2.0 then
                -- Linear dampening for moderate excess
                force_dampening = 1.0 / excess_ratio
            else
                -- Logarithmic dampening for extreme forces
                force_dampening = 1.0 / (1 + math.log(excess_ratio))
            end
        end
        
        -- Energy conservation check
        local kinetic_energy = 0.5 * actor_mass * actor_vel * actor_vel
        local rope_potential_energy = raw_spring_force * (raw_spring_force / 10) -- Approximation
        local total_energy = kinetic_energy + rope_potential_energy
        
        local energy_limit = 1500 -- Energy threshold
        if total_energy > energy_limit then
            local energy_dampening = energy_limit / total_energy
            force_dampening = force_dampening * energy_dampening
        end
        
        -- Calculate final safe force
        local safe_force_magnitude = raw_spring_force * force_dampening
        local safe_force_vector = force_direction * safe_force_magnitude
        
        -- Apply primary force to pull player toward hook when rope is taut
        if safe_force_magnitude > 0.1 then
            actor:AddForce(safe_force_vector, actor.Pos)
        end
        
        return false -- Don't break rope from tension
    end
    
    -- Fallback to old system if no tension force available
    local minRopeLength = 1
    local effectiveCurrentLength = math.max(minRopeLength, grappleInstance.currentLineLength)
    
    if grappleInstance.lineLength > effectiveCurrentLength then
        -- Calculate extension and forces
        local extension = grappleInstance.lineLength - effectiveCurrentLength
        local base_spring_constant = 0.5 -- Reduced for safety
        
        -- Dynamic force calculation based on extension ratio
        local extension_ratio = extension / effectiveCurrentLength
        local dynamic_spring_constant = base_spring_constant * (1 + extension_ratio * 0.3)
        
        -- Calculate raw spring force
        local raw_spring_force = extension * dynamic_spring_constant
        
        -- Apply sophisticated actor protection
        local force_direction = grappleInstance.lineVec:SetMagnitude(1)
        
        -- Multi-layered safety system
        local actor = grappleInstance.parent
        local actor_mass = actor.Mass
        local actor_vel = actor.Vel.Magnitude
        local actor_health = actor.Health
        
        -- Base safety limits
        local base_force_limit = 6.0 -- Conservative limit for terrain pulls
        local mass_scaling = math.min(actor_mass / 80, 1.8)
        local velocity_penalty = 1 + math.min(actor_vel / 15, 1.0)
        local health_scaling = math.min(actor_health / 100, 1.1)
        
        local safe_force_limit = base_force_limit * mass_scaling * health_scaling / velocity_penalty
        
        -- Progressive force dampening with multiple stages
        local force_dampening = 1.0
        if raw_spring_force > safe_force_limit then
            local excess_ratio = raw_spring_force / safe_force_limit
            if excess_ratio < 2.0 then
                -- Linear dampening for moderate excess
                force_dampening = 1.0 / excess_ratio
            else
                -- Logarithmic dampening for extreme forces
                force_dampening = 1.0 / (1 + math.log(excess_ratio))
            end
        end
        
        -- Energy conservation check
        local kinetic_energy = 0.5 * actor_mass * actor_vel * actor_vel
        local rope_potential_energy = raw_spring_force * extension
        local total_energy = kinetic_energy + rope_potential_energy
        
        local energy_limit = 1500 -- Energy threshold
        if total_energy > energy_limit then
            local energy_dampening = energy_limit / total_energy
            force_dampening = force_dampening * energy_dampening
        end
        
        -- Calculate final safe force
        local safe_force_magnitude = raw_spring_force * force_dampening
        local safe_force_vector = force_direction * safe_force_magnitude
        
        -- Force distribution over time for very high forces
        if raw_spring_force > safe_force_limit * 3 then
            -- Store excess force for gradual application
            if not grappleInstance.terrainForceBuffer then
                grappleInstance.terrainForceBuffer = {force = 0, decay = 0.85}
            end
            
            local excess_force = raw_spring_force - safe_force_magnitude
            grappleInstance.terrainForceBuffer.force = grappleInstance.terrainForceBuffer.force + excess_force * 0.2
        end
        
        -- Apply primary force
        if safe_force_magnitude > 0.1 then
            actor:AddForce(safe_force_vector, actor.Pos)
        end
        
        -- Apply buffered forces if they exist
        if grappleInstance.terrainForceBuffer and grappleInstance.terrainForceBuffer.force > 0.1 then
            local buffer_force = grappleInstance.terrainForceBuffer.force * 0.25 -- Apply 25% per frame
            local buffered_force_vector = force_direction * buffer_force
            
            -- Additional safety check for buffered forces
            if buffer_force < safe_force_limit * 0.8 then
                actor:AddForce(buffered_force_vector, actor.Pos)
            end
            
            -- Decay the buffered force
            grappleInstance.terrainForceBuffer.force = grappleInstance.terrainForceBuffer.force * grappleInstance.terrainForceBuffer.decay
        end
        
        -- Rope breaking with sophisticated criteria
        local break_threshold = (grappleInstance.lineStrength or 50) * 0.9
        
        -- Track sustained high forces
        if not grappleInstance.terrainForceHistory then
            grappleInstance.terrainForceHistory = {}
            for i = 1, 8 do
                grappleInstance.terrainForceHistory[i] = 0
            end
        end
        
        table.remove(grappleInstance.terrainForceHistory, 1)
        table.insert(grappleInstance.terrainForceHistory, raw_spring_force)
        
        local avg_force = 0
        for i = 1, #grappleInstance.terrainForceHistory do
            avg_force = avg_force + grappleInstance.terrainForceHistory[i]
        end
        avg_force = avg_force / #grappleInstance.terrainForceHistory
        
        -- Break rope if sustained high force or extreme instantaneous force
        if (avg_force > break_threshold * 0.7 and raw_spring_force > break_threshold) or 
           raw_spring_force > break_threshold * 2 then
            return true -- Signal to delete the hook due to excessive tension
        end
    end
    
    return false
end

-- Apply sophisticated MO pull physics with comprehensive force protection for both actor and target
function RopeStateManager.applyMOPullPhysics(grappleInstance)
    if grappleInstance.actionMode ~= 3 or not grappleInstance.target then return false end
    
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

        local target = grappleInstance.target
        -- Simplified root parent check without IsAttachable since it's not available
        if target.ID ~= target.RootID then
            local mo = target:GetRootParent()
            if mo.ID ~= rte.NoMOID then
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
            -- Check if we have rope tension from the constraint system
            if grappleInstance.ropeTensionForce and grappleInstance.ropeTensionDirection then
                -- Use the tension force calculated by the rope constraint system
                local raw_spring_force = grappleInstance.ropeTensionForce
                local actor = grappleInstance.parent
                local actor_mass = actor.Mass
                local actor_vel = actor.Vel.Magnitude
                local actor_health = actor.Health
                
                local target_mass = target.Mass
                local target_vel = target.Vel.Magnitude
                
                -- Dynamic force calculation with mass ratio considerations
                local mass_ratio = actor_mass / (actor_mass + target_mass)
                
                -- Multi-tier actor protection system
                local actor_base_limit = 5.0 -- Conservative limit for MO pulls
                local actor_mass_scaling = math.min(actor_mass / 70, 1.6)
                local actor_velocity_penalty = 1 + math.min(actor_vel / 12, 0.8)
                local actor_health_scaling = math.min(actor_health / 100, 1.05)
                
                local actor_safe_limit = actor_base_limit * actor_mass_scaling * actor_health_scaling / actor_velocity_penalty
                
                -- Actor force protection
                local actor_force_dampening = 1.0
                if raw_spring_force > actor_safe_limit then
                    local excess_ratio = raw_spring_force / actor_safe_limit
                    if excess_ratio < 1.5 then
                        actor_force_dampening = 1.0 / excess_ratio
                    else
                        actor_force_dampening = 1.0 / (1 + math.log(excess_ratio * 0.5))
                    end
                end
                
                -- Calculate safe actor force using tension direction
                local actor_safe_force = raw_spring_force * actor_force_dampening * mass_ratio
                local actor_force_vector = grappleInstance.ropeTensionDirection * actor_safe_force
                
                -- Target force protection (less strict than actor)
                local target_force_limit = 25.0 -- Targets can handle more force
                local target_force_scaling = math.min(1.0, target_force_limit / raw_spring_force)
                local target_safe_force = raw_spring_force * target_force_scaling * (1 - mass_ratio)
                local target_force_vector = grappleInstance.ropeTensionDirection * target_safe_force
                
                -- Apply forces when rope is taut
                if actor_safe_force > 0.1 then
                    actor:AddForce(actor_force_vector, actor.Pos)
                end
                
                if target_safe_force > 0.1 then
                    target:AddForce(-target_force_vector, target.Pos)
                end
            else
                -- Fallback to old spring system if no tension force available
                local minRopeLength = 1
                local effectiveCurrentLength = math.max(minRopeLength, grappleInstance.currentLineLength)
                
                if grappleInstance.lineLength > effectiveCurrentLength then
                    local extension = grappleInstance.lineLength - effectiveCurrentLength
                    
                    -- Calculate sophisticated force distribution
                    local actor = grappleInstance.parent
                    local actor_mass = actor.Mass
                    local actor_vel = actor.Vel.Magnitude
                    local actor_health = actor.Health
                    
                    local target_mass = target.Mass
                    local target_vel = target.Vel.Magnitude
                    
                    -- Dynamic force calculation with mass ratio considerations
                local mass_ratio = actor_mass / (actor_mass + target_mass)
                local base_spring_constant = 0.4 -- Conservative for MO interactions
                
                -- Adjust spring constant based on mass distribution
                local dynamic_spring_constant = base_spring_constant * (1 + math.abs(mass_ratio - 0.5))
                
                local raw_spring_force = extension * dynamic_spring_constant
                
                -- Multi-tier actor protection system
                local actor_base_limit = 5.0 -- Conservative limit for MO pulls
                local actor_mass_scaling = math.min(actor_mass / 70, 1.6)
                local actor_velocity_penalty = 1 + math.min(actor_vel / 12, 0.8)
                local actor_health_scaling = math.min(actor_health / 100, 1.05)
                
                local actor_safe_limit = actor_base_limit * actor_mass_scaling * actor_health_scaling / actor_velocity_penalty
                
                -- Actor force protection
                local actor_force_dampening = 1.0
                if raw_spring_force > actor_safe_limit then
                    local excess_ratio = raw_spring_force / actor_safe_limit
                    if excess_ratio < 1.5 then
                        actor_force_dampening = 1.0 / excess_ratio
                    else
                        actor_force_dampening = 1.0 / (1 + math.log(excess_ratio * 0.5))
                    end
                end
                
                -- Energy-based safety for actor
                local actor_kinetic_energy = 0.5 * actor_mass * actor_vel * actor_vel
                local actor_potential_energy = raw_spring_force * extension * mass_ratio
                local actor_total_energy = actor_kinetic_energy + actor_potential_energy
                
                local actor_energy_limit = 1200
                if actor_total_energy > actor_energy_limit then
                    local actor_energy_dampening = actor_energy_limit / actor_total_energy
                    actor_force_dampening = actor_force_dampening * actor_energy_dampening
                end
                
                -- Calculate safe actor force
                local actor_safe_force = raw_spring_force * actor_force_dampening * mass_ratio
                local actor_force_vector = grappleInstance.lineVec:SetMagnitude(actor_safe_force)
                
                -- Target force protection (less strict than actor)
                local target_force_limit = 25.0 -- Targets can handle more force
                local target_force_scaling = math.min(1.0, target_force_limit / raw_spring_force)
                local target_safe_force = raw_spring_force * target_force_scaling * (1 - mass_ratio)
                local target_force_vector = grappleInstance.lineVec:SetMagnitude(target_safe_force)
                
                -- Force distribution over time for extreme forces
                if raw_spring_force > actor_safe_limit * 2.5 then
                    if not grappleInstance.moForceBuffer then
                        grappleInstance.moForceBuffer = {
                            actorForce = 0, 
                            targetForce = 0, 
                            decay = 0.88
                        }
                    end
                    
                    local excess_actor_force = raw_spring_force - actor_safe_force
                    local excess_target_force = raw_spring_force - target_safe_force
                    
                    grappleInstance.moForceBuffer.actorForce = grappleInstance.moForceBuffer.actorForce + excess_actor_force * 0.15
                    grappleInstance.moForceBuffer.targetForce = grappleInstance.moForceBuffer.targetForce + excess_target_force * 0.15
                end
                
                -- Apply primary forces
                if actor_safe_force > 0.1 then
                    actor:AddForce(actor_force_vector, actor.Pos)
                end
                
                if target_safe_force > 0.1 then
                    target:AddForce(-target_force_vector, target.Pos)
                end
                
                -- Apply buffered forces gradually
                if grappleInstance.moForceBuffer then
                    local buffer = grappleInstance.moForceBuffer
                    
                    if buffer.actorForce > 0.1 then
                        local buffered_actor_force = buffer.actorForce * 0.2
                        if buffered_actor_force < actor_safe_limit * 0.6 then
                            local buffered_actor_vector = grappleInstance.lineVec:SetMagnitude(buffered_actor_force)
                            actor:AddForce(buffered_actor_vector, actor.Pos)
                        end
                        buffer.actorForce = buffer.actorForce * buffer.decay
                    end
                    
                    if buffer.targetForce > 0.1 then
                        local buffered_target_force = buffer.targetForce * 0.2
                        local buffered_target_vector = grappleInstance.lineVec:SetMagnitude(buffered_target_force)
                        target:AddForce(-buffered_target_vector, target.Pos)
                        buffer.targetForce = buffer.targetForce * buffer.decay
                    end
                end
                
                -- Enhanced rope breaking criteria for MO interactions
                local break_threshold = (grappleInstance.lineStrength or 50) * 0.85
                
                -- Track force history for MO interactions
                if not grappleInstance.moForceHistory then
                    grappleInstance.moForceHistory = {}
                    for i = 1, 6 do
                        grappleInstance.moForceHistory[i] = 0
                    end
                end
                
                table.remove(grappleInstance.moForceHistory, 1)
                table.insert(grappleInstance.moForceHistory, raw_spring_force)
                
                local avg_mo_force = 0
                for i = 1, #grappleInstance.moForceHistory do
                    avg_mo_force = avg_mo_force + grappleInstance.moForceHistory[i]
                end
                avg_mo_force = avg_mo_force / #grappleInstance.moForceHistory
                
                -- Break rope if forces are too extreme for MO interaction
                if (avg_mo_force > break_threshold * 0.6 and raw_spring_force > break_threshold) or 
                   raw_spring_force > break_threshold * 1.8 then
                    return true -- Signal to delete the hook due to excessive force
                end
                
                -- Add dampening for smoother motion
                target.Vel = target.Vel * 0.985
                target.AngularVel = target.AngularVel * 0.995
                end
            end
        end
    else
        -- Our MO has been destroyed, return hook
        return true -- Signal to delete the hook
    end
    
    return false
end

-- Determine if the grapple can be released based on its current state
function RopeStateManager.canReleaseGrapple(grappleInstance)
    -- Check if the grapple is in a state where it can be released
    -- For now just return the canRelease property, but this could be expanded
    -- with additional logic in the future if needed
    return grappleInstance.canRelease
end

return RopeStateManager
