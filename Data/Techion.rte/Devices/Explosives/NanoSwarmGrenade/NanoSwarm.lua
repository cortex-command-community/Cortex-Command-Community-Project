--Moves an object toward a target in a wasp-like fashion.
--object:	The MovableObject to move
--target:	The Vector position to move toward
--speed:	The speed to move at.
function SwarmTo(object, target, speed)
	local baseVector = SceneMan:ShortestDistance(object.Pos, target, true);
	local dirVector = baseVector/baseVector.Largest;
	local dist = baseVector.Magnitude;
	local modifier = dist * 0.2;
	if modifier < 1 then
		modifier = 1;
	end

	object.Vel = object.Vel + dirVector * speed * modifier;
end

function Create(self)
	--The initial number of wasps.
	self.waspNum = 25;

	--The chance of stinging.
	self.stingChance = 0.015;

	--The change of dying while stinging.
	self.stingDieChance = 0.1;

	--The chance of flickering.
	self.flickerChance = 0.1;

	--Speed of the sting particle.
	self.stingSpeed = 25;

	--How long it takes for one wasp to die off.
	self.dieTime = 500;

	--How high to go while idling, maximum.
	self.maxIdleAlt = 100;

	--How low to go while idling, minimum.
	self.minIdleAlt = 25;

	--The radius of the swarm.
	self.swarmRad = 15;

	--The maximum speed for one wasp.
	self.maxSpeed = 5;

	--The modifier for maximum speed when attacking.
	self.attackMaxMod = 5;

	--The basic acceleration for one wasp.
	self.baseAcc = 0.75;

	--The modifier for acceleration when attacking.
	self.attackAccMod = 2;

	--The acceleration speed of the base swarm.
	self.swarmSpeed = 1;

	--The maximum speed of the base swarm.
	self.maxBaseSpeed = 15;

	--The air reistance on the base swarm.
	self.airResistance = 1.1;

	--The maximum distance a wasp can be from the swarm.
	self.maxDist = 75;

	--The maximum distance to target at.
	self.targetDist = 250;

	--The maximum distance to attack at.
	self.attackDist = 75;

	--The maximum strength the swarm can push through.
	self.maxMoveStrength = 1;

	--The list of wasps in this swarm.
	self.roster = {};

	--The list of offsets for each wasp.
	self.offsets = {};

	--The target to attack.
	self.target = nil;

	--Timer for wasp death.
	self.dieTimer = Timer();

	--Fill the list.
	for i = 1, self.waspNum do
		local wasp = CreateMOPixel("Techion.rte/Nanowasp " .. math.random(3));
		wasp.Vel = Vector(math.random(-10, 10), math.random(-10, 10));
		self.offsets[i] = Vector(math.random(-self.swarmRad, self.swarmRad), math.random(-self.swarmRad, self.swarmRad));
		wasp.Pos = self.Pos + self.offsets[i];
		MovableMan:AddParticle(wasp);
		self.roster[i] = wasp;
	end
end

