--------------------------------------- Instructions ---------------------------------------

------- Require this in your script like so: 

-- self.particleUtility = require("Scripts/Utility/ParticleUtility");

-- This is a bunch of particle-related utilities. See each function for instructions.

--------------------------------------- Misc. Information ---------------------------------------

--

local ParticleUtility = {}

function ParticleUtility:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

-- Create Directional Smoke Effect

-- Self-explanatory function to quickly create a directional blast of smoke and explosion FX.
-- The minimum arguments are position, angle, and FX power.
-- Argument description in order:

------ positionOrFullTable:
-- The position to spawn the effect at, OR a table with every variable contained within as string keys.
-- It must have every variable in it to be valid, and must contain the following:
	-- Position;
	-- RadAngle;
	-- Power;
	-- Source;
	-- Spread;
	-- SmokeMult;
	-- ExploMult;
	-- WidthSpread;
	-- VelocityMult;
	-- LingerMult;
	-- AirResistanceMult;
	-- GravMult;
------ angOrVector:
-- Either a radian angle for the effect, or a Vector() to grab the angle from. You can pass in ShortestDistance results here for example.
----- power:
-- Effect power. Affects type of FX spawned, amount of FX spawned, and their velocity. 15 is a small muzzle-smoke-like puff, 100 is a pretty big explosion.
----- source:
-- An MO that can act as the "source" for this effect. All this does is add the source's velocity to the smoke.
----- spread:
-- Degree spread for the particles spawned.
----- smokeMult:
-- Power multiplier for the smokey FX particles.
----- exploMult:
-- Power multiplier for the firey, explody FX particles.
----- widthSpread:
-- Pixel integer. With this at 0, every effect spawns at the exact same point. With it above 0, the effects are spread out to a wider area, for example vertically for a right-facing effect.
----- velocityMult:
-- Velocity multiplier. If your power setting doesn't result in the velocity you want you can change it with this.
----- lingerMult:
-- Lifetime multiplier for effects.
----- airResistanceMult:
-- Air resistance multiplier - this makes effects both slow down faster and to a slower terminal velocity.
----- gravMult:
-- Gravity multiplier. Please note that some effects have negative gravity to begin with, and this is just a flat multiplier on top of that.

