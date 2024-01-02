-- Rain "Manager" Particle

function WeatherRain(duration,ppm,pvel)
	local rainrunner = CreateMOPixel("Particle Rain Manager","Base.rte");
	rainrunner.Mass = duration;
	rainrunner.Sharpness = ppm;
	rainrunner.Pos = pvel;
	MovableMan:AddParticle(rainrunner);
end

function Create(self)
	self.lifeTimer = Timer();
	self.spawnTimer = Timer();

	self.duration = self.Mass;
	self.Mass = 1;

	self.particleDelay = 60/self.Sharpness;
	self.Sharpness = 0;

	self.particleVel = self.Pos;
	self.Pos = Vector(0,0);
end

function Update(self)
	if self.lifeTimer:IsPastSimMS(self.duration) then
		self.ToDelete = true;
	else
		self.PinStrength = 1000;
		self.ToDelete = false;
		self.ToSettle = false;
		self:NotResting();
		if self.spawnTimer:IsPastSimMS(self.particleDelay) then
			self.spawnTimer:Reset();
			local weatherpar = CreateMOPixel("Particle Rain Drop","Base.rte");
			weatherpar.Pos = Vector(math.random(0,SceneMan.SceneWidth),0);
			weatherpar.Vel = self.particleVel;
			MovableMan:AddParticle(weatherpar);
		end
	end
end