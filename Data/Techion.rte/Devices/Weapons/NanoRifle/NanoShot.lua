function Create(self)
	--Collide with objects and deploy the destroy effect.
	self.CheckCollision = function(inverted)
		local trace = self.Vel * rte.PxTravelledPerFrame;
		if inverted then
			trace = trace * (-1);
		end
		local moid = SceneMan:CastMORay(self.Pos, trace, rte.NoMOID, -2, rte.airID, true, 1);
		if moid ~= rte.NoMOID then
			local hitPos = Vector();
			SceneMan:CastFindMORay(self.Pos, trace, moid, hitPos, rte.airID, true, 1);
			local target = ToMOSRotating(MovableMan:GetMOFromID(moid));
			if target and (target.Team ~= self.Team or (target.WoundCount > 0 and target.PresetName ~= "Nano Rifle")) then
				self.deleteNextFrame = true;
				self.Vel = (SceneMan:ShortestDistance(self.Pos, hitPos, true)/TimerMan.DeltaTimeSecs)/GetPPM();

				local effect = CreateAEmitter("Nanobot Effect", "Techion.rte");
				effect.Sharpness = target.UniqueID;
				effect.Pos = hitPos + SceneMan:ShortestDistance(hitPos, target.Pos, true):SetMagnitude(3);
				effect.Vel = target.Vel;
				effect.Team = self.Team;
				MovableMan:AddParticle(effect);
			end
		end
	end
	--Speed at which this can actually activate.
	self.speedThreshold = 100;

	self.deleteNextFrame = false;
	--Check backward.
	self.CheckCollision(true);

	self.trailLength = 50;

	self.trailPar = CreateMOPixel("Nano Rifle Trail Glow");
	self.trailPar.Pos = self.Pos;
	self.trailPar.Vel = self.Vel * 0.1;
	self.trailPar.Lifetime = 60;
	MovableMan:AddParticle(self.trailPar);

	self.lastVel = Vector(self.Vel.X, self.Vel.Y);
end

function Update(self)
	if not self.ToDelete and self.trailPar and MovableMan:IsParticle(self.trailPar) then
		self.trailPar.Pos = self.Pos - Vector(self.lastVel.X, self.lastVel.Y):SetMagnitude(math.min(self.lastVel.Magnitude * rte.PxTravelledPerFrame, self.trailLength) * 0.5);
		self.trailPar.Vel = self.lastVel * 0.5;
		self.trailPar.Lifetime = self.Age + TimerMan.DeltaTimeMS;
	end

	if self.deleteNextFrame then
		self.ToDelete = true;
	elseif self.Vel:MagnitudeIsGreaterThan(self.speedThreshold) then
		--Check forward.
		self.CheckCollision(false);
	end
	
	self.lastVel = Vector(self.Vel.X, self.Vel.Y);
end

function OnCollideWithTerrain(self, terrainID)
	if SceneMan:GetMaterialFromID(terrainID).StructuralIntegrity > (self.Vel.Magnitude * self.Mass * self.Sharpness) * 0.5 then
		self.ToDelete = true;
	end
end