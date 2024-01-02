function Create(self)
	self.shortFlame = CreatePEmitter("Flame Hurt Short Float", "Base.rte");
	
	self.flameTimer = Timer();
	self.flameSpawnDelay = math.random(1000);
	--Define Throttle for non-emitter particles
	if self.Throttle == nil then
		self.Throttle = 0;
	end
	self.flameToAdd = nil;
end

function ThreadedUpdate(self)
	self.ageRatio = 1 - self.Age/self.Lifetime;
	self:NotResting();
	--TODO: Use Throttle to combine multiple flames into one
	self.Throttle = self.Throttle - TimerMan.DeltaTimeMS/self.Lifetime;
	--Spawn another, shorter flame occasionally
	if self.flameTimer:IsPastSimMS(self.flameSpawnDelay) then
		self.flameTimer:Reset();
		self.flameSpawnDelay = 1000 + self.Age * 0.25;

		local particle = self.shortFlame:Clone();
		particle.Lifetime = 100 + math.max(particle.Lifetime * self.ageRatio);
		particle.Vel = self.Vel + Vector(0, -2) + Vector(math.random(), 0):RadRotate(RangeRand(-math.pi, math.pi));
		particle.Pos = self.Pos + Vector(0, -1);
		self.flameToAdd = particle;
		self:RequestSyncedUpdate();
	end
end

function SyncedUpdate(self)
	if self.flameToAdd then
		MovableMan:AddParticle(self.flameToAdd);
		self.flameToAdd = nil;
	end
end

function OnCollideWithTerrain(self, terrainID)
	if (self.Vel + self.PrevVel):MagnitudeIsLessThan(1) then
		local checkPos = self.Pos + Vector(math.random(-1, 1), math.random(-1, 1));
		if SceneMan:GetTerrMatter(checkPos.X, checkPos.Y) == rte.grassID then
			local px = SceneMan:DislodgePixel(checkPos.X, checkPos.Y);
			if px and (px.Material.PresetName == "Grass" or px.Material.PresetName == "Vegetation") then
				px.ToDelete = true;
				px = CreateMOPixel("Ash Particle " .. math.random(3), "Base.rte");
				px.Pos = checkPos;
				px.Vel = Vector(0, -2):RadRotate(math.pi * RangeRand(-0.5, 0.5));
				MovableMan:AddParticle(px);
			end
		end
	end
end