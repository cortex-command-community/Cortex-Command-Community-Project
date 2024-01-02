function Create(self)
	self.damageResistanceMultiplier = 1.5;
	self.withdrawalIncrement = 0.1;
	self.IgnoresTeamHits = false;
	self.Team = Activity.NOTEAM;
	self.width = 3;
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
					checkPos = checkPos + SceneMan:ShortestDistance(checkPos, self.PrevPos, SceneMan.SceneWrapsX):SetMagnitude(self.width);

					self.target = ToMOSRotating(foundMO);
					local dist = SceneMan:ShortestDistance(self.target.Pos, checkPos, SceneMan.SceneWrapsX);
					local stickOffset = Vector(dist.X * self.target.FlipFactor, dist.Y):RadRotate(-self.target.RotAngle * self.target.FlipFactor);

					local woundName = self.target:GetEntryWoundPresetName();
					if woundName ~= "" then
						local wound = CreateAEmitter(woundName);
						wound.BurstDamage = wound.BurstDamage * self.WoundDamageMultiplier;
						wound.EmitCountLimit = math.ceil(wound.EmitCountLimit * 0.5);
						wound.Scale = wound.Scale * 0.5;
						for em in wound.Emissions do
							em.ParticlesPerMinute = em.ParticlesPerMinute * 0.75;
							em.MaxVelocity = em.MaxVelocity * 0.75;
							em.MinVelocity = em.MinVelocity * 0.75;
						end
						wound.InheritedRotAngleOffset = stickOffset.AbsRadAngle;
						self.target:AddWound(wound, Vector(stickOffset.X, stickOffset.Y):SetMagnitude(stickOffset.Magnitude - self.width), math.random() < 0.5);
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
	if string.find(parent.Material.PresetName, "Flesh") then
		parent.DamageMultiplier = parent.DamageMultiplier/self.damageResistanceMultiplier;
		self:EnableEmission(true);
		parent = parent:GetRootParent();
		if IsActor(parent) and MovableMan:IsActor(parent) then
			local cross = CreateMOSParticle("Particle Heal Effect", "Base.rte");
			cross.Pos = ToActor(parent).AboveHUDPos + Vector(0, 4);
			MovableMan:AddParticle(cross);
		end
	end
	self.target = parent;
end

function OnDetach(self, parent)
	if parent and string.find(parent.Material.PresetName, "Flesh") then
		parent.DamageMultiplier = parent.DamageMultiplier * self.damageResistanceMultiplier + self.withdrawalIncrement;
		self:EnableEmission(false);
	end
end