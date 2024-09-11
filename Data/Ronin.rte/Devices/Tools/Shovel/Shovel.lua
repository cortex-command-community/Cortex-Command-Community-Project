function Create(self)
	self.origStanceOffset = Vector(1, 7);
	self.origSharpStanceOffset = Vector(4, 6);
	self.minimumRoF = self.RateOfFire * 0.5;
	self.particleCount = 200;

	self.suitableMaterials = {"Sand", "Topsoil", "Earth", "Dense Earth", "Dense Red Earth", "Red Earth", "Lunar Earth", "Dense Lunar Earth", "Earth Rubble", "Sandbag", "Scrap Metal", "Flesh Scraps"};
	self.collectSound = CreateSoundContainer("Device Switch", "Base.rte");
	self.hitSound = CreateSoundContainer("Ronin Shovel Hit", "Ronin.rte");
	--How much the shovel tilts when firing
	self.angleSize = 1.0;

	self.lastVel = Vector(50 * self.FlipFactor, 0):RadRotate(self.RotAngle);
	self.lastPos = Vector(self.Pos.X, self.Pos.Y);
end

function OnAttach(self, newParent)
	local rootParent = self:GetRootParent();
	if IsAHuman(rootParent) and MovableMan:IsActor(rootParent) then
		local humanRootParent = ToAHuman(rootParent);
		humanRootParent.PieMenu:GetFirstPieSliceByPresetName("Ronin Shovel Fill Sandbag PieSlice").Enabled = humanRootParent:GetNumberValue("RoninShovelResource") >= 10;
	end
end

