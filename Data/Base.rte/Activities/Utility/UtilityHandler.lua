local UtilityHandler = {}

function UtilityHandler:Create()
	local Members = {};

	setmetatable(Members, self);
	self.__index = self;

	return Members;
end

function UtilityHandler:Initialize(verboseLogging)

	self.verboseLogging = verboseLogging;
	

	if self.verboseLogging then
		print("INFO: UtilityHandler initialized!");
	end
	
end

function UtilityHandler:CreateDirectionalSmokeEffect(positionOrFullTable, source, angOrVector, power, spread, smokeMult, exploMult, widthSpread, velocityMult, lingerMult, airResistanceMult, gravMult)
	
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
			widthspread = 3;
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
			local tinySmokeCount = math.random(math.max(0, math.floor(power/25)), math.max(0, math.ceil(power/12.5)));
			for i = 1, tinySmokeCount do	
				local countFactor = i/tinySmokeCount;
				local randSpread = (math.random(-spread*500, spread*500)/1000) * countFactor;
				local easeFactor = math.abs(randSpread/(spread/2))
				factor = 1 - math.cos((easeFactor * math.pi) / 2);
				randSpread = randSpread * easeFactor;
				local randVelocityMult = RangeRand(0.5, 1.5);
				
				local particle = CreateMOSParticle("Tiny Smoke Ball 1", "Base.rte");
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

			-- Small smoke, medium spread
			local smallSmokeCount = math.random(math.max(0, math.floor(power/25)), math.max(0, math.floor(power/12.5)));
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
		
		local exploPower = power * smokeMult;
		count = exploPower / 5;
		velocity = Vector(exploPower / 1.5, 0):DegRotate(angle) * velocityMult;
		if source then
			velocity = velocity + source.Vel;
		end
		linger = (exploPower / 100) * lingerMult
		
		local exploCount = math.max(1, math.floor((count / (math.random(2,4))) * exploMult));
		for _ = 0, exploCount do
			
			-- Tiny explo particle
			local tinyExploCount = math.random(math.max(0, math.floor(power/25)), math.max(0, math.ceil(power/12.5)));
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
			local smallExploCount = math.random(math.max(0, math.floor(power/50)), math.max(0, math.floor(power/25)));
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
				particle.Lifetime = particle.Lifetime * RangeRand(0.75, 1.25) * lingerMult;
				particle.AirThreshold = (particle.AirThreshold / airResistanceMult) * (lingerMult/airResistanceMult);
				particle.AirResistance = math.min(62.5, particle.AirResistance * airResistanceMult);
				particle.GlobalAccScalar = particle.GlobalAccScalar * gravMult;
				particle.Team = source.Team;
				--particle.HitsMOs = false;
				MovableMan:AddParticle(particle);
			end
			
			-- Large explo particle 1
			local large1ExploCount = math.random(math.max(0, math.floor(power/260)), math.max(0, math.floor(power/130)));
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
				particle.AirResistance = math.min(60, particle.AirResistance * airResistanceMult);
				particle.GlobalAccScalar = particle.GlobalAccScalar * gravMult;
				particle.Team = source.Team;
				--particle.HitsMOs = false;
				MovableMan:AddParticle(particle);
			end
			
			-- Large explo particle 2
			local large2ExploCount = math.random(math.max(0, math.floor(power/300)), math.max(0, math.floor(power/200)));
			for i = 1, large2ExploCount do	
				local countFactor = i/large2ExploCount;
				local randSpread = (math.random(-spread*500, spread*500)/1000) * countFactor;
				local easeFactor = math.abs(randSpread/(spread/2))
				factor = 1 - math.cos((easeFactor * math.pi) / 2);
				randSpread = randSpread * easeFactor;
				local randVelocityMult = RangeRand(0.5, 1.5);
				
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
		print("ERROR: UtilityHandler tried to create a smoke effect but was not given a position, angle, or power amount!");
		return false;
	end
	
	return true;

end

return UtilityHandler:Create();