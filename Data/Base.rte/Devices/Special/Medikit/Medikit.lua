function Create(self)
	self.baseStrength = 40;
	self.maxStrength = self.baseStrength * 2;

	self.useSound = CreateSoundContainer("Medikit Use", "Base.rte");
	self.failSound = CreateSoundContainer("Medikit Fail", "Base.rte");
end

function OnFire(self)
	local parent = self:GetRootParent();
	if parent and IsActor(parent) then
		parent = ToActor(parent);
		local target = parent;
		local extend = parent:GetController():IsState(Controller.AIM_SHARP) and self.Radius or 0;

		local moCheck = SceneMan:CastMORay(self.MuzzlePos, Vector(self.IndividualRadius + 5, 0):RadRotate(parent:GetAimAngle(true)), parent.ID, Activity.NOTEAM, rte.airID, false, 1);
		local mo = MovableMan:GetMOFromID(moCheck)
		if mo and IsMOSRotating(mo) and IsActor(ToMOSRotating(mo):GetRootParent()) then
			target = ToActor(ToMOSRotating(mo):GetRootParent());
		end
		if target and (target.Health < target.MaxHealth or target.WoundCount > 0) then
			local targetToughnessCoefficient = 1/(math.sqrt(math.abs(target.Mass - target.InventoryMass) * 0.01) + math.sqrt(target.Material.StructuralIntegrity * 0.01));
			local strength = math.max(self.maxStrength - target:RemoveWounds(math.ceil(self.maxStrength * targetToughnessCoefficient), true, false, false), self.baseStrength) * targetToughnessCoefficient;
			target.Health = math.min(target.Health + strength, target.MaxHealth);
			
			target:FlashWhite(50);
			self.useSound:Play(self.Pos);

			local particleCount = math.ceil(1 + target.Radius * 0.5);
			for i = 1, particleCount do
				local part = CreateMOPixel("Heal Glow", "Base.rte");
				local vec = Vector(particleCount * 2, 0):RadRotate(math.pi * 2 * i/particleCount);
				part.Pos = target.Pos + Vector(0, -particleCount * 0.3):RadRotate(target.RotAngle) + vec;
				part.Vel = target.Vel * 0.5 - Vector(vec.X, vec.Y) * 0.25;
				MovableMan:AddParticle(part);
			end
			local cross = CreateMOSParticle("Particle Heal Effect", "Base.rte");
			cross.Pos = target.AboveHUDPos + Vector(0, 5);
			MovableMan:AddParticle(cross);
			
			if target.Status == Actor.DEAD and target.Health == target.MaxHealth and target.WoundCount == 0 then
				local newActor = target:Clone();
				target.ToDelete = true;
				newActor:SetControllerMode(Controller.CIM_AI, -1);
				newActor.Status = Actor.STABLE;
				MovableMan:AddActor(newActor);
				newActor:FlashWhite(50);
				ActivityMan:GetActivity():ReportDeath(target.Team, -1);
			end
		else
			self.failSound:Play(self.Pos);
			if self.Magazine then
				self.Magazine.RoundCount = self.Magazine.RoundCount + self.RoundsFired;
			end
		end
	end
	if self.RoundInMagCount == 0 then
		self.ToDelete = true;
	end
end