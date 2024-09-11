function Create(self)
	self.penetrationStrength = 200;
	self.disintegrationStrength = 250;
	self.EffectRotAngle = self.Vel.AbsRadAngle;
	--Check backward (second argument) on the first frame as the projectile might be bouncing off something immediately
	PulsarDissipate(self, true);

	self.trailPar = CreateMOPixel(self.PresetName .. " Trail Glow", "Techion.rte");
	self.trailPar.Pos = self.Pos - (self.Vel * rte.PxTravelledPerFrame);
	self.trailPar.Vel = self.Vel * 0.1;
	self.trailPar.Lifetime = 60;
	MovableMan:AddParticle(self.trailPar);
end

function Update(self)
	self.ToSettle = false;
	if self.explosion then
		self.ToDelete = true;
	else
		self.EffectRotAngle = self.Vel.AbsRadAngle;
		if self.trailPar and MovableMan:IsParticle(self.trailPar) then
			self.trailPar.Pos = self.Pos - Vector(self.Vel.X, self.Vel.Y):SetMagnitude(self.TrailLength - 1);
			self.trailPar.Vel = self.Vel * 0.5;
			self.trailPar.Lifetime = self.Age + TimerMan.DeltaTimeMS;
		else
			self.trailPar = nil;
		end
	end
end

function PulsarDissipate(self, inverted)
	local trace = inverted and Vector(-self.Vel.X, -self.Vel.Y):SetMagnitude(GetPPM()) or Vector(self.Vel.X, self.Vel.Y):SetMagnitude(self.Vel.Magnitude * rte.PxTravelledPerFrame + 1);
	local hit = inverted == false;
	local hitPos = Vector(self.Pos.X, self.Pos.Y);
	local skipPx = math.sqrt(self.Vel.Magnitude) * 0.5;

	local moid = SceneMan:CastObstacleRay(self.Pos, trace, hitPos, Vector(), self.ID, self.Team, rte.airID, skipPx) >= 0 and SceneMan:GetMOIDPixel(hitPos.X, hitPos.Y) or self.HitWhatMOID;
	local mo = MovableMan:GetMOFromID(moid);
	
	if mo and mo.Team ~= self.Team then
		hit = true;
		if IsMOSRotating(mo) and self.penetrationStrength > mo.Material.StructuralIntegrity then
			mo = ToMOSRotating(mo);
			local woundName = mo:GetEntryWoundPresetName();
			if woundName ~= "" then
				local wound = CreateAEmitter(woundName);
				wound.BurstDamage = wound.BurstDamage * self.WoundDamageMultiplier;
				local woundOffset = SceneMan:ShortestDistance(mo.Pos, hitPos, SceneMan.SceneWrapsX);
				woundOffset.X = woundOffset.X * mo.FlipFactor;
				wound.InheritedRotAngleOffset = woundOffset.AbsRadAngle;
				mo:AddWound(wound, woundOffset:RadRotate(-mo.RotAngle * mo.FlipFactor), true);
				
				local melter = CreateMOPixel("Disintegrator", "Techion.rte");
				melter.Pos = self.Pos;
				melter.Team = self.Team;
				melter.Sharpness = mo.RootID;
				melter.PinStrength = self.disintegrationStrength or 1;
				MovableMan:AddMO(melter);
			end
		end
	elseif self.Vel:MagnitudeIsLessThan(1) then
		hit = true;
	end

	if hit then
		local offset = Vector(self.Vel.X, self.Vel.Y):SetMagnitude(skipPx);
		self.explosion = CreateAEmitter("Techion.rte/Laser Dissipate Effect");
		self.explosion.Pos = hitPos - offset;
		self.explosion.RotAngle = offset.AbsRadAngle;
		self.explosion.Team = self.Team;
		self.explosion.Vel = offset;
		MovableMan:AddParticle(self.explosion);

		self.TrailLength = 0;
		self.ToDelete = true;
	end
	
	return hit;
end

function OnCollideWithMO(self, mo, parentMO)
	PulsarDissipate(self, false);
end

function OnCollideWithTerrain(self, terrainID)
	PulsarDissipate(self, false);
end