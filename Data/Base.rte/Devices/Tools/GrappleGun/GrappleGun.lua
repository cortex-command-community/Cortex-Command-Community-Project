---@diagnostic disable: undefined-global
-- Localize Cortex Command globals
local Timer = Timer
local PresetMan = PresetMan
local CreateMOSRotating = CreateMOSRotating
local IsActor = IsActor 
local Actor = Actor 
local ToMOSParticle = ToMOSParticle
local ToMOSprite = ToMOSprite
local PrimitiveMan = PrimitiveMan
local ActivityMan = ActivityMan
local MovableMan = MovableMan 
local Vector = Vector
local Controller = Controller -- For Controller.BODY_PRONE etc.
local rte = rte

function Create(self)
	-- Timers and counters for tap-based controls (e.g., double-tap to retrieve hook)
	self.tapTimerJump = Timer() -- Used for crouch-tap detection.
	self.tapCounter = 0
	-- self.didTap = false -- Seems unused, consider removing.
	self.canTap = false -- Flag to register the first tap in a sequence.

	self.tapTime = 200 -- Max milliseconds between taps for them to count as a sequence.
	self.tapAmount = 2 -- Number of taps required.
	
	self.guide = false -- Whether to show the aiming guide arrow.

	-- Create the guide arrow MOSRotating. This is a visual aid.
	-- Ensure "Grapple Gun Guide Arrow" preset exists and is a MOSRotating.
	local arrowPreset = PresetMan:GetPreset("Grapple Gun Guide Arrow", "MOSRotating", "Grapple Gun Guide Arrow")
	if arrowPreset and arrowPreset.ClassName == "MOSRotating" then
		self.arrow = CreateMOSRotating("Grapple Gun Guide Arrow")
		if self.arrow then
			self.arrow.GlobalAccurateDelete = true -- Ensure it cleans up properly
		end
	else
		self.arrow = nil -- Preset not found or incorrect type
		-- Log an error or warning if preset is missing/incorrect
		-- print("Warning: Grapple Gun Guide Arrow preset not found or incorrect type.")
	end

	self.originalRoundCount = 1
	self.hasGrappleActive = false
end

