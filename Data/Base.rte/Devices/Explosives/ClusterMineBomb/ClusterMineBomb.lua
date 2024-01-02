function Create(self)
	self.alliedTeam = -1;
	self.lastAngle = self.RotAngle;

	self.deployRange = 100;

	self.mineCount = 4;
end

function Update(self)
	self.lastAngle = self.RotAngle;

	if not self:GetParent() then
		if self.alliedTeam == -1 then
			self.curdist = 500;
			for actor in MovableMan.Actors do
				local dist = SceneMan:ShortestDistance(self.Pos, actor.Pos, SceneMan.SceneWrapsX).Magnitude - actor.Radius;
				if dist < self.curdist then
					self.curdist = dist;
					self.alliedTeam = ToActor(actor).Team;
				end
			end
		end

		local rayHitPos = Vector();
		local terrainRaycast = SceneMan:CastStrengthRay(self.Pos, Vector(self.Vel.X, self.Vel.Y):SetMagnitude(self.Radius + self.deployRange + self.Vel.Magnitude), 0, rayHitPos, 1, rte.airID, SceneMan.SceneWrapsX);

		if terrainRaycast == true then
			local spread = 3;
			for i = 1, self.mineCount do
				local mine = CreateMOSRotating("Anti Personnel Mine Active");
				mine.Pos = self.Pos;
				mine.Vel = self.Vel * 0.5 + Vector(self.Vel.X, self.Vel.Y):RadRotate(spread * 0.6 - (spread * (i/self.mineCount)) + RangeRand(-0.1, 0.1)):SetMagnitude(20);
				mine.Sharpness = self.alliedTeam;
				MovableMan:AddParticle(mine);
				self:GibThis();
			end
		end
	end
end

function Destroy(self)
	if MovableMan:ValidMO(self) then
		local gibA = CreateMOSRotating("Cluster Mine Bomb Gib A");
		gibA.Pos = self.Pos;
		gibA.Vel = self.Vel;
		gibA.RotAngle = self.lastAngle;
		MovableMan:AddParticle(gibA);

		local gibB = CreateMOSRotating("Cluster Mine Bomb Gib B");
		gibB.Pos = self.Pos + Vector(1, -3):RadRotate(self.lastAngle);
		gibB.Vel = self.Vel + Vector(0, 20):RadRotate(self.lastAngle);
		gibB.AngularVel = 10;
		gibB.RotAngle = self.lastAngle;
		MovableMan:AddParticle(gibB);

		local gibC = CreateMOSRotating("Cluster Mine Bomb Gib C");
		gibC.Pos = self.Pos + Vector(1, 3):RadRotate(self.lastAngle);
		gibC.Vel = self.Vel + Vector(0, -20):RadRotate(self.lastAngle);
		gibC.AngularVel = -10;
		gibC.RotAngle = self.lastAngle;
		MovableMan:AddParticle(gibC);
	end
end