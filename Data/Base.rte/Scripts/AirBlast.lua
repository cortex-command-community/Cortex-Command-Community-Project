function Create(self)
	self.strength = self.Mass * self.Vel.Magnitude;
	self.range = self.Lifetime * self.Vel.Magnitude;
end

function Update(self)
	--Run the effect on Update() to give other particles a chance to reach the target
	for mo in MovableMan:GetMOsInRadius(self.Pos, self.range, -1, true) do
		if mo.PinStrength == 0 and IsMOSRotating(mo) then
			local dist = SceneMan:ShortestDistance(self.Pos, mo.Pos, SceneMan.SceneWrapsX);
			local strSumCheck = SceneMan:CastStrengthSumRay(self.Pos, self.Pos + dist, 3, rte.airID);
			if strSumCheck < self.strength then
				local massFactor = math.sqrt(1 + math.abs(mo.Mass));
				local distFactor = 1 + dist.Magnitude * 0.1;
				local forceVector =	dist:SetMagnitude((self.strength - strSumCheck)/distFactor);
				if IsAttachable(mo) then
					--Diminish transferred impulses from attachables since we are likely already targeting its' parent
					forceVector = forceVector * math.abs(1 - ToAttachable(mo).JointStiffness);
				end
				mo.Vel = mo.Vel + forceVector/massFactor;
				mo.AngularVel = mo.AngularVel - forceVector.X/(massFactor + math.abs(mo.AngularVel));
				mo:AddImpulseForce(forceVector * massFactor, Vector());
				--Add some additional points of damage to actors
				if IsActor(mo) then
					local actor = ToActor(mo);
					local impulse = (forceVector.Magnitude * self.strength/massFactor) - actor.ImpulseDamageThreshold;
					local damage = impulse/(actor.GibImpulseLimit * 0.1 + actor.Material.StructuralIntegrity * 10);
					actor.Health = damage > 0 and actor.Health - damage or actor.Health;
					actor.Status = (actor.Status == Actor.STABLE and damage > (actor.Health * 0.7)) and Actor.UNSTABLE or actor.Status;
				end
			end
		end
	end
	
	self.ToDelete = true;
end