-- filepath: /home/cretin/git/Cortex-Command-Community-Project/Data/Base.rte/Devices/Tools/GrappleGun/Grapple.lua
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
    self.actionMode = 0    -- 0 = start, 1 = flying, 2 = grab terrain, 3 = grab MO
    self.climb = 0
    self.canRelease = false

    self.tapTimer = Timer()
    self.tapCounter = 0
    self.didTap = false
    self.canTap = false

    self.fireVel = 40      -- This immediately overwrites the .ini FireVel
    self.maxLineLength = 400   -- Shorter rope for faster gameplay
    self.maxShootDistance = self.maxLineLength * 0.95 -- 95% of maxLineLength (5% less shooting distance)
    self.setLineLength = 0
    self.lineStrength = 10000 -- EXTREMELY HIGH force threshold - virtually unbreakable (was 120)

    self.limitReached = false
    self.stretchMode = false   -- Disabled for rigid rope behavior
    self.stretchPullRatio = 0.0 -- No stretching allowed for rigid rope
    self.pieSelection = 0  -- 0 is nothing, 1 is full retract, 2 is partial retract, 3 is partial extend, 4 is full extend

    self.climbDelay = 8    -- Faster climbing for shorter rope
    self.tapTime = 150     -- Maximum amount of time between tapping for claw to return
    self.tapAmount = 2     -- How many times to tap to bring back rope
    self.mouseClimbLength = 200    -- Adjusted for shorter rope
    self.climbInterval = 4.0       -- Faster retraction/extension
    self.autoClimbIntervalA = 5.0  -- Faster auto-climbing
    self.autoClimbIntervalB = 3.0  -- Faster auto-climbing

    self.stickSound = CreateSoundContainer("Grapple Gun Claw Stick", "Base.rte")
    self.clickSound = CreateSoundContainer("Grapple Gun Click", "Base.rte")
    self.returnSound = CreateSoundContainer("Grapple Gun Return", "Base.rte")

    -- Rope physics variables from VelvetGrapple
    self.currentLineLength = 0
    self.longestLineLength = 0
    self.cablespring = 0.01 -- Very low for completely rigid rope behavior (was 0.05)
    
    -- Dynamic rope segment calculation variables
    self.minSegments = 1   -- Minimum number of segments
    self.maxSegments = 500  -- Maximum number of segments
    self.segmentLength = 12 -- Target length per segment (increased for better performance)
    self.currentSegments = self.minSegments -- Current number of segments
    
    -- Mousewheel control variables
    self.shiftScrollSpeed = 8.0 -- Faster rope control with Shift+Mousewheel

    --ESTABLISH LINE
    self.apx = {}
    self.apy = {}
    self.lastX = {}
    self.lastY = {}
    
    local px = self.Pos.X
    local py = self.Pos.Y
    
    -- Initialize with minimum number of segments
    for i = 0, self.maxSegments do
        self.apx[i] = px
        self.apy[i] = py
        self.lastX[i] = px
        self.lastY[i] = py
    end
    
    self.lastX[self.minSegments] = px - self.Vel.X
    self.lastY[self.minSegments] = py - self.Vel.Y
    self.currentSegments = self.minSegments -- Start with minimum segments
    --slots 0 and currentSegments are ANCHOR POINTS

    --Find the parent gun that fired us
    for gun in MovableMan:GetMOsInRadius(self.Pos, 50) do
        if gun and gun.ClassName == "HDFirearm" and gun.PresetName == "Grapple Gun" and SceneMan:ShortestDistance(self.Pos, ToHDFirearm(gun).MuzzlePos, self.mapWrapsX):MagnitudeIsLessThan(5) then
            self.parentGun = ToHDFirearm(gun)
            self.parent = MovableMan:GetMOFromID(gun.RootID)
            if MovableMan:IsActor(self.parent) then
                self.parent = ToActor(self.parent)
                if IsAHuman(self.parent) then
                    self.parent = ToAHuman(self.parent)
                elseif IsACrab(self.parent) then
                    self.parent = ToACrab(self.parent)
                end

                self.Vel = (self.parent.Vel * 0.5) + Vector(self.fireVel, 0):RadRotate(self.parent:GetAimAngle(true))
                self.parentGun:RemoveNumberValue("GrappleMode")
                for part in self.parent.Attachables do
                    local radcheck = SceneMan:ShortestDistance(self.parent.Pos, part.Pos, self.mapWrapsX).Magnitude + part.Radius
                    if self.parentRadius == nil or radcheck > self.parentRadius then
                        self.parentRadius = radcheck
                    end
                end

                self.actionMode = 1
            end
            break
        end
    end

    if self.parentGun == nil then   -- Failed to find our gun, abort
        self.ToDelete = true
    end
