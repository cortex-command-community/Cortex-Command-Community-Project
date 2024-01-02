function Create(self)
	self.width = math.floor(ToMOSprite(self):GetSpriteWidth() * 0.5 + 0.5);
	self.ignoreDelay = TimerMan.DeltaTimeMS * 3;
end

function OnCollideWithMO(self, mo, rootMO)
	if not self.hitTimer or self.hitTimer:IsPastSimMS(self.ignoreDelay) then
		local dots = math.sqrt(self.PrevVel.Magnitude);
		local trace = Vector((self.PrevVel.Magnitude * rte.PxTravelledPerFrame + self.width) * math.abs(math.cos(self.RotAngle - self.Vel.AbsRadAngle)) * self.FlipFactor, 0):RadRotate(self.PrevRotAngle);
		for i = 1, dots do
			local checkPos = self.PrevPos + trace * (i/dots);
			local checkPix = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
			if checkPix ~= rte.NoMOID then
				self.hitTimer = Timer();
				local mo = ToMOSRotating(MovableMan:GetMOFromID(checkPix));
				local penetration = (self.Mass * self.PrevVel.Magnitude * self.Sharpness)/math.max(mo.Material.StructuralIntegrity, 1);
				if penetration >= 1 then
					checkPos = checkPos + SceneMan:ShortestDistance(checkPos, self.PrevPos, SceneMan.SceneWrapsX):SetMagnitude(self.width * 0.5);
					
					local dist = SceneMan:ShortestDistance(mo.Pos, checkPos, SceneMan.SceneWrapsX);
					local setOffset = Vector(dist.X * mo.FlipFactor, dist.Y):RadRotate(-mo.RotAngle * mo.FlipFactor):SetMagnitude(dist.Magnitude - self.width);
					local setAngle = trace.AbsRadAngle - mo.RotAngle;

					local woundName = mo:GetEntryWoundPresetName();
					local multiplier = math.min(penetration, self.WoundDamageMultiplier);
					local mildMultiplier = math.sqrt(multiplier);
					local milderMultiplier = math.sqrt(mildMultiplier);
					if woundName ~= "" then
						local wound = CreateAEmitter(woundName);
						wound.BurstScale = wound.BurstScale * multiplier;
						wound.DamageMultiplier = multiplier;
						wound.EmitCountLimit = math.ceil(wound.EmitCountLimit * mildMultiplier);
						wound.Scale = wound.Scale * milderMultiplier;
						if wound.BurstSound then
							wound.BurstSound.Pitch = wound.BurstSound.Pitch/milderMultiplier;
							wound.BurstSound.Volume = wound.BurstSound.Volume * milderMultiplier;
						end
						for em in wound.Emissions do
							em.BurstSize = em.BurstSize * milderMultiplier;
							em.ParticlesPerMinute = em.ParticlesPerMinute * mildMultiplier;
						end
						wound.InheritedRotAngleOffset = setAngle;
						wound.DrawAfterParent = true;
						mo:AddWound(wound, setOffset, true);
					end

					if penetration > 2 then
						woundName = mo:GetExitWoundPresetName();
						if woundName ~= "" then
							local wound = CreateAEmitter(woundName);
							wound.BurstScale = wound.BurstScale * multiplier;
							wound.DamageMultiplier = multiplier;
							wound.EmitCountLimit = math.ceil(wound.EmitCountLimit * mildMultiplier);
							wound.Scale = wound.Scale * milderMultiplier;
							if wound.BurstSound then
								wound.BurstSound.Pitch = wound.BurstSound.Pitch/milderMultiplier;
								wound.BurstSound.Volume = wound.BurstSound.Volume * milderMultiplier;
							end
							for em in wound.Emissions do
								em.BurstSize = em.BurstSize * milderMultiplier;
								em.ParticlesPerMinute = em.ParticlesPerMinute * mildMultiplier;
							end
							wound.InheritedRotAngleOffset = setAngle;
							wound.DrawAfterParent = true;
							mo:AddWound(wound, setOffset, true);
						end
					end
					self:SetWhichMOToNotHit(mo:GetRootParent(), self.ignoreDelay);
				end
				self.Vel = Vector(self.Vel.X, self.Vel.Y):SetMagnitude(self.Vel.Magnitude^0.9);
				break;
			end
		end
	end
end