--This MOPixel handles the Actor-disintegrating effect of Techion weapons
--TODO: Redo all of this convoluted crap
function Create(self)
	self.effectSpeed = 0.01;
	self.setScale = 1;
	self.minScale = 0.1;

	self.disintegrationSound = CreateSoundContainer("Disintegration Sound", "Techion.rte");

	local strength = math.random(self.PinStrength * 0.5, self.PinStrength);
	--Get assigned target from Sharpness because MOPixels cannot carry NumberValues
	local target = MovableMan:GetMOFromID(self.Sharpness);
	--[[Alternative: find suitable target from nearby
	local dist = Vector();
	local actor = MovableMan:GetClosestEnemyActor(self.Team, self.Pos, strength, dist);
	if actor then
		local sizeEstimate = (actor.Height + actor.Diameter)/3;
		if dist:MagnitudeIsLessThan(sizeEstimate) and sizeEstimate < 50 then
			target = ToActor(actor);
			strength = strength/math.sqrt(dist.Magnitude);
		end
	end]]--
	if target and IsActor(target) then
		local actor = ToActor(target);
		--Elaborate effect resistance strength calculations based on various Actor values
		local resistance = (actor.Mass - actor.InventoryMass) + actor.Radius + actor.Material.StructuralIntegrity + (actor.GibWoundLimit > 0 and actor.GibWoundLimit * (1 - actor:GetWoundCount(false, false, false)/actor.GibWoundLimit) or actor.GibImpulseLimit);
		if actor.Health < 0 and strength > resistance then
			--Leave this Actor alone if it already has a disintegrator particle assigned to it
			if actor:NumberValueExists("ToDisintegrate") then
				self.ToDelete = true;
			else
				actor.Health = actor.Health - actor.MaxHealth;
				actor:RemoveWounds(actor:GetWoundCount(true, true, true), true, true, true);
				actor.RestThreshold = -1;
				actor.HitsMOs = false;
				actor.AngularVel = actor.AngularVel * 0.5 - actor.Vel.X + actor.FlipFactor;

				self.disintegrationSound:Play(actor.Pos);
				
				if IsAHuman(actor) then
					self.target = ToAHuman(actor);
				elseif IsACrab(actor) then
					self.target = ToACrab(actor);
				else
					self.target = actor;
				end
			end
			--Flag this actor as being hit by a disintegrator particle
			actor:SetNumberValue("ToDisintegrate", actor:GetNumberValue("ToDisintegrate") + 1);
			--Additional glow effects
			local parts = {actor};
			for att in actor.Attachables do
				if att.GetsHitByMOs then
					table.insert(parts, att);
				end
			end
			for _, mo in pairs(parts) do
				local glowDiameter = math.min(math.floor(1 + (mo.Diameter * mo.Scale) * 0.1) * 10, 50);
				local glow = CreateMOPixel("Techion.rte/Disintegration Glow ".. glowDiameter);
				glow.Pos = mo.Pos;
				glow.Vel = mo.Vel;
				MovableMan:AddParticle(glow);
			end
		end
	end
end

function Update(self)
	if self.target and self.target.ID ~= rte.NoMOID then
		self.target.ToSettle = false;
		self.target.Vel = (self.target.Vel * 0.9) - (SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs);
		self.target.AngularVel = self.target.AngularVel * 0.5;

		self.target.GlobalAccScalar = self.target.GlobalAccScalar * (1 - 1/(math.abs(self.target.Mass) + 1));
		self.target:FlashWhite(10);

		local flag = self.target:GetNumberValue("ToDisintegrate");
		local woundCount = self.target:GetWoundCount(false, false, false);
		if woundCount > 0 then
			flag = flag + 1;
			self.target:SetNumberValue("ToDisintegrate", flag);
			self.target:RemoveWounds(woundCount, false, false, false);
		end

		local parts = {self.target};

		--Offset several Attachables in a floating fashion
		for mo in self.target.Attachables do
			table.insert(parts, mo);
			local inverseScale = 1 - self.setScale;

			for att in mo.Attachables do
				table.insert(parts, att);
				att.ParentOffset = Vector(att.ParentOffset.X + RangeRand(-0.6, 0.6), att.ParentOffset.Y + RangeRand(-0.6, 0.6)):SetMagnitude(att.ParentOffset.Magnitude + RangeRand(0, 0.6) * inverseScale);
				att.Pos = att.Pos - Vector(0, 100/att.Radius * inverseScale);

				for attAtt in att.Attachables do
					attAtt.Scale = math.min(self.setScale, att.Scale);
					attAtt.ParentOffset = Vector(attAtt.ParentOffset.X + RangeRand(-0.9, 0.9), attAtt.ParentOffset.Y + RangeRand(-0.9, 0.9)):SetMagnitude(attAtt.ParentOffset.Magnitude + RangeRand(0, 0.9) * inverseScale);
					attAtt.Pos = attAtt.Pos - Vector(0, 100/attAtt.Radius * inverseScale);
				end
			end

			mo.ParentOffset = Vector(mo.ParentOffset.X + RangeRand(-0.3, 0.3), mo.ParentOffset.Y + RangeRand(-0.3, 0.3)):SetMagnitude(mo.ParentOffset.Magnitude + RangeRand(0, 0.3) * inverseScale);
			mo.Pos = mo.Pos - Vector(0, 100/mo.Radius * inverseScale);
			if mo.Radius == mo.IndividualRadius then
				mo.RotAngle = mo.RotAngle + (mo.FlipFactor * inverseScale) * 0.5;
			end

			--Induce randomized deletion
			if (mo.Radius * self.setScale) < math.random(3) then
				mo.ToDelete = true;
			end
		end
		--Shrink all found parts and add particle effects
		for _, mo in pairs(parts) do
			mo.Scale = math.min(self.setScale, mo.Scale);
			local radius = math.sqrt(mo.Radius * mo.Scale) * math.sqrt(flag);
			if mo:GetWoundCount(false, false, false) > 1 then
				mo.ToDelete = true;
				radius = radius * 2;
			end

			for i = 1, radius do
				if math.random(radius) > i then

					local piece = CreateMOSParticle("Techion.rte/White Goo Particle");
					if math.random() < 0.3 then
						piece = CreateMOPixel("Techion.rte/Nanogoo " .. math.random(6));
					end
					local offset = Vector(mo.Radius * mo.Scale * RangeRand(0, 0.5), 0):RadRotate(6.28 * math.random());
					piece.Pos = mo.Pos + offset;
					piece.Vel = mo.Vel + offset:SetMagnitude(RangeRand(radius, radius * 2)/math.sqrt(1 + offset.Magnitude));
					piece.Lifetime = (piece.Lifetime * math.sqrt(flag)) * RangeRand(0.1, 1.0);
					piece.AirResistance = piece.AirResistance * RangeRand(0.1, 1.0);
					piece.GlobalAccScalar = piece.GlobalAccScalar * RangeRand(0.1, 1.0);
					MovableMan:AddParticle(piece);
				end
			end
		end
		
		self.setScale = self.setScale - (self.effectSpeed * flag);
		if self.setScale < self.minScale then

			self.target.ToDelete = true;
		end
	else
		self.ToDelete = true;
	end
end