function Create(self)
	self.fireTimer = Timer();
	self.blinkTimer = Timer();
	self.chargeTimer = Timer();
	self.effectTimer = Timer();
	self.fireOn = false;

	self.particleTable = {};
	self.glowCount = 0;
	self.glowIncrementCount = 0;
	self.chargeCounter = 0;

	self.glowChargeIncrement = 0.2;
	self.maxCharge = 40;
	self.chargesPerSecond = 20;
end

function Update(self)
	if self:IsActivated() then
		if self.chargeCounter > self.maxCharge/2 then
			local actor = MovableMan:GetMOFromID(self.RootID);
			if MovableMan:IsActor(actor) and ToActor(actor):IsPlayerControlled() == false then
				ToActor(actor):GetController():SetState(Controller.WEAPON_FIRE,false);
			end
		end

		if self.glowCount < 5 and self.chargeCounter >= self.maxCharge*(self.glowChargeIncrement*self.glowCount) then
			self.glowCount = self.glowCount + 1;
			local effectPar = CreateMOPixel("Annihilator Effect Particle");
			effectPar.Pos = self.MuzzlePos;
			effectPar.PinStrength = 1000;
			MovableMan:AddParticle(effectPar);
			self.particleTable[#self.particleTable+1] = effectPar;
		end

		if self.chargeCounter <= self.maxCharge then
			self.chargeCounter = math.min(self.chargeCounter + ((self.chargeTimer.ElapsedSimTimeMS/1000)*self.chargesPerSecond),self.maxCharge);
			self.chargeTimer:Reset();
		end

		for i = 1, #self.particleTable do
			if MovableMan:IsParticle(self.particleTable[i]) then
				self.particleTable[i].ToDelete = false;
				self.particleTable[i].ToSettle = false;
				self.particleTable[i].PinStrength = 1000;
				self.particleTable[i].Pos = self.MuzzlePos;
			end
		end

		if self.effectTimer:IsPastSimMS(550-((self.chargeCounter/self.maxCharge)*500)) then
			self.effectTimer:Reset();
			local actor = MovableMan:GetMOFromID(self.RootID);
			local effectPar = CreateMOPixel("Annihilator Effect Particle 2");
			effectPar.Pos = self.MuzzlePos;
			effectPar.Vel = Vector(40,0):RadRotate(math.random()*(math.pi*2));
			effectPar:SetWhichMOToNotHit(MovableMan:GetMOFromID(self.RootID),-1);
			if MovableMan:IsActor(actor) then
				effectPar.Team = ToActor(actor).Team;
				effectPar.IgnoresTeamHits = true;
			end
			MovableMan:AddParticle(effectPar);
		end
	else
		if self.chargeCounter > 0 then
			if self.HFlipped == false then
				self.reverseNum = 1;
			else
				self.reverseNum = -1;
			end

			local actor = MovableMan:GetMOFromID(self.RootID);

			for i = 1, self.chargeCounter do
				local damagePar = CreateMOPixel("Annihilator Particle");
				damagePar.Pos = self.MuzzlePos + Vector(((i-1)*-1)*self.reverseNum,0):RadRotate(self.RotAngle);
				damagePar.Vel = Vector(200*self.reverseNum,0):RadRotate(self.RotAngle);
				damagePar:SetWhichMOToNotHit(MovableMan:GetMOFromID(self.RootID),-1);

				if MovableMan:IsActor(actor) then
					damagePar.Team = ToActor(actor).Team;
					damagePar.IgnoresTeamHits = true;
				end

				MovableMan:AddParticle(damagePar);
			end

			for i = 1, #self.particleTable do
				if MovableMan:IsParticle(self.particleTable[i]) then
					self.particleTable[i].ToDelete = true;
				end
			end

			self.particleTable = {};

			local soundfx = CreateAEmitter("Dummy Annihilator Sound Fire");
			soundfx.Pos = self.MuzzlePos;
			MovableMan:AddParticle(soundfx);

			self.chargeCounter = 0;
			self.glowCount = 0;
		end

		for i = 1, #self.particleTable do
			if MovableMan:IsParticle(self.particleTable[i]) then
				self.particleTable[i].ToDelete = true;
			end
		end
		
		self.fireTimer:Reset();
		self.particleTable = {};
	end
end

function Destroy(self)
	for i = 1, #self.particleTable do
		if MovableMan:IsParticle(self.particleTable[i]) then
			self.particleTable[i].ToDelete = true;
		end
	end
end