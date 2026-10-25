function Create(self)
	local var = {};

	--Range of the shot.
	var.range = 600;

	--Amplitude of the wave.
	var.maxAmplitude = math.floor(1 + math.sqrt(self.Vel.Magnitude) + 0.5);

	--Wavelength of the wave.
	var.flaceLength = math.floor(3 + math.sqrt(self.Vel.Magnitude) + 0.5);

	--Track intersecting beams
	var.lastAmplitude = 1;

	--Speed of the wave (pixels per second).
	var.speed = self.Vel.Magnitude * 10;

	--Speed of damage particles.
	var.damageSpeed = 50 + self.Vel.Magnitude * 0.1;

	--Maximum strength for material penetration (both MOs and terrain).
	var.strengthThreshold = 50 + self.Vel.Magnitude * 0.3;

	--Direction of the wave.
	var.direction = Vector(self.Vel.X, self.Vel.Y);
	var.direction:SetMagnitude(1);
	var.up = Vector(var.direction.X, var.direction.Y);
	var.up:RadRotate(math.pi * 0.5);

	--Interval at which to create damage particles.
	var.damageInterval = 3;

	--Timer for the wave.
	var.timer = Timer();

	--The last starting position along the line.
	var.lastI = 0;

	--Count MO and terrain hits.
	var.hits = 0;

	--Disintegration strength.
	var.disintegrationStrength = 500;
	
	var.melter = CreateMOPixel("Disintegrator", "Techion.rte");

	var.Pos = self.Pos;
	self.var = var;
end

function Update(self)
	local var = self.var;
	local endPoint = var.timer.ElapsedSimTimeS * var.speed;
	if endPoint > var.range then
		endPoint = var.range;
		self.ToDelete = true;
	else
		self.ToDelete = false;
		self.ToSettle = false;
	end

	endPoint = math.floor(endPoint);

	--Draw out the path.
	for i = var.lastI, endPoint, 1 do
		local amplitude = math.sin((i/var.flaceLength) * 2 * math.pi) * var.maxAmplitude;
		local waveOffset = Vector(var.up.X, var.up.Y);
		waveOffset:SetMagnitude(amplitude);

		local linePos = var.Pos + Vector(var.direction.X, var.direction.Y):SetMagnitude(i * 2);
		local fireVector = Vector(var.direction.X, var.direction.Y):SetMagnitude(var.damageSpeed);
		local upPos = linePos + waveOffset;
		local downPos = linePos - waveOffset * 0.2;

		--Cancel the beam if there's a terrain collision.
		local trace = Vector(fireVector.X, fireVector.Y):SetMagnitude(var.damageSpeed * 0.1);

		local strSumRay = SceneMan:CastStrengthSumRay(downPos, downPos + trace, 3, 160);
		var.hits = var.hits + math.sqrt(strSumRay);

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

		if i % var.damageInterval == 0 then
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
						local melter = var.melter:Clone();
						melter.Pos = hitPos;
						melter.Team = self.Team;
						melter.Sharpness = rootMO.ID;
						melter.PinStrength = var.disintegrationStrength;
						MovableMan:AddMO(melter);
					end

					var.hits = var.hits + (i == 2 and math.sqrt(mo.Material.StructuralIntegrity) + math.sqrt(mo.Radius + mo.Mass) * 0.1 or 1);
					
					--Add the dissipate effect.
					local effect = CreateAEmitter("Techion.rte/Laser Dissipate Effect");
					effect.Pos = checkPos;
					MovableMan:AddParticle(effect);
					effect:GibThis();
				end
			end
		end

		if (var.lastAmplitude > 0 and amplitude < 0) or (var.lastAmplitude < 0 and amplitude > 0) then
			local part = CreateMOPixel("Techion.rte/Dihelical Cannon Large Effect Particle");
			part.Pos = linePos - Vector(var.direction.X, var.direction.Y):SetMagnitude(2);
			part.Vel = fireVector * 0.1;
			MovableMan:AddParticle(part);
		end

		var.lastAmplitude = amplitude;
		if var.hits > var.strengthThreshold then

			local effect = CreateAEmitter("Techion.rte/Dihelical Cannon Impact Particle");
			effect.Pos = linePos - Vector(var.direction.X, var.direction.Y):SetMagnitude(2);
			effect.Team = self.Team;
			effect.IgnoresTeamHits = true;
			MovableMan:AddParticle(effect);

			self.ToDelete = true;
			break;
		else
			var.hits = var.hits * 0.9;
		end

		var.flaceLength = var.flaceLength * (1 + 0.05/var.flaceLength);
		var.maxAmplitude = var.flaceLength * 0.5;
	end
	
	var.lastI = endPoint;
end