function Create(self)
	--Get the target from Sharpness.
	local mo = MovableMan:FindObjectByUniqueID(self.Sharpness);
	if mo and IsMOSRotating(mo) then
		self.target = ToMOSRotating(mo);
	end
	if self.target == nil then
		self.ToDelete = true;
		return;
	end

	self.healing = self.target.Team == self.Team;
	self.healMultiplier = self.target.ModuleName == "Techion.rte" and 1.0 or 0.5;

	if self.target then
		--Current number of damage pulses.
		self.pulses = 0;

		--Maximum number of damage pulses to go through before stopping.
		self.maxPulses = 15;

		--Chance of flickering each frame.
		self.flickerChance = 0.9;

		--Length of time between pulses of damage.
		self.pulseTime = 300;

		--Timer for damage.
		self.pulseTimer = Timer();

		--Offset information.
		self.Pos = self.Pos + self.Vel * rte.PxTravelledPerFrame;

		if self.target then
			self.targetOffset = SceneMan:ShortestDistance(self.target.Pos, self.Pos, true);
			self.hitAngle = self.target.RotAngle;
		else
			self.targetOffset = Vector();
			self.hitAngle = 0;
		end

		self:EnableEmission(true);
	end
end

function Update(self)
	if self.target and IsMOSRotating(self.target) and not self.target.ToDelete then
		self.Pos = self.target.Pos + Vector(self.targetOffset.X, self.targetOffset.Y):RadRotate(self.target.RotAngle - self.hitAngle);

		--Flicker.
		if math.random() <= self.flickerChance then
			local flicker = CreateMOPixel("Techion.rte/Nanobot Flicker");
			local offset = Vector(ToMOSprite(self.target):GetSpriteWidth() * RangeRand(-0.5, 0.5), ToMOSprite(self.target):GetSpriteHeight() * RangeRand(-0.5, 0.5)):RadRotate(self.target.RotAngle);
			flicker.Pos = self.target.Pos + offset;
			flicker.Vel = offset * (-0.1);
			MovableMan:AddParticle(flicker);
		end

		--Cause damage to enemies, or heal friendlies.
		if self.pulseTimer:IsPastSimMS(self.pulseTime + self.target.Material.StructuralIntegrity * 2) then
			if IsAttachable(self.target) then
				self.nextTarget = ToAttachable(self.target):GetParent();
				self.nextTargetOffset = ToAttachable(self.target).ParentOffset;
			end

			if self.healing then
				if self.target.WoundCount > 0 then
					local damage = self.target:RemoveWounds(1);
					local parent = self.target:GetParent() or self.target;
					if IsActor(parent) then
						ToActor(parent):AddHealth(damage * self.healMultiplier);
					end
				else
					--Move on to the next target MO to repair.
					self.target = nil;
				end
			else
				local woundName = ToMOSRotating(self.target):GetEntryWoundPresetName();
				if woundName ~= "" then
					local wound = CreateAEmitter(woundName);
					wound.InheritedRotAngleOffset = self.targetOffset.AbsRadAngle + RangeRand(-0.1, 0.1);
					ToMOSRotating(self.target):AddWound(wound, self.targetOffset + Vector(math.random(-1, 1), math.random(-1, 1)), true);
				end
			end

			self.pulses = self.pulses + 1;
			self.pulseTimer:Reset();
		end
		
		if self.pulses > self.maxPulses then
			self.ToDelete = true;
		end
	elseif self.nextTarget then
		self.target = self.nextTarget;
		self.targetOffset = self.nextTargetOffset;
		self.hitAngle = 0;

		self.nextTarget = nil;
	else
		if self.healing then
			self.ToDelete = true;
		else
			self:GibThis();
		end
	end
end