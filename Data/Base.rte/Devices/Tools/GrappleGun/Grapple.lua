---@diagnostic disable: undefined-global
-- filepath: /home/cretin/git/Cortex-Command-Community-Project/Data/Base.rte/Devices/Tools/GrappleGun/Grapple.lua
-- Main logic for the grapple claw MovableObject.

-- Load Modules
local RopePhysics = require("Devices.Tools.GrappleGun.Scripts.RopePhysics")
local RopeRenderer = require("Devices.Tools.GrappleGun.Scripts.RopeRenderer")
local RopeInputController = require("Devices.Tools.GrappleGun.Scripts.RopeInputController")
local RopeStateManager = require("Devices.Tools.GrappleGun.Scripts.RopeStateManager")

function Create(self)
    self.lastPos = self.Pos
    
    self.mapWrapsX = SceneMan.SceneWrapsX
    self.climbTimer = Timer()
    self.mouseClimbTimer = Timer()
    self.tapTimer = Timer() -- Initialize tapTimer
    
    -- Initialize state using the state manager. This sets self.actionMode = 0.
    RopeStateManager.initState(self)

    -- self.initializationOk = true -- This flag is effectively replaced by checking self.actionMode == 0 in Update.

    -- Core grapple properties
    self.fireVel = 40      -- Initial velocity of the hook. Overwrites .ini FireVel. Crucial for HDFirearm.
    self.maxLineLength = 600   -- Maximum allowed length of the rope.
    self.maxShootDistance = self.maxLineLength * 0.95 -- Hook will detach if it travels further than this before sticking.
    self.setLineLength = 0 -- Target length set by input/logic.
    self.lineStrength = 10000 -- Force threshold for breaking (effectively unbreakable).

    self.limitReached = false -- True if the rope has reached its maxLineLength.
    self.stretchMode = false   -- Disabled for rigid rope behavior.
    self.stretchPullRatio = 0.0 -- No stretching for rigid rope.
    self.pieSelection = 0  -- Current pie menu selection (0: none, 1: full retract, etc.).

    -- Timing and interval properties for rope actions
    self.climbDelay = 8    -- Delay between climb ticks.
    self.tapTime = 150     -- Max time between taps for double-tap unhook.
    self.tapAmount = 2     -- Number of taps required for unhook.
    self.mouseClimbLength = 200    -- Duration mouse scroll input is considered active.
    self.climbInterval = 4.0       -- Amount rope length changes per climb tick.
    self.autoClimbIntervalA = 5.0  -- Auto-retract speed (primary).
    self.autoClimbIntervalB = 3.0  -- Auto-extend speed (secondary, e.g., from pie menu).

    -- Sound effects
    self.stickSound = CreateSoundContainer("Grapple Gun Claw Stick", "Base.rte")
    self.clickSound = CreateSoundContainer("Grapple Gun Click", "Base.rte")
    self.returnSound = CreateSoundContainer("Grapple Gun Return", "Base.rte")
    self.crankSoundInstance = nil

    -- Rope physics variables
    self.currentLineLength = 0 
    self.cablespring = 0.01 
    
    self.minSegments = 1
    self.maxSegments = 1000
    self.segmentLength = 6
    self.currentSegments = self.minSegments
    
    self.shiftScrollSpeed = 1.0

    self.apx = {} 
    self.apy = {} 
    self.lastX = {}
    self.lastY = {}
    
    local px = self.Pos.X
    local py = self.Pos.Y
    
    for i = 0, self.maxSegments do
        self.apx[i] = px
        self.apy[i] = py
        self.lastX[i] = px
        self.lastY[i] = py
    end
    
    self.currentSegments = self.minSegments

    -- Parent gun, parent actor, and related properties (Vel, anchor points, parentRadius)
    -- will be determined and set in the first Update call.
    -- No self.ToDelete = true will be set in Create.
end

