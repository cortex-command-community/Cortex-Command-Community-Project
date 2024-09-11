function Create(self)
	self.charge = 0;

	self.minFireVel = 10;
	self.maxFireVel = 50;

	self.chargeDelay = 1000;

	self.animTimer = Timer();
	self.chargeTimer = Timer();
	self.chargeTimer:SetSimTimeLimitMS(self.chargeDelay);
	--The following timer prevents a glitch where you can fire twice by putting the gun inside the inventory while charging
	self.inventorySwapTimer = Timer();
	self.inventorySwapTimer:SetSimTimeLimitMS(math.ceil(TimerMan.DeltaTimeMS));
	self.activeSound = CreateSoundContainer("Destroyer Emission Sound", "Dummy.rte");
end

function Update(self)
	if self.Magazine then
		if self.inventorySwapTimer:IsPastSimTimeLimit() then
			self.activeSound:Stop();
			self.charge = 0;
		end
		
		self.inventorySwapTimer:Reset();
		if self.Magazine.RoundCount > 0 then
			if self.animTimer:IsPastSimMS(200 * (1 - self.charge)) then
				self.animTimer:Reset();
				self.Frame = self.Frame < (self.FrameCount - 1) and self.Frame + 1 or 0;
				if self.Frame == 1 then
					local effect = CreateMOPixel("Destroyer Muzzle Glow");
					effect.Pos = self.MuzzlePos;
					effect.Vel = self.Vel * 0.5;
					MovableMan:AddParticle(effect);

					local damagePar = CreateMOPixel("Dummy.rte/Destroyer Emission Particle 2");
					damagePar.Pos = self.MuzzlePos;
					damagePar.Vel = self.Vel * 0.5 + Vector(math.random(5) * (1 + self.charge), 0):RadRotate(6.28 * math.random());
					damagePar.Team = self.Team;
					damagePar.IgnoresTeamHits = true;
					damagePar.Lifetime = 100 * (1 + self.charge);
					MovableMan:AddParticle(damagePar);
				end
			end

			if self:DoneReloading() then
				self:Deactivate();
			end

			if self:IsActivated() and not self.forceFire then
				self:Deactivate();

				if self.activeSound:IsBeingPlayed() then
					self.activeSound.Pos = self.Pos;
					self.activeSound.Pitch = self.charge;
				else
					self.activeSound:Play(self.Pos);
				end

				if not self.chargeTimer:IsPastSimTimeLimit() then
					self.charge = self.chargeTimer.ElapsedSimTimeMS/self.chargeDelay;
				else
					self.charge = 1;
					--CPU actor will release the beam at full power
					local parent = self:GetRootParent();
					if parent and IsActor(parent) and not ToActor(parent):IsPlayerControlled() then
						self.forceFire = true;
					end
				end

				self.Magazine.RoundCount = math.ceil(self.charge * 100);
			else
				self.Magazine.RoundCount = 1;
				if self.charge > 0 then
					--Trigger gun like normal and dispense the shot
					self:Activate();
				end
				self.chargeTimer:Reset();
			end
		else
			self:Reload();
		end
	else
		self.Frame = 0;
	end
	if self.FiredFrame then
		local par = CreateAEmitter("Destroyer Cannon Shot");
		par.Team = self.Team;
		par.IgnoresTeamHits = true;
		par.Pos = self.MuzzlePos;
		par.Vel = Vector((self.minFireVel + (self.maxFireVel - self.minFireVel) * self.charge) * self.FlipFactor, 0):RadRotate(self.RotAngle);
		MovableMan:AddParticle(par);

		self.charge = 0;
		self.activeSound:Stop();

		self.forceFire = false;
	end
end

function Destroy(self)
	self.activeSound:Stop();
end