function Update(self)
	local parent = self:GetRootParent()

	-- Ensure the gun is held by a valid, player-controlled Actor.
	if not parent or not IsActor(parent) then
		self:Deactivate() -- If not held by an actor, deactivate.
		return
	end

	local parentActor = ToActor(parent) -- Cast to Actor base type
	-- Specific casting to AHuman or ACrab can be done if needed for type-specific logic

	if not parentActor:IsPlayerControlled() or parentActor.Status >= Actor.DYING then
		self:Deactivate() -- Deactivate if not player controlled or if player is dying.
		return
	end

	local controller = parentActor:GetController()
	if not controller then
		self:Deactivate() -- Should not happen if IsPlayerControlled is true, but good check.
		return
	end

	-- REMOVE/COMMENT OUT this section that deactivates in background:
	--[[
	if parentActor.EquippedBGItem and parentActor.EquippedBGItem.ID == self.ID and parentActor.EquippedItem then
		self:Deactivate()
		// Potentially return here if no further logic should run for a BG equipped grapple gun.
	end
	--]]
	
	-- Allow gun to stay active in background for rope functionality

	-- Magazine handling (visual representation of the hook's availability)
	if self.Magazine and MovableMan:IsParticle(self.Magazine) then
		local magazineParticle = ToMOSParticle(self.Magazine)
		
		-- Double tapping crouch retrieves the hook (if a grapple is active)
		-- This logic seems to be for initiating a retrieve action from the gun itself.
		-- The actual unhooking is handled by the Grapple.lua script's tap detection.
		-- This section might be redundant if Grapple.lua's tap detection is comprehensive.
		if magazineParticle.Scale == 1 then -- Assuming Scale 1 means hook is "loaded" / available to fire
			-- The following stance offsets seem to be for when the hook is *not* fired yet.
			-- Consider if this is the correct condition.
			local parentSprite = ToMOSprite(self:GetParent()) -- Assuming self:GetParent() is the gun's sprite component
			if parentSprite then
				local spriteWidth = parentSprite:GetSpriteWidth() or 0
				self.StanceOffset = Vector(spriteWidth, 1)
				self.SharpStanceOffset = Vector(spriteWidth, 1)
			end

			-- REMOVE the entire crouch-tap section from the gun - it should only be in the hook
			-- The gun should NOT handle unhooking directly
			
			-- Only keep this for other gun functionality, NOT for unhooking:
			if controller:IsState(Controller.WEAPON_RELOAD) then
				-- Gun's own reload logic here (if any)
				-- Do NOT send unhook signals from here
			end
			
		end

		-- Guide arrow visibility logic
		-- Show if magazine scale is 0 (hook is fired) AND not sharp aiming, OR if parent is moving fast.
		local shouldShowGuide = false
		if magazineParticle.Scale == 0 and not controller:IsState(Controller.AIM_SHARP) then
			shouldShowGuide = true
		elseif parentActor.Vel and parentActor.Vel:MagnitudeIsGreaterThan(6) then
			shouldShowGuide = true
		end
		self.guide = shouldShowGuide
	else
		self.guide = false -- No magazine or not a particle, so no guide based on it.
	end

	-- Draw the guide arrow if enabled and valid
	if self.guide and self.arrow and self.arrow.ID ~= rte.NoMOID then
		local frame = 0
		if parentActor.Vel and parentActor.Vel:MagnitudeIsGreaterThan(12) then
			frame = 1 -- Use a different arrow frame for higher speeds
		end
		
		-- Calculate positions for drawing the arrow
		-- EyePos might not exist on all Actor types, ensure parentActor has it or use a fallback.
		local eyePos = parentActor.EyePos or Vector(0,0) 
		local startPos = (parentActor.Pos + eyePos + self.Pos)/3 -- Averaged position
		local aimAngle = parentActor:GetAimAngle(true)
		local aimDistance = parentActor.AimDistance or 50 -- Default AimDistance if not present
		local guidePos = startPos + Vector(aimDistance + (parentActor.Vel and parentActor.Vel.Magnitude or 0), 0):RadRotate(aimAngle)
		
		-- Ensure the arrow MO still exists before trying to draw with it
		if MovableMan:IsValid(self.arrow) then
			 PrimitiveMan:DrawBitmapPrimitive(ActivityMan:GetActivity():ScreenOfPlayer(controller.Player), guidePos, self.arrow, aimAngle, frame)
		else
			self.arrow = nil -- Arrow MO was deleted, nullify reference
		end
	end
	
	-- Check if we have an active grapple
	local hasActiveGrapple = false
	for mo in MovableMan.AddedActors do
		if mo and mo.PresetName == "Grapple Gun Claw" and mo.parentGun and mo.parentGun.ID == self.ID then
			hasActiveGrapple = true
			break
		end
	end
	
	-- Update magazine based on grapple state
	if self.Magazine and MovableMan:IsParticle(self.Magazine) then
		local mag = ToMOSParticle(self.Magazine)
		if hasActiveGrapple then
			mag.RoundCount = 0 -- Empty when grapple is out
			self.hasGrappleActive = true
		elseif self.hasGrappleActive and not hasActiveGrapple then
			-- Grapple just returned, restore ammo
			mag.RoundCount = 1
			self.hasGrappleActive = false
		end
	end

	-- Ensure magazine is visually "full" and ready if no grapple is active.
	-- This assumes the HDFirearm's standard magazine logic handles firing.
	-- If a grapple claw MO (the projectile) is active, Grapple.lua will hide the magazine.
	-- This section ensures it's visible when no grapple is out.
	if self.Magazine and MovableMan:IsParticle(self.Magazine) then
		local magParticle = ToMOSParticle(self.Magazine)
		local isActiveGrapple = false
		-- Check if there's an active grapple associated with this gun
		for mo_instance in MovableMan:GetMOsByPreset("Grapple Gun Claw") do
			if mo_instance and mo_instance.parentGun and mo_instance.parentGun.ID == self.ID then
				isActiveGrapple = true
				break
			end
		end

		if not isActiveGrapple then
			magParticle.RoundCount = 1 -- Visually full
			magParticle.Scale = 1	  -- Visible
			magParticle.Frame = 0	  -- Standard frame
		else
			magParticle.Scale = 0 -- Hidden by active grapple (Grapple.lua also does this)
			magParticle.RoundCount = 0 -- Visually empty

		end
	end
end

function Destroy(self)
	-- Clean up the guide arrow if it exists
	if self.arrow and self.arrow.ID ~= rte.NoMOID and MovableMan:IsValid(self.arrow) then
		MovableMan:RemoveMO(self.arrow)
		self.arrow = nil
	end
end