end

function Update(self)
	if self.parent and IsMOSRotating(self.parent) and self.parent:HasObject("Grapple Gun") then
		if MovableMan:IsActor(self.parent) then
			local controller = self.parent:GetController()
			if controller then
				local player = controller.Player or 0 -- Get player for drawing, fallback to 0
				local startPos = self.parent.Pos

				self.ToDelete = false
				self.ToSettle = false

				-- Make sure we have valid rope data, but allow zero length
				if self.actionMode == 1 and self.currentLineLength < 0 then
					self.currentLineLength = 0 -- Allow zero length compression
				end
				
				-- Update line length when in flight
				if self.actionMode == 1 then
					-- Immediately update rope length based on actual hook position
					self.lineVec = SceneMan:ShortestDistance(self.parent.Pos, self.Pos, self.mapWrapsX)
					self.lineLength = self.lineVec.Magnitude
					self.currentLineLength = self.lineLength
					
					-- Check if we\'ve reached the maximum shooting distance during flight
					if self.lineLength >= self.maxShootDistance then
						-- Stop the claw at max shooting distance but keep it in flight mode
						local maxShootVec = self.lineVec:SetMagnitude(self.maxShootDistance)
						self.Pos = self.parent.Pos + maxShootVec
						self.Vel = Vector(0, 0) -- Stop the claw
						self.currentLineLength = self.maxShootDistance
						self.limitReached = true
						-- Keep actionMode = 1 (flight) so it can still detect collisions
						self.clickSound:Play(self.parent.Pos)
					end
					
					-- Update rope anchor points directly for flight mode
					self.apx[0] = self.parent.Pos.X
					self.apy[0] = self.parent.Pos.Y
					self.apx[self.currentSegments] = self.Pos.X
					self.apy[self.currentSegments] = self.Pos.Y
					
					-- Set all lastX/lastY positions to prevent velocity inheritance from previous mode
					-- Commenting out this loop allows for rope physics during flight
					-- for i = 0, self.currentSegments do
					-- 	self.lastX[i] = self.apx[i]
					-- 	self.lastY[i] = self.apy[i]
					-- end
				end
				
				-- Calculate optimal number of segments based on rope length using our module function
				local desiredSegments = RopePhysics.calculateOptimalSegments(self, math.max(1, self.currentLineLength))
				
				-- Resize rope if needed (don't resize on every minor change to avoid performance issues)
				if desiredSegments ~= self.currentSegments then
					-- Add some hysteresis to prevent frequent resizing at length boundaries
					if math.abs(desiredSegments - self.currentSegments) > 1 then
						RopePhysics.resizeRopeSegments(self, desiredSegments)
					end
				end
				
				-- Proper rope physics simulation using the RopePhysics module
				local endPos = self.Pos
				
				-- Use full rope physics simulation for both flight and attached modes
				RopePhysics.updateRopePhysics(self, startPos, endPos, self.currentLineLength)
				
				-- Apply constraints and check for rope breaking (extremely high threshold)
				local ropeBreaks = RopePhysics.applyRopeConstraints(self, self.currentLineLength)
				if ropeBreaks or self.shouldBreak then
					-- Rope snapped due to EXTREME tension (500% stretch)
					self.ToDelete = true
					if self.parent and self.parent:IsPlayerControlled() then
						-- Add screen shake and sound effect when rope breaks
						FrameMan:SetScreenScrollSpeed(10.0) -- More dramatic shake for extreme break
						if self.returnSound then
							self.returnSound:Play(self.parent.Pos)
						end
					end
					return -- Exit early since rope is breaking
				end
				
				-- Special handling for attached targets (MO grabbing)
				if self.actionMode == 3 and self.target and self.target.ID ~= rte.NoMOID then
					local target = self.target
					if target.ID ~= target.RootID then
						local mo = target:GetRootParent()
						if mo.ID ~= rte.NoMOID and IsAttachable(target) then
							target = mo
						end
					end
					
					-- Update hook position to follow the target
					self.Pos = target.Pos
					self.apx[self.currentSegments] = target.Pos.X
					self.apy[self.currentSegments] = target.Pos.Y
					
					-- Apply target velocity to the hook anchor for physics continuity
					self.lastX[self.currentSegments] = self.apx[self.currentSegments] - target.Vel.X
					self.lastY[self.currentSegments] = self.apy[self.currentSegments] - target.Vel.Y
				else
					-- Update hook position from rope physics when not attached to MO
					if self.actionMode > 1 then -- Hook is stuck to terrain
						-- Let the rope physics determine hook position constraints
						self.Pos.X = self.apx[self.currentSegments]
						self.Pos.Y = self.apy[self.currentSegments]
					end
				end

				-- Draw the rope using the renderer module
				RopeRenderer.drawRope(self, player)

				-- Update lineVec and lineLength based on current positions
				self.lineVec = SceneMan:ShortestDistance(self.parent.Pos, self.Pos, self.mapWrapsX)
				self.lineLength = self.lineVec.Magnitude

				-- Update hook position if length limit is reached during flight
				if self.actionMode == 1 and self.limitReached == true then
					self.Pos.X = self.apx[self.currentSegments]
					self.Pos.Y = self.apy[self.currentSegments]
				end

				-- Update current line length based on action mode - CENTRALIZED CONTROL
				if self.actionMode == 1 and self.limitReached == false then 
					-- Always update rope length while in flight - rope should be tight
					self.currentLineLength = self.lineLength 
					self.setLineLength = self.currentLineLength
				elseif self.actionMode > 1 then
					-- When attached, currentLineLength is controlled by input/auto-climbing
					-- Ensure it stays within bounds
					self.currentLineLength = math.max(10, math.min(self.currentLineLength, self.maxLineLength))
					self.setLineLength = self.currentLineLength
				end

				-- Single length limit check - now handled during flight phase
				if self.currentLineLength > self.maxLineLength then
					self.currentLineLength = self.maxLineLength
					self.setLineLength = self.maxLineLength
					-- limitReached is now set during flight phase
				else
					if self.actionMode > 1 then  -- Only reset limit flag when attached, not during flight
						self.limitReached = false
					end
				end

				if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
					self.parent = ToMOSRotating(MovableMan:GetMOFromID(self.parentGun.RootID))

					if self.parentGun.Magazine then
						self.parentGun.Magazine.Scale = 0
					end

					startPos = self.parentGun.Pos
					local flipAng = self.parent.HFlipped and 3.14 or 0
					self.parentGun.RotAngle = self.lineVec.AbsRadAngle + flipAng

					-- Handle pie menu selection
					if RopeInputController.handlePieMenuSelection(self) then
						self.ToDelete = true
					end

					-- Handle unhooking from firing
					if self.parentGun.FiredFrame then
						if self.actionMode == 1 then
							self.ToDelete = true
						else
							self.canRelease = true
						end
					end

					if self.parentGun.FiredFrame and self.canRelease and 
					   (Vector(self.parentGun.Vel.X, self.parentGun.Vel.Y) ~= Vector(0, -1) or 
						self.parentGun:IsActivated()) then
						self.ToDelete = true
					end
				end

				if IsAHuman(self.parent) then
					self.parent = ToAHuman(self.parent)
					-- We now have a user that controls this grapple (controller already obtained above)
					-- Point the gun towards the hook if our user is holding it
					if (self.parentGun and self.parentGun.ID ~= rte.NoMOID) and (self.parentGun:GetRootParent().ID == self.parent.ID) then
						if self.parent:IsPlayerControlled() then
							if controller:IsState(Controller.WEAPON_RELOAD) then
								-- Only unhook with R if holding the Grapple Gun
								if self.parent.EquippedItem and self.parentGun and self.parent.EquippedItem.ID == self.parentGun.ID then
									self.ToDelete = true
								end
							end
							
							if self.parentGun.Magazine then
								self.parentGun.Magazine.RoundCount = 0
							end
						end
						
						local offset = Vector(self.lineLength, 0):RadRotate(self.parent.FlipFactor * (self.lineVec.AbsRadAngle - self.parent:GetAimAngle(true)))
						self.parentGun.StanceOffset = offset
					end
				end
				
				-- Add crank sound if not already present
				if MovableMan:IsParticle(self.crankSound) then
					self.crankSound.ToDelete = false
					self.crankSound.ToSettle = false
					self.crankSound.Pos = startPos
					if self.lastSetLineLength ~= self.currentLineLength then
						self.crankSound:EnableEmission(true)
					else
						self.crankSound:EnableEmission(false)
					end
				else
					self.crankSound = CreateAEmitter("Grapple Gun Sound Crank")
					self.crankSound.Pos = startPos
					MovableMan:AddParticle(self.crankSound)
				end

				self.lastSetLineLength = self.currentLineLength

				if self.actionMode == 1 then    -- Hook is in flight
					-- Apply stretch mode physics for retracting the hook
					RopeStateManager.applyStretchMode(self)
					
					-- Check for collisions and update state if needed
					RopeStateManager.checkAttachmentCollisions(self)
					
					-- Check for length limit and apply physics if needed
					RopeStateManager.checkLengthLimit(self)
				elseif self.actionMode > 1 then -- Hook has stuck
					-- Update rope anchor point for hook position
					self.apx[self.currentSegments] = self.Pos.X
					self.apy[self.currentSegments] = self.Pos.Y

					-- Actor mass and velocity affect pull strength negatively, rope length affects positively
					self.parentForces = 1 + (self.parent.Vel.Magnitude * 10 + self.parent.Mass)/(1 + self.lineLength)
					
					-- Check if there is terrain between the hook and the user
					local terrVector = Vector()
					local terrCheck = false
					if self.parentRadius ~= nil then
						terrCheck = SceneMan:CastStrengthRay(self.parent.Pos, 
														 self.lineVec:SetMagnitude(self.parentRadius), 
														 0, terrVector, 2, rte.airID, self.mapWrapsX)
					end

					-- Process automatic retraction
					RopeInputController.handleAutoRetraction(self, terrCheck)

					-- Process input based climbing
					RopeInputController.handleRopePulling(self)
					
					-- DISABLE force-based physics - using pure Verlet constraint system instead
					-- The RopePhysics.applyRopeConstraints handles all position constraints
					-- No need for additional spring forces that conflict with rigid constraints
					
					-- UNBREAKABLE ROPE: No automatic unhooking due to target destruction
					-- Rope remains attached even if target MO is destroyed for maximum persistence
				end
				
				-- Check if we should unhook via double-tap mechanic
				if RopeInputController.handleTapDetection(self, controller) then
					self.ToDelete = true
				end
				
				-- Check if we should unhook via R key press
				if RopeInputController.handleReloadKeyUnhook(self, controller) then
					self.ToDelete = true
				end

				-- Special handling for hook deletion - show magazine and play sound
				if self.ToDelete == true then
					if self.parentGun and self.parentGun.Magazine then
						-- Show the magazine as if the hook is being retracted
						local drawPos = self.parent.Pos + (self.lineVec * 0.5)
						self.parentGun.Magazine.Pos = drawPos
						self.parentGun.Magazine.Scale = 1
						self.parentGun.Magazine.Frame = 0
					end
					self.returnSound:Play(self.parent.Pos)
				end

			else
				self.ToDelete = true -- Parent Actor has no controller
			end
		else
			self.ToDelete = true -- Parent is not an Actor
		end
	else
		self.ToDelete = true -- Parent is nil, not MOSRotating, or doesn't have "Grapple Gun"
	end
end

function Destroy(self)
    if MovableMan:IsParticle(self.crankSound) then
        self.crankSound.ToDelete = true
    end
    
    if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
        self.parentGun.HUDVisible = true
        self.parentGun:RemoveNumberValue("GrappleMode")
    end
end