function Create(self)
	self.chargeTimer = Timer();
	self.chargeCounter = 10;

	self.maxCharge = 10;
	self.chargesPerSecond = self.RateOfFire/100;

	self.bleepSoundPlayed = false;
	self.setAngle = 0;

	self.fireSound = {};
	self.fireSound.low = CreateSoundContainer("Dummy Lancer Fire Sound Low", "Dummy.rte");
	self.fireSound.medium = CreateSoundContainer("Dummy Lancer Fire Sound Medium", "Dummy.rte");
	self.fireSound.high = CreateSoundContainer("Dummy Lancer Fire Sound High", "Dummy.rte");
	self.bleepSound = CreateSoundContainer("Dummy Lancer Bleep", "Dummy.rte");

	self.addedParticles = {};
end

function ThreadedUpdate(self)
	if self.FiredFrame then
		self:RequestSyncedUpdate();
		self.setAngle = self.setAngle + self.chargeCounter/(20 * (1 + self.setAngle));
		self.bleepSoundPlayed = false;

		local charge = math.floor(self.chargeCounter * 0.8);
		for i = 1, charge do
			local damagePar = CreateMOPixel("Dummy Lancer Particle " .. math.ceil(i/2));
			damagePar.Pos = self.MuzzlePos + Vector(((i - 1) * 4 - charge) * self.FlipFactor, 0):RadRotate(self.RotAngle);
			damagePar.Vel = Vector((60 + 10 * charge) * self.FlipFactor, 0):RadRotate(self.RotAngle);
			damagePar.Team = self.Team;
			damagePar.IgnoresTeamHits = true;
			table.insert(self.addedParticles, damagePar);
		end

		local shellPar1 = CreateMOSParticle("Tiny Smoke Ball 1 Glow Yellow");
		table.insert(self.addedParticles, shellPar1);

		local highCharge = self.maxCharge * 0.7;
		local lowCharge = self.maxCharge * 0.3;

		for i = 1, self.chargeCounter do
			local size = i > lowCharge and (i > highCharge and "" or "Small ") or "Tiny ";
			local smokePar = CreateMOSParticle(size .. "Smoke Ball 1 Glow Yellow");
			smokePar.Pos = self.MuzzlePos;
			smokePar.Vel = Vector(math.random(5) * (charge/i) * self.FlipFactor, 0):RadRotate(self.RotAngle);
			smokePar.Team = self.Team;
			smokePar.IgnoresTeamHits = true;
			table.insert(self.addedParticles, smokePar);
		end
		local sound = self.chargeCounter > highCharge and self.fireSound.high or (self.chargeCounter < lowCharge and self.fireSound.low or self.fireSound.medium);
		sound:Play(self.MuzzlePos);

		self.chargeCounter = 1;
	else
		if self.chargeCounter <= self.maxCharge then
			self.chargeCounter = math.min(self.chargeCounter + ((self.chargeTimer.ElapsedSimTimeMS/1000) * self.chargesPerSecond), self.maxCharge);
			self.chargeTimer:Reset();

			if self.chargeCounter == self.maxCharge and self.bleepSoundPlayed == false then
				self.bleepSoundPlayed = self.bleepSound:Play(self.Pos);
			end
		end
	end
	if self.Magazine then
		self.Magazine.RoundCount = self.chargeCounter;
	end
	if self.setAngle > 0 then
		self.setAngle = self.setAngle - (0.02 * (1 + self.setAngle));
		if self.setAngle < 0 then
			self.setAngle = 0;
		end
	end
	self.RotAngle = self.RotAngle + (self.setAngle * self.FlipFactor);
	local jointOffset = Vector(self.JointOffset.X * self.FlipFactor, self.JointOffset.Y):RadRotate(self.RotAngle);
	self.Pos = self.Pos - jointOffset + Vector(jointOffset.X, jointOffset.Y):RadRotate(-self.setAngle * self.FlipFactor);
end

function SyncedUpdate(self)
	for i = 1, #self.addedParticles do
		MovableMan:AddParticle(self.addedParticles[i]);
	end
	self.addedParticles = {};
end