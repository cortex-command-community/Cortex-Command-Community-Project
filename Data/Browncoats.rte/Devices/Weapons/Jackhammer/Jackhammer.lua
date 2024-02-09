-- This script incorporates Filipawn Industries code
-- There are likely better ways of doing a lot of this, potentially even standardizing it so it can be easily used more widely

-- Last worked on 19/11/2023

function OnFire(self)
	self.mechSound:Play(self.Pos);
end

function Create(self)
	self.preSound = CreateSoundContainer("Pre Browncoat SG-10", "Browncoat.rte");
	self.mechSound = CreateSoundContainer("Mech Browncoat A", "Browncoat.rte");
	
	self.delayedFire = false
	self.delayedFireTimer = Timer();
	self.delayedFireTimeMS = 75;
	self.delayedFireEnabled = true;
	self.fireDelayTimer = Timer();
	self.activated = false;
	self.delayedFirstShot = true;
end

function ThreadedUpdate(self)
	self.preSound.Pos = self.Pos;
	
	if self:DoneReloading() then
		self.fireDelayTimer:Reset();
		self.activated = false;
		self.delayedFire = false;
		
	end
	
	if self.delayedFire and self.delayedFireTimer:IsPastSimMS(self.delayedFireTimeMS) then
		self:Activate();
		self.delayedFire = false
		self.delayedFirstShot = false;
	end

	local fire = self:IsActivated() and self.RoundInMagCount > 0

	if self.delayedFirstShot == true then
		if self.RoundInMagCount > 0 then
			self:Deactivate()
		end
		
		--if self.parent:GetController():IsState(Controller.WEAPON_FIRE) and not self:IsReloading() then
		if fire and not self:IsReloading() then
			if not self.Magazine or self.RoundInMagCount < 1 then
				--self:Reload()
				self:Activate()
			elseif not self.activated and not self.delayedFire and self.fireDelayTimer:IsPastSimMS(1 / (self.RateOfFire / 60) * 1000) then
				self.activated = true
				
				self.preSound:Play(self.Pos);
				
				self.fireDelayTimer:Reset()
				
				self.delayedFire = true
				self.delayedFireTimer:Reset()
			end
		else
			if self.activated then
				self.activated = false
			end
		end
	elseif fire == false then
		self.firstShot = true;
		self.delayedFirstShot = true;
	end
end