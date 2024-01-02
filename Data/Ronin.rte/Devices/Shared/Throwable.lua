function Create(self)
	self.impactPower = self:NumberValueExists("ImpactPower") and self:GetNumberValue("ImpactPower") or self.Mass;
end

function OnDetach(self, exParent)
	self.AngularVel = self.AngularVel - self.Vel.Magnitude * self.FlipFactor * math.random();
	self.thrown = true;
end

function OnCollideWithMO(self, mo, rootMO)
	if self.thrown and IsMOSprite(mo) then
		local force = self.PrevVel * self.impactPower;
		mo:AddImpulseForce(force, Vector());
		if force.Magnitude * self.Sharpness > mo.Material.StructuralIntegrity then
			local woundName = mo:GetEntryWoundPresetName();
			if woundName ~= "" then
				local wound = CreateAEmitter(woundName);
				local dist = SceneMan:ShortestDistance(mo.Pos, self.Pos, SceneMan.SceneWrapsX);
				local stickOffset = Vector(dist.X * mo.FlipFactor, dist.Y):RadRotate(-mo.RotAngle * mo.FlipFactor);
				wound.InheritedRotAngleOffset = stickOffset.AbsRadAngle;
				mo:AddWound(wound, Vector(stickOffset.X, stickOffset.Y):SetMagnitude(stickOffset.Magnitude - self.Radius), true);
			end
		end
		self.thrown = false;
	end
end