function Update(self)
    if self.ToDelete then return end -- Already marked for deletion from a previous frame or early in this one.

    -- First-time setup: Find parent, initialize velocity, anchor points, etc.
    if self.actionMode == 0 then
        local foundAndValidParent = false
        for gun_mo in MovableMan:GetMOsInRadius(self.Pos, 75) do
            if gun_mo and gun_mo.ClassName == "HDFirearm" and gun_mo.PresetName == "Grapple Gun" then
                local hdfGun = ToHDFirearm(gun_mo)
                if hdfGun and SceneMan:ShortestDistance(self.Pos, hdfGun.MuzzlePos, self.mapWrapsX):MagnitudeIsLessThan(20) then
                    self.parentGun = hdfGun
                    local rootParentMO = MovableMan:GetMOFromID(hdfGun.RootID)
                    if rootParentMO then
                        if MovableMan:IsActor(rootParentMO) then
                            self.parent = ToActor(rootParentMO) -- Store as Actor type

                            -- Initialize player anchor point (segment 0)
                            self.apx[0] = self.parent.Pos.X
                            self.apy[0] = self.parent.Pos.Y
                            self.lastX[0] = self.parent.Pos.X - (self.parent.Vel.X or 0)
                            self.lastY[0] = self.parent.Pos.Y - (self.parent.Vel.Y or 0)

                            -- Set initial velocity of the hook based on parent's aim and velocity
                            local aimAngle = self.parent:GetAimAngle(true)
                            self.Vel = (self.parent.Vel or Vector(0,0)) + Vector(self.fireVel, 0):RadRotate(aimAngle)
                            
                            -- Initialize hook's lastX/Y for its initial trajectory
                            self.lastX[self.currentSegments] = self.Pos.X - self.Vel.X 
                            self.lastY[self.currentSegments] = self.Pos.Y - self.Vel.Y

                            if self.parentGun then -- Should be valid here
                                self.parentGun:RemoveNumberValue("GrappleMode") -- Clear any previous mode
                            end

                            -- Determine parent's effective radius for terrain checks
                            self.parentRadius = 5 -- Default radius
                            if self.parent.Attachables and type(self.parent.Attachables) == "table" then
                                for _, part in ipairs(self.parent.Attachables) do
                                    if part and part.Pos and part.Radius then
                                        local radcheck = SceneMan:ShortestDistance(self.parent.Pos, part.Pos, self.mapWrapsX).Magnitude + part.Radius
                                        if self.parentRadius == nil or radcheck > self.parentRadius then
                                            self.parentRadius = radcheck
                                        end
                                    end
                                end
                            end
                            self.actionMode = 1 -- Set to flying, initialization successful
                            foundAndValidParent = true
                        end -- if MovableMan:IsActor(rootParentMO)
                    end -- if rootParentMO
                    break -- Found our gun, processed it.
                end -- if hdfGun and distance check
            end -- if gun_mo is grapple gun
        end -- for gun_mo

        if not foundAndValidParent then
            self.ToDelete = true
            return -- Exit Update if initialization failed
        end
        -- If we reach here, initialization was successful, self.actionMode = 1
    end

    -- If ToDelete was set during initialization, or by other logic, exit.
    if self.ToDelete then return end

    -- Continuous validation checks for parent and gun
    -- self.parent should be an Actor if initialization succeeded and actionMode >= 1
    if not self.parent or self.parent.ID == rte.NoMOID then
        self.ToDelete = true
        return
    end
    
    local parentActor = self.parent -- self.parent is already an Actor type from the setup block
    
    if not self.parentGun or self.parentGun.ID == rte.NoMOID or not parentActor:HasObject("Grapple Gun") then
        self.ToDelete = true
        return
    end

    local controller = parentActor:GetController()
    if not controller then
        self.ToDelete = true
        return
    end
    local player = controller.Player or 0

    -- Standard update flags
    self.ToSettle = false -- Grapple claw should not settle

    -- Update player anchor point (segment 0)
    self.apx[0] = parentActor.Pos.X
    self.apy[0] = parentActor.Pos.Y
    self.lastX[0] = parentActor.Pos.X - (parentActor.Vel.X or 0)
    self.lastY[0] = parentActor.Pos.Y - (parentActor.Vel.Y or 0)

    -- Update hook anchor point (segment self.currentSegments)
    -- This depends on whether the hook is attached or flying
    if self.actionMode == 1 then -- Flying
        -- Hook position is determined by its own physics
        self.apx[self.currentSegments] = self.Pos.X
        self.apy[self.currentSegments] = self.Pos.Y
        -- lastX/Y for the hook end are updated by its own Verlet integration
    elseif self.actionMode == 2 then -- Grabbed terrain
        -- Hook position is fixed where it grabbed
        self.Pos.X = self.apx[self.currentSegments] -- Ensure self.Pos matches anchor
        self.Pos.Y = self.apy[self.currentSegments]
        -- Velocity of the terrain anchor is zero
        self.lastX[self.currentSegments] = self.apx[self.currentSegments]
        self.lastY[self.currentSegments] = self.apy[self.currentSegments]
    elseif self.actionMode == 3 and self.target and self.target.ID ~= rte.NoMOID then -- Grabbed MO
        local effective_target = RopeStateManager.getEffectiveTarget(self)
        if effective_target and effective_target.ID ~= rte.NoMOID then
            self.Pos = effective_target.Pos
            self.apx[self.currentSegments] = effective_target.Pos.X
            self.apy[self.currentSegments] = effective_target.Pos.Y
            self.lastX[self.currentSegments] = effective_target.Pos.X - (effective_target.Vel.X or 0)
            self.lastY[self.currentSegments] = effective_target.Pos.Y - (effective_target.Vel.Y or 0)
        else
            -- Target lost or invalid, consider unhooking or reverting to terrain grab
            self.ToDelete = true -- Or change actionMode to 2 if it should stick to the last location
            return
        end
    end
    
    -- Calculate current actual distance between player and hook
    self.lineVec = SceneMan:ShortestDistance(parentActor.Pos, self.Pos, self.mapWrapsX)
    self.lineLength = self.lineVec.Magnitude -- This is the visual length

    -- State-dependent logic for currentLineLength (the physics length)
    if self.actionMode == 1 then -- Flying
        if self.lineLength >= self.maxShootDistance then
            if not self.limitReached then
                self.clickSound:Play(parentActor.Pos)
                self.limitReached = true
            end
            self.currentLineLength = self.maxShootDistance -- Physics length capped
            -- The RopePhysics.applyRopeConstraints will handle the "binding"
        else
            self.currentLineLength = self.lineLength -- Physics length matches visual
            self.limitReached = false
        end
        self.setLineLength = self.currentLineLength -- Keep setLineLength synchronized during flight
    else -- Attached (Terrain or MO)
        -- currentLineLength is controlled by input or auto-climbing, clamped.
        self.currentLineLength = math.max(10, math.min(self.currentLineLength, self.maxLineLength))
        self.setLineLength = self.currentLineLength -- Keep setLineLength synchronized
        -- limitReached is true if currentLineLength is at maxLineLength, false otherwise
        self.limitReached = (self.currentLineLength >= self.maxLineLength - 0.1) -- Small tolerance
    end

    -- Dynamic rope segment calculation
    local desiredSegments = RopePhysics.calculateOptimalSegments(self, math.max(1, self.currentLineLength))
    if desiredSegments ~= self.currentSegments and math.abs(desiredSegments - self.currentSegments) > 1 then -- Hysteresis
        RopePhysics.resizeRopeSegments(self, desiredSegments)
    end

    -- Core rope physics simulation
    RopePhysics.updateRopePhysics(self, parentActor.Pos, self.Pos, self.currentLineLength)
    
    -- Apply constraints and check for breaking
    local ropeBreaks = RopePhysics.applyRopeConstraints(self, self.currentLineLength)
    if ropeBreaks or self.shouldBreak then -- self.shouldBreak can be set by other logic
        self.ToDelete = true
        if parentActor:IsPlayerControlled() then
            FrameMan:SetScreenScrollSpeed(10.0)
            if self.returnSound then self.returnSound:Play(parentActor.Pos) end
        end
        return -- Exit update if rope breaks
    end

    -- Update hook's own position if it's not attached to an MO
    -- If attached to terrain (actionMode 2), its position is already fixed by its anchor point.
    -- If flying (actionMode 1), its position is determined by its Verlet integration + constraints.
    if self.actionMode == 1 then
         -- The hook's self.Pos is updated by its own physics, but constraints might adjust segment end
        self.Pos.X = self.apx[self.currentSegments]
        self.Pos.Y = self.apy[self.currentSegments]
    end

    -- Aim the gun
    if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
        local flipAng = parentActor.HFlipped and math.pi or 0
        self.parentGun.RotAngle = self.lineVec.AbsRadAngle + flipAng
        if MovableMan:IsParticle(self.parentGun.Magazine) then -- Check if Magazine is a particle
             ToMOSParticle(self.parentGun.Magazine).Scale = 0 -- Hide magazine when grapple is active
        end

        -- Handle unhooking from firing the gun again
        if self.parentGun.FiredFrame then
            if self.actionMode == 1 then -- If flying, just delete
                self.ToDelete = true
            elseif self.actionMode > 1 then -- If attached, mark as ready to release
                self.canRelease = true 
            end
        end
        -- If marked ready and gun is fired again (or activated for some guns)
        if self.canRelease and self.parentGun.FiredFrame and 
           (self.parentGun.Vel.Y ~= -1 or self.parentGun:IsActivated()) then -- Original logic for release condition
            self.ToDelete = true
        end
    end
    
    -- Player-specific controls and unhooking mechanisms
    if IsAHuman(parentActor) then -- Or IsACrab, if they can use it
        local parentHuman = ToAHuman(parentActor) -- Cast for specific human properties if needed
        if parentHuman:IsPlayerControlled() then
            -- Unhook with Reload key (R)
            if RopeInputController.handleReloadKeyUnhook(self, controller) then
                self.ToDelete = true
            end
            -- Unhook with double-tap crouch (if not holding the gun)
            if RopeInputController.handleTapDetection(self, controller) then
                self.ToDelete = true
            end
        end
        -- Gun stance offset when holding the gun
        if self.parentGun and self.parentGun.RootID == parentActor.ID then
             if MovableMan:IsParticle(self.parentGun.Magazine) then -- Check if Magazine is a particle
                ToMOSParticle(self.parentGun.Magazine).RoundCount = 0 -- Visually empty
             end
            local offsetAngle = parentActor.FlipFactor * (self.lineVec.AbsRadAngle - parentActor:GetAimAngle(true))
            self.parentGun.StanceOffset = Vector(self.lineLength, 0):RadRotate(offsetAngle)
        end
    end

    -- Handle Pie Menu actions
    if RopeInputController.handlePieMenuSelection(self) then
        self.ToDelete = true -- Pie menu selected "Unhook"
    end

    -- Manage crank sound
    if not self.crankSoundInstance or self.crankSoundInstance.ToDelete then
        self.crankSoundInstance = CreateAEmitter("Grapple Gun Sound Crank")
        self.crankSoundInstance.Pos = parentActor.Pos
        MovableMan:AddParticle(self.crankSoundInstance)
    else
        self.crankSoundInstance.Pos = parentActor.Pos
        if self.lastSetLineLength and math.abs(self.lastSetLineLength - self.currentLineLength) > 0.1 then
            self.crankSoundInstance:EnableEmission(true)
        else
            self.crankSoundInstance:EnableEmission(false)
        end
    end
    self.lastSetLineLength = self.currentLineLength


    -- State-specific updates
    if self.actionMode == 1 then -- Hook is in flight
        RopeStateManager.applyStretchMode(self) -- (Currently does nothing if stretchMode is false)
        RopeStateManager.checkAttachmentCollisions(self) -- This can change self.actionMode
        -- RopeStateManager.checkLengthLimit(self) -- Length limit during flight is handled above
    elseif self.actionMode > 1 then -- Hook has stuck (terrain or MO)
        -- Calculate forces affecting player (used by input controller for climb speed)
        self.parentForces = 1 + (parentActor.Vel.Magnitude * 10 + parentActor.Mass) / (1 + self.lineLength)
        
        local terrCheck = false
        if self.parentRadius then
            terrCheck = SceneMan:CastStrengthRay(parentActor.Pos, 
                                                 self.lineVec:SetMagnitude(self.parentRadius), 
                                                 0, Vector(), 2, rte.airID, self.mapWrapsX)
        end

        RopeInputController.handleAutoRetraction(self, terrCheck)
        RopeInputController.handleRopePulling(self) -- Handles manual climb/extend inputs

        -- Physics for attached states (pulling player/MO) are now primarily handled by RopePhysics.applyRopeConstraints
        -- and the resulting tension. Direct force application here should be minimal or for specific effects.
        -- RopeStateManager.applyTerrainPullPhysics(self) -- If direct forces are still desired
        -- RopeStateManager.applyMOPullPhysics(self)
    end
    
    -- Render the rope
    RopeRenderer.drawRope(self, player)

    -- Final deletion check and cleanup
    if self.ToDelete then
        if self.parentGun and MovableMan:IsParticle(self.parentGun.Magazine) then
            local mag = ToMOSParticle(self.parentGun.Magazine)
            -- Show magazine briefly as if hook is retracting
            mag.Pos = parentActor.Pos + (self.lineVec * 0.5)
            mag.Scale = 1
            mag.Frame = 0 -- Assuming frame 0 is the visible magazine
        end
        if self.returnSound then self.returnSound:Play(parentActor.Pos) end
    end
end

function Destroy(self)
    if self.crankSoundInstance and not self.crankSoundInstance.ToDelete then
        self.crankSoundInstance.ToDelete = true
    end
    
    -- Clean up references on the parent gun
    if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
        self.parentGun.HUDVisible = true -- Assuming it was hidden
        self.parentGun:RemoveNumberValue("GrappleMode")
        -- Reset stance offset if it was modified
        self.parentGun.StanceOffset = Vector(0,0) 
        if MovableMan:IsParticle(self.parentGun.Magazine) then
            ToMOSParticle(self.parentGun.Magazine).Scale = 1 -- Ensure magazine is visible
        end
    end
end