function Update(self)
	--Move the swarm.
	local moving = false;
	local attacking = false;

	if not MovableMan:IsActor(self.target) or self.target.Team == self.Team then
		--Find a target.
		local shortestDist;
		for actor in MovableMan.Actors do
			if actor.Team ~= self.Team then
				local dist = SceneMan:ShortestDistance(self.Pos, actor.Pos, true).Magnitude;
				if dist < self.targetDist and (not shortestDist or dist < shortestDist) then
					self.target = actor;
					shortestDist = dist;
				end
			end
		end

		if not MovableMan:IsActor(self.target) then
			--If all else fails, move randomly.
			self.Vel = self.Vel + Vector(math.random() * 1 - 0.5, math.random() * 1 - 0.5);

			--Keep the wasps at the desired altitude.
			local alt = self:GetAltitude(0, 10);
			if alt > self.maxIdleAlt then
				self.Vel = Vector(self.Vel.X, math.abs(self.Vel.Y) * 0.5);
			elseif alt < self.minIdleAlt then
				self.Vel = Vector(self.Vel.X, -math.abs(self.Vel.Y) * 0.5);
			end

			moving = true;
		else
			--Attack if this is stuck in something.
			if SceneMan:CastStrengthRay(self.Pos, Vector(0, -5), 0, Vector(), 0, rte.airID, true) then
				attacking = true;
			end
		end
	end

	if MovableMan:IsActor(self.target) then
		--Go after the target.
		if not SceneMan:CastStrengthRay(self.Pos, SceneMan:ShortestDistance(self.Pos, self.target.Pos, true), self.maxMoveStrength, Vector(), 5, rte.airID, true) then
			local dirVec = SceneMan:ShortestDistance(self.Pos, self.target.Pos, true);
			local movement = (dirVec/dirVec.Largest) * self.maxBaseSpeed;

			self.Vel = self.Vel + movement;

			if movement.Largest ~= 0 then
				moving = true;
			end

			--Attack, if necessary.
			if self.target.PresetName ~= "Nanowasp Swarm" and self.target.Team ~= self.Team then
				if SceneMan:ShortestDistance(self.Pos, self.target.Pos, true):MagnitudeIsLessThan(self.attackDist) then
					attacking = true;
				end
			end
		else
			target = nil;
		end
	else
		target = nil;
	end

	if not moving then
		self.Vel = self.Vel/self.airResistance;
	end

	if self.Vel.Largest > self.maxBaseSpeed then
		self.Vel = (self.Vel/self.Vel.Largest) * self.maxBaseSpeed;
	end

	--Check if the swarm is about to run into a wall, and if it is, stop it.
	if SceneMan:CastStrengthRay(self.Pos, self.Vel, self.maxMoveStrength, Vector(), 0, rte.airID, true) then
		self.Vel = Vector();
	end

	--Attack.
	local attackMax = 1;
	local attackAcc = 1;

	if attacking then
		attackMax = self.attackMaxMod;
		attackAcc = self.attackAccMod;
	end

	--Make all the wasps in this swarm's roster follow it.
	for i = 1, #self.roster do
		if MovableMan:IsParticle(self.roster[i]) then
			local wasp = self.roster[i];

			--Keep the wasp alive.
			wasp.ToDelete = false;
			wasp.ToSettle = false;
			wasp:NotResting();
			wasp.Age = 0;

			--Make the wasp follow the swarm.
			local target = self.Pos + self.offsets[i];
			SwarmTo(wasp,target,math.random() * self.baseAcc * attackAcc);

			--Keep the wasp from going too fast.
			local speedMod = SceneMan:ShortestDistance(wasp.Pos, target, true).Magnitude * 0.2;
			if speedMod < 1 then
				speedMod = 1;
			end

			--Counteract gravity.
			wasp.Vel = Vector(wasp.Vel.X, wasp.Vel.Y - SceneMan.Scene.GlobalAcc.Y * TimerMan.DeltaTimeSecs);

			if wasp.Vel.Largest > self.maxSpeed * speedMod * attackMax then
				wasp.Vel = (wasp.Vel/wasp.Vel.Largest) * self.maxSpeed * speedMod * attackMax;
			end

			--Keep the wasp within decent bounds of the swarm.
			local distVec = SceneMan:ShortestDistance(target, wasp.Pos, true);

			if math.abs(distVec.Largest) > self.maxDist then
				wasp.Pos = distVec:SetMagnitude(self.maxDist) + target;
			end

			--Flicker.
			if math.random() <= self.flickerChance then
				local flicker = CreateMOPixel("Techion.rte/Nanowasp Flicker");
				flicker.Pos = wasp.Pos;
				MovableMan:AddParticle(flicker);
			end

			--Sting.
			if attacking == true and math.random() <= self.stingChance then
				local sting = CreateMOPixel("Techion.rte/Nanowasp Sting");
				sting.Pos = self.Pos + Vector(math.random(-self.swarmRad, self.swarmRad), math.random(-self.swarmRad, self.swarmRad));
				sting.Vel = (wasp.Vel/wasp.Vel.Largest) * self.stingSpeed;
				MovableMan:AddParticle(sting);

				if math.random() < self.stingDieChance then
					self.waspNum = self.waspNum - 1;
					table.remove(self.roster, #self.roster);
				end
			end
		else
			if #self.roster < self.waspNum then
				--Replace the wasp.
				local wasp = CreateMOPixel("Techion.rte/Nanowasp " .. math.random(3));
				wasp.Pos = self.Pos + self.offsets[i];
				wasp.Vel = Vector(math.random(-10, 10), math.random(-10, 10));
				MovableMan:AddParticle(wasp);
				self.roster[i] = wasp;
			else
				table.remove(self.roster, i);
			end
		end
	end

	--Die off gradually.
	if self.dieTimer:IsPastSimMS(self.dieTime) and self.waspNum > 0 then
		self.waspNum = self.waspNum - 1;
		table.remove(self.roster, #self.roster);
		self.dieTimer:Reset();
	end

	if self.waspNum > 0 then
		self.ToDelete = false;
		self.ToSettle = false;
		self:NotResting();
		self.Age = 0;
	else
		self.ToDelete = true;
	end
end

function Destroy(self)
	--Remove all wasps.
	for i = 1, #self.roster do
		if MovableMan:IsParticle(self.roster[i]) then
			self.roster[i].ToDelete = true;
		end
	end
end