function Update(self)
	local aimAngle = 0;
	local parent = self:GetRootParent();
	if parent and IsActor(parent) then
		parent = ToActor(parent);
		aimAngle = parent:GetAimAngle(false);
		local controller = parent:GetController();
		local resource = parent:GetNumberValue("RoninShovelResource");
		if parent:IsPlayerControlled() and controller:IsState(Controller.AIM_SHARP) and (not controller:IsMouseControlled() or controller.AnalogAim:MagnitudeIsGreaterThan(0.9)) then
			self.InheritedRotAngleOffset = math.min(self.InheritedRotAngleOffset + (0.0003 * self.RateOfFire), self.angleSize * 2);
		else
			self.InheritedRotAngleOffset = math.max(self.InheritedRotAngleOffset - (0.0003 * self.RateOfFire), 0);
		end

		self.SupportOffset = Vector(-self.InheritedRotAngleOffset * 5, 2);
		if self.FiredFrame then
			local offsetMultiplier = math.max(self.InheritedRotAngleOffset, 1);
			local fireVec = Vector(40 * self.FlipFactor, 0):RadRotate(aimAngle * self.FlipFactor + (0.3 - self.InheritedRotAngleOffset * 0.3) * self.FlipFactor);
			--Digging particles
			for i = 1, self.particleCount do
				local part = CreateMOPixel("Particle Ronin Shovel", "Ronin.rte");
				local goldenAngleSmallerPart = 2.39996;
				part.Pos = self.MuzzlePos + Vector(math.sqrt(self.particleCount - i), 0):RadRotate(i * goldenAngleSmallerPart);
				part.Vel = fireVec;
				part.Sharpness = part.Sharpness * (0.5 + (i/self.particleCount) * 0.5);
				MovableMan:AddParticle(part);
			end

			--Damage particles
			fireVec = fireVec * offsetMultiplier;
			local spread = 0.3;
			local particleCount = 3 * offsetMultiplier;
			for i = 1, particleCount do
				local damagePar = CreateMOPixel("Smack Particle Light", "Base.rte");
				damagePar.Pos = self.MuzzlePos;
				damagePar.Vel = Vector(fireVec.X, fireVec.Y):RadRotate(spread - (spread * 2) * i/particleCount);
				damagePar.Team = self.Team;
				damagePar.IgnoresTeamHits = true;

				damagePar.Mass = math.sqrt(self:GetParent().Mass + self.Mass);
				damagePar.Sharpness = self.Sharpness * (i/particleCount);
				MovableMan:AddParticle(damagePar);
			end

			local overhead = self.InheritedRotAngleOffset > self.angleSize * 0.5;
			local trace = fireVec * rte.PxTravelledPerFrame;
			--Play a radical sound if a MO is met
			if SceneMan:CastMORay(self.MuzzlePos, trace, self.ID, self.Team, 0, false, 1) ~= rte.NoMOID then
				self.hitSound:Play(self.MuzzlePos);
			elseif not overhead and resource < 10 then
				--Gather materials and turn them into sandbags
				local rayCount = 3;
				local hits = 0;
				for i = 1, rayCount do
					local hitPos = self.MuzzlePos;
					local terrRay = SceneMan:CastStrengthRay(self.MuzzlePos, Vector(trace.X, trace.Y):DegRotate(self.ParticleSpreadRange * 0.5 - self.ParticleSpreadRange * (i/rayCount)), 30, hitPos, 3, rte.grassID, SceneMan.SceneWrapsX);
					if terrRay then
						local material = SceneMan:GetMaterialFromID(SceneMan:GetTerrMatter(hitPos.X, hitPos.Y)).PresetName;
						for _, terrainMaterial in pairs(self.suitableMaterials) do
							if material == terrainMaterial then
								hits = hits + 1;
								break;
							end
						end
						if hits > rayCount * 0.5 then
							resource = resource + 1;
							parent:SetNumberValue("RoninShovelResource", resource);
							parent.PieMenu:GetFirstPieSliceByPresetName("Ronin Shovel Fill Sandbag PieSlice").Enabled = resource >= 10;
							self.collectSound:Play(self.Pos);
							break;
						end
					end
				end
			end

			self.InheritedRotAngleOffset = overhead and -0.1 or self.angleSize;
		end

		--This trick disables the collision for the weapon while the Magazine has no collision but looks exactly the same
		self.Scale = 0;
		if self.Magazine then
			self.Magazine.Scale = 1;

			self.Magazine.RoundCount = resource > 0 and resource or -1;
		end

		self.RateOfFire = self.minimumRoF + (self.minimumRoF) * (parent.Health/parent.MaxHealth);
	else
		self.Scale = 1;
		if self.Magazine then
			self.Magazine.Scale = 0;
		end

		if self.lastVel:MagnitudeIsGreaterThan(25) then
			local spread = 0.3 + self.AngularVel * TimerMan.DeltaTimeSecs * 0.5;
			local mo = MovableMan:GetMOFromID(self.HitWhatMOID);
			if mo then
				local particleCount = math.sqrt(self.lastVel.Magnitude);
				for i = 0, particleCount - 1 do
					local damagePar = CreateMOPixel("Smack Particle Light", "Base.rte");
					damagePar.Mass = self.Mass--/particleCount;
					damagePar.Sharpness = self.Sharpness-- * particleCount;

					damagePar.Pos = self.lastPos;
					damagePar.Vel = Vector(self.lastVel.X, self.lastVel.Y):RadRotate(spread * 0.5 - spread * i/(particleCount - 1)) * 1.5;

					damagePar:SetWhichMOToNotHit(self, -1);
					MovableMan:AddParticle(damagePar);
				end

				self.hitSound:Play(self.MuzzlePos);
			end
			for i = 1, self.lastVel.Magnitude * 0.5 do
				local dig = CreateMOPixel("Particle Ronin Shovel", "Ronin.rte");
				dig.Pos = self.Pos;
				dig.Vel = Vector(self.lastVel.X, self.lastVel.Y):RadRotate(spread * RangeRand(-1, 1)) * RangeRand(0.5, 1.0);
				MovableMan:AddParticle(dig);
			end
		end
	end
	
	self.StanceOffset = Vector(self.InheritedRotAngleOffset * 5, 0) + Vector(self.origStanceOffset.X + self.InheritedRotAngleOffset * 5, self.origStanceOffset.Y):RadRotate(self.angleSize * 0.5 * self.InheritedRotAngleOffset * 0.8);
	self.SharpStanceOffset = Vector(self.origSharpStanceOffset.X + self.InheritedRotAngleOffset * 5, self.origSharpStanceOffset.Y):RadRotate(self.angleSize * 0.5 * self.InheritedRotAngleOffset * 0.8);

	self.lastVel = Vector(self.Vel.X, self.Vel.Y);
	self.lastPos = Vector(self.Pos.X, self.Pos.Y);
end