function ParticleUtility:CreateDirectionalSmokeEffect(positionOrFullTable, angOrVector, power, source, spread, smokeMult, exploMult, widthSpread, velocityMult, lingerMult, airResistanceMult, gravMult)
	
	local position;
		
	if type(positionOrFullTable) == "table" then
		position = positionOrFullTable.Position;
		source = positionOrFullTable.Source;
		angOrVector = positionOrFullTable.RadAngle;
		power = positionOrFullTable.Power;
		spread = positionOrFullTable.Spread;
		smokeMult = positionOrFullTable.SmokeMult;
		exploMult = positionOrFullTable.ExploMult;
		widthSpread = positionOrFullTable.WidthSpread;
		velocityMult = positionOrFullTable.VelocityMult;
		lingerMult = positionOrFullTable.LingerMult;
		airResistanceMult = positionOrFullTable.AirResistanceMult;
		gravMult = positionOrFullTable.GravMult;
	end
		
	
	if position and angOrVector and power then
		if type(angOrVector) == "number" then
			angOrVector = math.deg(angOrVector);
		elseif angOrVector.Magnitude then
			angOrVector = angOrVector.AbsDegAngle;
		end
		if not spread then
			spread = 15;
		end
		if not widthspread then
			widthspread = 1;
		end
		if not smokeMult then
			smokeMult = 1;
		end
		if not exploMult then
			exploMult = 0;
		end
		if not velocityMult then
			velocityMult = 1;
		end
		if not lingerMult then
			lingerMult = 1;
		end
		if not gravMult then
			gravMult = 1;
		end
		
		local angle = angOrVector;
		local smokePower = power * smokeMult;
		local count = smokePower / 5;
		local velocity = Vector(smokePower / 1.5, 0):DegRotate(angle) * velocityMult;
		if source then
			velocity = velocity + source.Vel;
		end
		local linger = (smokePower / 100) * lingerMult
		
		
		local smokeCount = math.max(1, math.floor((count / (math.random(2,4))) * smokeMult));
		for _ = 0, math.floor((count / (math.random(2,4))) * smokeMult) do
		
			-- Tiny smoke, high spread
			local tinySmokeCount = math.random(math.max(0, math.floor(smokePower/12.5)), math.max(0, math.ceil(smokePower/6.25)));
			for i = 1, tinySmokeCount do	
				local randSpread = (math.random(-spread*500, spread*500)/1000)
				local randVelocityMult = RangeRand(0.5, 1.5);
				
				local particle = CreateMOSParticle("Tiny Smoke Ball 1", "Base.rte");
				particle.Pos = position + Vector(0, RangeRand(-widthspread/2, widthspread/2)):DegRotate(angle);
				particle.Vel = Vector(velocity.X, velocity.Y):DegRotate(randSpread) * randVelocityMult;
				particle.Lifetime = particle.Lifetime * RangeRand(0.75, 1.75) * lingerMult;
				particle.AirThreshold = (particle.AirThreshold / airResistanceMult) * (lingerMult/airResistanceMult);
				particle.AirResistance = math.min(62.5, particle.AirResistance * airResistanceMult);
				particle.GlobalAccScalar = particle.GlobalAccScalar * gravMult;
				particle.Team = source.Team;
				--particle.HitsMOs = false;
				MovableMan:AddParticle(particle);
			end

			-- Small smoke, medium spread
			local smallSmokeCount = math.random(math.max(0, math.floor(smokePower/25)), math.max(0, math.floor(smokePower/12.5)));
			for i = 1, smallSmokeCount do	
				local countFactor = i/smallSmokeCount;
				local randSpread = (math.random(-spread*500, spread*500)/1000) * countFactor;
				local easeFactor = math.abs(randSpread/(spread/2))
				factor = 1 - math.cos((easeFactor * math.pi) / 2);
				randSpread = randSpread * easeFactor;
				local randVelocityMult = RangeRand(0.5, 1.5);
				
				local particle = CreateMOSParticle("Small Smoke Ball 1", "Base.rte");
				particle.Pos = position + Vector(0, RangeRand(-widthspread/2, widthspread/2)):DegRotate(angle);
				particle.Vel = Vector(velocity.X, velocity.Y):DegRotate(randSpread) * randVelocityMult;
				particle.Lifetime = particle.Lifetime * RangeRand(0.75, 1.25) * lingerMult;
				particle.AirThreshold = (particle.AirThreshold / airResistanceMult) * (lingerMult/airResistanceMult);
				particle.AirResistance = math.min(62.5, particle.AirResistance * airResistanceMult);
				particle.GlobalAccScalar = particle.GlobalAccScalar * gravMult;
				particle.Team = source.Team;
				--particle.HitsMOs = false;
				MovableMan:AddParticle(particle);
			end
		end
		
		local exploPower = power * exploMult;
		count = exploPower / 5;
		velocity = Vector(exploPower / 1.5, 0):DegRotate(angle) * velocityMult;
		if source then
			velocity = velocity + source.Vel;
		end
		linger = (exploPower / 100) * lingerMult
		
		local exploCount = math.max(1, math.floor((count / (math.random(2,4))) * exploMult));
		for _ = 0, exploCount do
			
			-- Tiny explo particle
			local tinyExploCount = math.random(math.max(0, math.floor(exploPower/25)), math.max(0, math.ceil(exploPower/12.5)));
			for i = 1, tinyExploCount do	
				local countFactor = i/tinyExploCount;
				local randSpread = (math.random(-spread*500, spread*500)/1000) * countFactor;
				local easeFactor = math.abs(randSpread/(spread/2))
				factor = 1 - math.cos((easeFactor * math.pi) / 2);
				randSpread = randSpread * easeFactor;
				local randVelocityMult = RangeRand(0.5, 1.5);
				
				local particle = CreateMOSParticle("Flame Smoke 1 Micro", "Base.rte");
				particle.Pos = position + Vector(0, RangeRand(-widthspread/2, widthspread/2)):DegRotate(angle);
				particle.Vel = Vector(velocity.X, velocity.Y):DegRotate(randSpread) * randVelocityMult;
				particle.Lifetime = particle.Lifetime * RangeRand(0.75, 1.25) * lingerMult;
				particle.AirThreshold = (particle.AirThreshold / airResistanceMult) * (lingerMult/airResistanceMult);
				particle.AirResistance = math.min(62.5, particle.AirResistance * airResistanceMult);
				particle.GlobalAccScalar = particle.GlobalAccScalar * gravMult;
				particle.Team = source.Team;
				--particle.HitsMOs = false;
				MovableMan:AddParticle(particle);
			end

			-- Small explo particle
			local smallExploCount = math.random(math.max(0, math.floor(exploPower/50)), math.max(0, math.floor(exploPower/25)));
			for i = 1, smallExploCount do	
				local countFactor = i/smallExploCount;
				local randSpread = (math.random(-spread*500, spread*500)/1000) * countFactor;
				local easeFactor = math.abs(randSpread/(spread/2))
				factor = 1 - math.cos((easeFactor * math.pi) / 2);
				randSpread = randSpread * easeFactor;
				local randVelocityMult = RangeRand(0.5, 1.5);
				
				local particle = CreateMOSParticle("Side Thruster Blast Ball 1", "Base.rte");
				particle.Pos = position + Vector(0, RangeRand(-widthspread/2, widthspread/2)):DegRotate(angle);
				particle.Vel = Vector(velocity.X, velocity.Y):DegRotate(randSpread) * randVelocityMult;
				particle.Lifetime = particle.Lifetime * RangeRand(0.4, 0.8) * (lingerMult);
				particle.AirThreshold = (particle.AirThreshold / airResistanceMult);
				particle.AirResistance = math.min(62.5, particle.AirResistance * airResistanceMult);
				particle.GlobalAccScalar = particle.GlobalAccScalar * gravMult;
				particle.Team = source.Team;
				--particle.HitsMOs = false;
				MovableMan:AddParticle(particle);
			end
			
			-- Large explo particle 1
			local large1ExploCount = math.random(math.max(0, math.floor(exploPower/260)), math.max(0, math.floor(exploPower/130)));
			for i = 1, large1ExploCount do	
				local countFactor = i/large1ExploCount;
				local randSpread = (math.random(-spread*500, spread*500)/1000) * countFactor;
				local easeFactor = math.abs(randSpread/(spread/2))
				factor = 1 - math.cos((easeFactor * math.pi) / 2);
				randSpread = randSpread * easeFactor;
				local randVelocityMult = RangeRand(0.5, 1.5);
				
				local particle = CreateMOSParticle("Explosion Smoke " .. math.random(1, 2), "Base.rte");
				particle.Pos = position + Vector(0, RangeRand(-widthspread/2, widthspread/2)):DegRotate(angle);
				particle.Vel = Vector(velocity.X, velocity.Y):DegRotate(randSpread) * randVelocityMult;
				particle.Lifetime = particle.Lifetime * RangeRand(0.75, 1.25) * lingerMult;
				particle.AirThreshold = (particle.AirThreshold / airResistanceMult) * (lingerMult/airResistanceMult);
				particle.AirResistance = math.min(62.5, particle.AirResistance * airResistanceMult);
				particle.GlobalAccScalar = particle.GlobalAccScalar * gravMult;
				particle.Team = source.Team;
				--particle.HitsMOs = false;
				MovableMan:AddParticle(particle);
			end
			
			-- Large explo particle 2
			local large2ExploCount = math.random(math.max(0, math.floor(exploPower/300)), math.max(0, math.floor(exploPower/200)));
			for i = 1, large2ExploCount do	
				local countFactor = i/large2ExploCount;
				local randSpread = (math.random(-spread*500, spread*500)/1000) * countFactor;
				local easeFactor = math.abs(randSpread/(spread/2))
				factor = 1 - math.cos((easeFactor * math.pi) / 2);
				randSpread = randSpread * easeFactor;
				local randVelocityMult = RangeRand(0.3, 1.1);
				
				local particle = CreateAEmitter("Explosion Trail 1", "Base.rte");
				particle.Pos = position + Vector(0, RangeRand(-widthspread/2, widthspread/2)):DegRotate(angle);
				particle.Vel = Vector(velocity.X, velocity.Y):DegRotate(randSpread) * randVelocityMult;
				particle.Lifetime = particle.Lifetime * RangeRand(0.75, 1.25) * lingerMult;
				particle.AirThreshold = (particle.AirThreshold / airResistanceMult) * (lingerMult/airResistanceMult);
				particle.AirResistance = math.min(62.5, particle.AirResistance * airResistanceMult);
				particle.GlobalAccScalar = particle.GlobalAccScalar * gravMult;
				particle.Team = source.Team;
				--particle.HitsMOs = false;
				MovableMan:AddParticle(particle);
			end
		end
		
	else
		print("ERROR: ParticleUtility tried to create a smoke effect but was not given a position, angle, or power amount!");
		return false;
	end
	
	return true;

end

return ParticleUtility:Create();