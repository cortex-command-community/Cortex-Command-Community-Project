function Create(self)
	self.explodeTimer = Timer();
	self.partList = {};

	self.explodeTime = 1500;
	self.numOfParticles = 50;
	self.particlesPerOil = 10;

	self:EraseFromTerrain();

	for i = 1, self.numOfParticles do
		self.partList[i] = CreateMOPixel("Browncoat Boss Oil Bomb Fuel");
		self.partList[i].Pos = self.Pos;
		self.partList[i].Vel = Vector(math.random(20), 0):RadRotate(math.pi * 2 * math.random()) + self.Vel;
		MovableMan:AddParticle(self.partList[i]);
		self.partList[i].queue = math.abs(self.partList[i].Vel.X - self.Vel.X) * TimerMan.DeltaTimeMS;

		if i < self.numOfParticles * 0.5 then
			local part = CreateMOSParticle("Oil Spray Particle");
			part.Pos = self.Pos;
			part.Lifetime = part.Lifetime*RangeRand(0.8, 1.2);
			part.Vel = self.partList[i].Vel;
			MovableMan:AddParticle(part);
		end
	end

	self.toPlaySound = true;
	if self:NumberValueExists("Secondary") then
		self.toPlaySound = false;
	end
end

function Update(self)
	self.ToSettle = false;
	self.ToDelete = false;

	if self.explodeTimer:IsPastSimMS(self.explodeTime) then
		if self.toPlaySound == true then
			self.toPlaySound = false;
			local sfx = CreateMOSRotating("Browncoat Boss Oil Bomb Ignition");
			sfx.Pos = self.Pos;
			sfx:GibThis();
			MovableMan:AddParticle(sfx);
		end

		local partsLeft = 0;
		for i = 1, #self.partList do
			if self.partList[i] and MovableMan:IsParticle(self.partList[i]) and self.partList[i].PresetName == "Browncoat Boss Oil Bomb Fuel" then
				if self.explodeTimer:IsPastSimMS(self.explodeTime + self.partList[i].queue) then
					local fire = CreatePEmitter("Flame ".. math.random(2) .." Hurt Browncoat Boss Oil Bomb");
					fire.Team = self.Team;
					if self.partList[i].target and self.partList[i].target.ID ~= rte.NoMOID and not self.partList[i].target.ToDelete then
						fire.Pos = self.partList[i].target.Pos + self.partList[i].stickOffset;
						fire.Vel = Vector(-self.partList[i].stickOffset.X, -self.partList[i].stickOffset.Y):SetMagnitude(3);
					else
						fire.Pos = Vector(self.partList[i].Pos.X, self.partList[i].Pos.Y);
						fire.Vel = self.Vel;
						fire.Lifetime = math.random(14000, 16000);
					end

					MovableMan:AddParticle(fire);
					for j = 1, self.particlesPerOil do
						local firePar;
						if j > self.particlesPerOil * 0.5 then
							firePar = CreateMOPixel("Ground Fire Burn Particle");
							firePar.Team = self.Team;
							firePar.IgnoresTeamHits = true;
							firePar.Vel = self.Vel + Vector(RangeRand(-20, 20), -math.random(-10, 30));
						else
							firePar = CreateMOSParticle("Flame Smoke 2");
							firePar.Team = self.Team;
							firePar.Vel = self.Vel + Vector(math.random() * j, 0):RadRotate(math.random() * math.pi * 2);
							firePar.Lifetime = math.random(500, 1000);
							firePar.GlobalAccScalar = RangeRand(-0.6, -0.1);
						end
						firePar.Pos = Vector(self.partList[i].Pos.X, self.partList[i].Pos.Y);
						MovableMan:AddParticle(firePar);
					end
					
					self.partList[i].ToDelete = true;
				else
					partsLeft = partsLeft + 1;
				end
			end
		end

		if partsLeft == 0 then
			self.ToDelete = true;
		end
	else
		--Look for targets to douse with fuel
		for i = 1, #self.partList do
			if self.partList[i] and MovableMan:IsParticle(self.partList[i]) and self.partList[i].PresetName == "Browncoat Boss Oil Bomb Fuel" then

				if self.partList[i].target and self.partList[i].target.ID ~= rte.NoMOID and not self.partList[i].target.ToDelete then

					if math.random() < 0.01 then
						self.partList[i].Vel = self.partList[i].target.Vel;
						self.partList[i].Pos = self.partList[i].target.Pos + Vector(self.partList[i].stickOffset.X, self.partList[i].stickOffset.Y):RadRotate(self.partList[i].target.RotAngle - self.partList[i].targetStickAngle);
					end
				else
					self.partList[i].target = nil;
					local velNum = math.ceil(math.sqrt(self.partList[i].Vel.Magnitude + 1));

					local mocheck = SceneMan:CastMORay(self.partList[i].Pos, Vector(velNum, 0):RadRotate(self.partList[i].Vel.AbsRadAngle), self.partList[i].ID, -2, rte.airID, true, 1);
					if mocheck ~= rte.NoMOID then
						local mo = MovableMan:GetMOFromID(MovableMan:GetMOFromID(mocheck).ID);
						if mo and mo.Team ~= self.Team and mo.PresetName ~= self.PresetName then

							self.partList[i].target = ToMOSRotating(mo);

							self.partList[i].targetStickAngle = mo.RotAngle;

							self.partList[i].stickOffset = SceneMan:ShortestDistance(mo.Pos, self.partList[i].Pos, SceneMan.SceneWrapsX) * 0.8;
						end
					end
				end
			else
				self.partList[i] = nil;
			end
		end
	end
end