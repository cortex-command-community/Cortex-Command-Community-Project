function Explode(self)
	--TODO: Add wounds through Lua like the other disintegrator weapons
	local particleCount = 13;
	for i = 1, particleCount do
		local blastpar = i < particleCount * 0.5 and CreateMOPixel("Nucleo Damage Particle") or CreateMOPixel("Nucleo Air Blast");

		local randomDir = Vector(40, 0):RadRotate(math.random() * math.pi * 2);
		blastpar.Pos = self.Pos - Vector(randomDir.X, randomDir.Y);
		blastpar.Vel = Vector(randomDir.X, randomDir.Y);
		blastpar.Team = self.Team;
		blastpar.IgnoresTeamHits = true;
		MovableMan:AddParticle(blastpar);
	end

	if self.target then
		local parent = MovableMan:GetMOFromID(self.target.RootID);
		if IsActor(parent) then
			local melter = CreateMOPixel("Disintegrator");
			melter.Pos = self.Pos;
			melter.Team = self.Team;
			melter.Sharpness = ToActor(parent).ID;
			melter.PinStrength = self.disintegrationStrength * math.sqrt(math.max(1, (self.connectableParticles and #self.connectableParticles or 1)));
			MovableMan:AddMO(melter);
		end
	end

	local blastpar = CreateMOSRotating("Nucleo Explosion");
	blastpar.Pos = self.Pos;
	blastpar:GibThis();
	MovableMan:AddParticle(blastpar);

	self.ToDelete = true;
end

function OnMessage(self, message, context)
	if message == "Nucleo_ConnectableParticles" then
		self.connectableParticles = {};
		for k, particle in pairs(context) do
			if MovableMan:FindObjectByUniqueID(particle) then
				self.connectableParticles[k] = MovableMan:FindObjectByUniqueID(particle);
			end
		end
		self.detDelay = 4000/math.sqrt(math.max(1, #self.connectableParticles));
		self.Vel = Vector(self.Vel.X, self.Vel.Y):DegRotate(#self.connectableParticles * RangeRand(-1, 1));
		for k, particle in pairs(self.connectableParticles) do
			particle:SendMessage("Nucleo_NewConnectableParticle", self.UniqueID);
		end
	elseif message == "Nucleo_NewConnectableParticle" then
		table.insert(self.connectableParticles, MovableMan:FindObjectByUniqueID(context));
	elseif message == "Nucleo_Explode" then
		Explode(self);
	end
end

function Create(self)
	self.detTimer = Timer();
	self.detDelay = 1000;
	self.boom = false;

	self.speed = 15;
	self.acceleration = 0.1;
	self.disintegrationStrength = 300;

	self.linkRange = 100 + (FrameMan.PlayerScreenWidth + FrameMan.PlayerScreenHeight) * 0.25;
	self.linkPullRatio = 0.005;
	--Table of colors being used in primitive effects
	self.colors = {5, 186, 198, 196};
end

function ThreadedUpdate(self)
	if self.detTimer:IsPastSimMS(self.detDelay) then
		Explode(self);
	else
		local moid = SceneMan:CastMORay(self.Pos, self.Vel * rte.PxTravelledPerFrame, rte.NoMOID, self.Team, rte.airID, true, 1);
		if moid ~= rte.NoMOID then
			local hitPos = Vector();
			SceneMan:CastFindMORay(self.Pos, self.Vel * rte.PxTravelledPerFrame, moid, hitPos, rte.airID, true, 1);
			self.Pos = hitPos;
			self.boom = true;
			self.hitTarget = true;
			self.target = MovableMan:GetMOFromID(moid);
		end
	end

	PrimitiveMan:DrawCirclePrimitive(self.Pos, self.Radius, self.colors[math.random(#self.colors)]);

	if self.connectableParticles then
		for k, particle in pairs(self.connectableParticles) do
			if MovableMan:ValidMO(particle) then
				local dist = SceneMan:ShortestDistance(self.Pos, particle.Pos, SceneMan.SceneWrapsX);
				if dist:MagnitudeIsLessThan(self.linkRange) then

					PrimitiveMan:DrawLinePrimitive(self.Pos + Vector(math.random(-1, 1), math.random(-1, 1)), self.Pos + dist + Vector(math.random(-1, 1), math.random(-1, 1)), self.colors[math.random(#self.colors)]);
					--The projectiles are drawn towards each other
					self.Vel = self.Vel + dist:RadRotate(RangeRand(-0.1, 0.1)) * rte.PxTravelledPerFrame * self.linkPullRatio;

					local moid = SceneMan:CastMORay(self.Pos, dist, rte.NoMOID, self.Team, rte.airID, true, 1);
					if moid ~= rte.NoMOID then
						local hitPos = Vector();
						SceneMan:CastFindMORay(self.Pos, dist, moid, hitPos, rte.airID, true, 1);
						self.Pos = hitPos;
						self.boom = true;
						self.hitTarget = true;
						self.target = MovableMan:GetMOFromID(moid);
					end
				end
			end
		end
	end

	self.Vel = Vector(self.Vel.X, self.Vel.Y):SetMagnitude(math.min(self.Vel.Magnitude + self.acceleration, self.speed));
	self.AngularVel = self.AngularVel * 0.9;
	self.RotAngle = 0;

	if self.hitTarget then
		self:RequestSyncedUpdate();
	end
end

function SyncedUpdate(self)
	if self.connectableParticles then
		for k, particle in pairs(self.connectableParticles) do
			if MovableMan:ValidMO(particle) then
				particle.Pos = self.Pos + Vector(math.random() * 5, 0):RadRotate(math.random() * math.pi * 2);
				particle:SendMessage("Nucleo_Explode");
			end
		end
	end

	Explode(self)
end