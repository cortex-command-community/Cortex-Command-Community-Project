function Create(self)
	self.Sharpness = self.Sharpness * RangeRand(0.9, 1.1);
	self.width = math.ceil(self:GetSpriteWidth() * 0.5);
end

function OnCollideWithMO(self, mo, rootMO)
	if not self.target then
		local trajectoryScalar = math.abs(math.cos(self.RotAngle - self.Vel.AbsRadAngle));

		local dots = math.sqrt(self.PrevVel.Magnitude);
		local trace = Vector((self.PrevVel.Magnitude * rte.PxTravelledPerFrame + self.width) * trajectoryScalar * self.FlipFactor, 0):RadRotate(self.PrevRotAngle);
		for i = 1, dots do
			local checkPos = self.PrevPos + trace * (i/dots);
			local checkPix = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
			if checkPix ~= rte.NoMOID then
				local foundMO = MovableMan:GetMOFromID(checkPix);
				if self.Mass * self.PrevVel.Magnitude * self.Sharpness > foundMO.Material.StructuralIntegrity then
					checkPos = checkPos + SceneMan:ShortestDistance(checkPos, self.PrevPos, SceneMan.SceneWrapsX):SetMagnitude(self.width * 0.5);
					self.target = ToMOSRotating(foundMO);
					local dist = SceneMan:ShortestDistance(self.target.Pos, checkPos, SceneMan.SceneWrapsX);
					local stickOffset = Vector(dist.X * self.target.FlipFactor, dist.Y):RadRotate(-self.target.RotAngle * self.target.FlipFactor);

					local woundName = self.target:GetEntryWoundPresetName();
					local damageMultiplier = math.sqrt(self.target.WoundDamageMultiplier) + self.WoundDamageMultiplier;
					if woundName ~= "" then
						local wound = CreateAEmitter(woundName);
						wound.EmitCountLimit = math.ceil(wound.EmitCountLimit * 0.5);
						wound.BurstDamage = wound.BurstDamage * damageMultiplier;
						for em in wound.Emissions do
							em.BurstSize = em.BurstSize * damageMultiplier;
							em.ParticlesPerMinute = em.ParticlesPerMinute * 0.5;
							em.MaxVelocity = em.MaxVelocity * 0.5;
							em.MinVelocity = em.MinVelocity * 0.5;
						end
						wound.InheritedRotAngleOffset = stickOffset.AbsRadAngle;
						self.target:AddWound(wound, Vector(stickOffset.X, stickOffset.Y):SetMagnitude(stickOffset.Magnitude - self.width * 0.5), math.random() < 0.5);
					end
					self.Team = self.target.Team;
					self.InheritsHFlipped = (self.HFlipped == self.target.HFlipped) and 1 or -1;
					self.DrawAfterParent = math.random() * self.target.Radius < self.Radius;

					self.InheritedRotAngleOffset = (self.PrevRotAngle - self.target.RotAngle) * self.target.FlipFactor;
					self.target:AddAttachable(self:Clone(), stickOffset);
					self.ToDelete = true;
				end
				break;
			end
		end
	end
end
function OnAttach(self, parent)
	self.target = parent;
end