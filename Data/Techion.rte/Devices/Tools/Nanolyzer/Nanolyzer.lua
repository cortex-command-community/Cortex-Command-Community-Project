function Create(self)
	--The number of goo particles available to pick from
	self.gooCount = 6;

	--The range of the tool
	self.range = 30;

	--The arc radius of the tool's beam
	self.beamRadius = math.rad(self.ParticleSpreadRange) + 0.1;

	--The radial resolution of the beam
	self.resolution = 0.05;

	--The chance of deconstructing a single particle
	self.deconstructChance = 0.1;

	--The base damage output when used on MOs
	self.damageOutput = 10;

	--How many pixels to skip when detecting MOs, for optimization reasons
	self.skipPixels = 1;

	--Terrain remover particle
	self.remover = CreateMOSRotating("Techion.rte/Pixel Remover");
	
	--Sounds
	self.dissipateSound = CreateSoundContainer("Dissipate Sound", "Techion.rte");
	self.disintegrationSound = CreateSoundContainer("Disintegration Sound", "Techion.rte");
end

function OnFire(self)
	self.MuzzleOffset = Vector(5, (self.MuzzleOffset.Y + 1) % 3 - 1);
	
	local aimAngle = self.HFlipped and self.RotAngle + math.pi or self.RotAngle;
	local aimVec = Vector(self.range, 0):RadRotate(aimAngle);

	--Find MOs to disintegrate
	local moCheck = SceneMan:CastMORay(self.MuzzlePos, aimVec * RangeRand(0.5, 1.0), self.RootID, self.Team, rte.airID, true, 2);
	if moCheck ~= rte.NoMOID then

		local initMO = MovableMan:GetMOFromID(moCheck);
		if initMO then

			local targetMO = MovableMan:GetMOFromID(ToMOSRotating(initMO).RootID);
			local dustTarget;

			if targetMO then
				local resistance = math.sqrt(targetMO.Diameter + math.abs(targetMO.Mass) + targetMO.Material.StructuralIntegrity + 1);
				local gooCount = 0;
				if IsActor(targetMO) then
					local actor = ToActor(targetMO);
					local damage = self.damageOutput/resistance;
					if actor.Status > Actor.UNSTABLE then
						if IsADoor(actor) then
							dustTarget = ToADoor(actor).Door;
							ToADoor(actor):RemoveAttachable(ToADoor(actor).Door, true, false);
						else
							dustTarget = actor;
						end
					else
						actor.Health = actor.Health - damage;
						local healthRatio = actor.Health/actor.MaxHealth;
						if math.random() > healthRatio then
							actor:FlashWhite(20/(1 + healthRatio));
							gooCount = math.sqrt(actor.Diameter)/(1 + healthRatio);
						end
					end
				elseif IsMOSRotating(targetMO) then
					targetMO = ToMOSRotating(targetMO);
					local wound = CreateAEmitter("Dent Metal No Spark", "Base.rte");
					wound.InheritedRotAngleOffset = RangeRand(-math.pi, math.pi);
					targetMO:AddWound(wound, Vector(targetMO:GetSpriteWidth() * RangeRand(-0.3, 0.3), targetMO:GetSpriteHeight() * RangeRand(-0.3, 0.3)), false);
					
					local woundRatio = 1 - targetMO.WoundCount/(targetMO.GibWoundLimit > 0 and targetMO.GibWoundLimit or targetMO.Radius);
					gooCount = math.sqrt(targetMO.Diameter)/(1 + woundRatio);
					if woundRatio <= 0 then
						dustTarget = ToMOSRotating(targetMO);
					end
				end
				if gooCount > 0 then
					if self.dissipateSound:IsBeingPlayed() then
						self.dissipateSound.Pitch = RangeRand(RangeRand(0.7, 1.3));
						self.dissipateSound:Play(targetMO.Pos);
					end
					for i = 1, gooCount do
						local piece = CreateMOPixel("Techion.rte/Nanogoo " .. math.random(self.gooCount));
						piece.Pos = targetMO.Pos;
						piece.Vel = targetMO.Vel * 0.5 + Vector(gooCount, 0):RadRotate(6.28 * math.random()) + Vector(0, -1);
						piece.Lifetime = math.random(300, 900);
						piece.GlobalAccScalar = RangeRand(0.2, 0.4);
						piece.AirResistance = RangeRand(0.1, 0.2);
						MovableMan:AddParticle(piece);
					end
				end
			end
			if dustTarget then
				local parts = {dustTarget};
				--Measure the corners of a box that the Actor is supposedly inside of
				local topLeft = Vector(-dustTarget.IndividualRadius, -dustTarget.IndividualRadius);
				local bottomRight = Vector(dustTarget.IndividualRadius, dustTarget.IndividualRadius);
				for att in dustTarget.Attachables do
					if IsAttachable(att) then
						local dist = SceneMan:ShortestDistance(dustTarget.Pos, att.Pos, SceneMan.SceneWrapsX);
						local reach = dist:SetMagnitude(dist.Magnitude + att.Radius + 5);
						if reach.X < topLeft.X then
							topLeft.X = reach.X;
						elseif reach.X > bottomRight.X then
							bottomRight.X = reach.X;
						end
						if reach.Y < topLeft.Y then
							topLeft.Y = reach.Y;
						elseif reach.Y > bottomRight.Y then
							bottomRight.Y = reach.Y;
						end
						table.insert(parts, att);
					end
				end
				--Spawn a pixel particle for each pixel of the Actor
				local skip = 1 + self.skipPixels;
				for x = 1, (bottomRight.X - topLeft.X)/skip do
					for y = 1, (bottomRight.Y - topLeft.Y) do
						local checkPos = (dustTarget.Pos + topLeft) + Vector(x * skip, y);
						local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
						if terrCheck == rte.airID or terrCheck == rte.grassID then
							local id = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
							if id ~= rte.NoMOID and MovableMan:GetMOFromID(id).RootID == dustTarget.ID then
								for px = 1, skip do
									local piece = CreateMOPixel("Techion.rte/Nanogoo " .. math.random(1, self.gooCount));
									piece.Pos = checkPos + Vector(px - 1, 0);
									piece.Vel = dustTarget.Vel * 0.3 + Vector(0, -1) + SceneMan:ShortestDistance(dustTarget.Pos, piece.Pos, SceneMan.SceneWrapsX):SetMagnitude(math.random());
									piece.Lifetime = math.random(500, 1500);
									piece.GlobalAccScalar = RangeRand(0.2, 0.4);
									piece.AirResistance = RangeRand(0.1, 0.2);
									MovableMan:AddParticle(piece);
								end
							end
						end
					end
				end
				for _, mo in pairs(parts) do
					local glowDiameter = math.min(math.floor(1 + (mo.Diameter * mo.Scale) * 0.1) * 10, 50);
					local glow = CreateMOPixel("Techion.rte/Disintegration Glow ".. glowDiameter);
					glow.Pos = mo.Pos;
					glow.Vel = mo.Vel;
					MovableMan:AddParticle(glow);
				end
				self.disintegrationSound:Play(dustTarget.Pos);
				dustTarget.ToDelete = true;
			end
		end
	else
		--Find terrain to disintegrate
		for i = -self.beamRadius * 0.5, self.beamRadius * 0.5, self.resolution do
			if math.random() < self.deconstructChance then
				local hitPos = Vector(self.MuzzlePos.X, self.MuzzlePos.Y);
				if SceneMan:CastStrengthRay(self.MuzzlePos, Vector(aimVec.X, aimVec.Y):RadRotate(i), 1, hitPos, 0, 166, true) then
					self.remover.Pos = hitPos;
					self.remover:EraseFromTerrain();

					local piece = CreateMOPixel("Techion.rte/Nanogoo " .. math.random(1, self.gooCount));
					piece.Pos = hitPos;
					MovableMan:AddParticle(piece);
					piece.ToSettle = true;

					local glow = CreateMOPixel("Techion.rte/Pixel Creation Glow");
					glow.Pos = hitPos;
					MovableMan:AddParticle(glow);

					if self.dissipateSound:IsBeingPlayed() then
						self.dissipateSound.Pitch = RangeRand(RangeRand(0.7, 1.3));
						self.dissipateSound:Play(hitPos);
					end
				end
			end
		end
	end
end