function Create(self)
	--Range of the shot.
	self.range = 600;

	--Amplitude of the wave.
	self.maxAmplitude = math.floor(1 + math.sqrt(self.Vel.Magnitude) + 0.5);

	--Wavelength of the wave.
	self.flaceLength = math.floor(3 + math.sqrt(self.Vel.Magnitude) + 0.5);

	--Track intersecting beams
	self.lastAmplitude = 1;

	--Speed of the wave (pixels per second).
	self.speed = self.Vel.Magnitude * 10;

	--Speed of damage particles.
	self.damageSpeed = 50 + self.Vel.Magnitude * 0.1;

	--Maximum strength for material penetration (both MOs and terrain).
	self.strengthThreshold = 50 + self.Vel.Magnitude * 0.3;

	--Direction of the wave.
	self.direction = Vector(self.Vel.X, self.Vel.Y);
	self.direction:SetMagnitude(1);
	self.up = Vector(self.direction.X, self.direction.Y);
	self.up:RadRotate(math.pi * 0.5);

	--Interval at which to create damage particles.
	self.damageInterval = 3;

	--Timer for the wave.
	self.timer = Timer();

	--The last starting position along the line.
	self.lastI = 0;

	--Count MO and terrain hits.
	self.hits = 0;

	--Disintegration strength.
	self.disintegrationStrength = 500;
	
	self.melter = CreateMOPixel("Disintegrator", "Techion.rte");
end

function Update(self)
	local endPoint = self.timer.ElapsedSimTimeS * self.speed;
	if endPoint > self.range then
		endPoint = self.range;
		self.ToDelete = true;
	else
		self.ToDelete = false;
		self.ToSettle = false;
	end

	endPoint = math.floor(endPoint);

	--Draw out the path.
	for i = self.lastI, endPoint, 1 do
		local amplitude = math.sin((i/self.flaceLength) * 2 * math.pi) * self.maxAmplitude;
		local waveOffset = Vector(self.up.X, self.up.Y);
		waveOffset:SetMagnitude(amplitude);

		local linePos = self.Pos + Vector(self.direction.X, self.direction.Y):SetMagnitude(i * 2);
		local fireVector = Vector(self.direction.X, self.direction.Y):SetMagnitude(self.damageSpeed);
		local upPos = linePos + waveOffset;
		local downPos = linePos - waveOffset * 0.2;

		--Cancel the beam if there's a terrain collision.
		local trace = Vector(fireVector.X, fireVector.Y):SetMagnitude(self.damageSpeed * 0.1);

		local strSumRay = SceneMan:CastStrengthSumRay(downPos, downPos + trace, 3, 160);
		self.hits = self.hits + math.sqrt(strSumRay);

		if SceneMan:GetTerrMatter(upPos.X, upPos.Y) == rte.airID then
			--Add the blue wave effect.
			local partA = CreateMOPixel("Techion.rte/Dihelical Cannon Effect Particle");
			partA.Pos = upPos;
			partA.Vel = (fireVector * 0.5 - waveOffset) * 0.2;
			MovableMan:AddParticle(partA);

			--Add the wave front effect.
			local frontA = CreateMOPixel("Techion.rte/Dihelical Cannon Front Effect Particle");
			frontA.Pos = upPos;
			frontA.Vel = (fireVector * 0.5 - waveOffset) * 0.2;
			MovableMan:AddParticle(frontA);
		end

		--Add the blue wave effect.
		local partB = CreateMOPixel("Techion.rte/Dihelical Cannon Effect Particle");
		partB.Pos = downPos;
		partB.Vel = (fireVector + waveOffset) * 0.04;
		MovableMan:AddParticle(partB);

		--Add the wave front effect.
		local frontB = CreateMOPixel("Techion.rte/Dihelical Cannon Front Effect Particle");
		frontB.Pos = downPos;
		frontB.Vel = (fireVector + waveOffset) * 0.04;
		MovableMan:AddParticle(frontB);

		if i % self.damageInterval == 0 then
			local pos = {upPos, downPos};
			local hitID = rte.NoMOID;
			for i = 1, #pos do
				local checkPos = pos[i];
				local hitPos = Vector();
				local hitID = SceneMan:CastObstacleRay(checkPos, fireVector * rte.PxTravelledPerFrame, hitPos, Vector(), hitID, self.Team, rte.airID, 2) >= 0 and SceneMan:GetMOIDPixel(hitPos.X, hitPos.Y) or rte.NoMOID;
				local mo = MovableMan:GetMOFromID(hitID);
				if mo and IsMOSRotating(mo) then
					mo = ToMOSRotating(mo);
					
					local wounds = {mo:GetEntryWoundPresetName(), mo:GetExitWoundPresetName()};
					--Center beam creates both entry and exit wounds.
					for j = 1, i do
						local woundName = wounds[j];
						if woundName ~= "" and not mo.ToDelete then
							local wound = CreateAEmitter(woundName);
							wound.BurstDamage = wound.BurstDamage * self.WoundDamageMultiplier;
							local woundOffset = SceneMan:ShortestDistance(mo.Pos, hitPos, SceneMan.SceneWrapsX);
							woundOffset.X = woundOffset.X * mo.FlipFactor;
							wound.InheritedRotAngleOffset = woundOffset.AbsRadAngle;
							mo:AddWound(wound, woundOffset:RadRotate(-mo.RotAngle * mo.FlipFactor), true);
						end
					end

					local rootMO = mo:GetRootParent();
					if i == 2 and IsActor(rootMO) then
						local melter = self.melter:Clone();
						melter.Pos = hitPos;
						melter.Team = self.Team;
						melter.Sharpness = rootMO.ID;
						melter.PinStrength = self.disintegrationStrength;
						MovableMan:AddMO(melter);
					end

					self.hits = self.hits + (i == 2 and math.sqrt(mo.Material.StructuralIntegrity) + math.sqrt(mo.Radius + mo.Mass) * 0.1 or 1);
					
					--Add the dissipate effect.
					local effect = CreateAEmitter("Techion.rte/Laser Dissipate Effect");
					effect.Pos = checkPos;
					MovableMan:AddParticle(effect);
					effect:GibThis();
				end
			end
		end

		if (self.lastAmplitude > 0 and amplitude < 0) or (self.lastAmplitude < 0 and amplitude > 0) then
			local part = CreateMOPixel("Techion.rte/Dihelical Cannon Large Effect Particle");
			part.Pos = linePos - Vector(self.direction.X, self.direction.Y):SetMagnitude(2);
			part.Vel = fireVector * 0.1;
			MovableMan:AddParticle(part);
		end

		self.lastAmplitude = amplitude;
		if self.hits > self.strengthThreshold then

			local effect = CreateAEmitter("Techion.rte/Dihelical Cannon Impact Particle");
			effect.Pos = linePos - Vector(self.direction.X, self.direction.Y):SetMagnitude(2);
			effect.Team = self.Team;
			effect.IgnoresTeamHits = true;
			MovableMan:AddParticle(effect);

			self.ToDelete = true;
			break;
		else
			self.hits = self.hits * 0.9;
		end

		self.flaceLength = self.flaceLength * (1 + 0.05/self.flaceLength);
		self.maxAmplitude = self.flaceLength * 0.5;
	end
	
	self.lastI = endPoint;
end