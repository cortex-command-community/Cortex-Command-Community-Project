-- Weather "Manager" Particle

function WeatherSnow(duration,ppm,pvel)
	local snowrunner = CreateMOPixel("Particle Snow Manager","Base.rte");
	snowrunner.Mass = duration;
	snowrunner.Sharpness = ppm;
	snowrunner.Pos = pvel;
	MovableMan:AddParticle(snowrunner);
end

function Create(self)
	self.particleList = {"Particle Snow Flake A","Particle Snow Flake B","Particle Snow Flake C"};

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
			local weatherpar = CreateMOPixel(self.particleList[math.random(1,#self.particleList)],"Base.rte");
			weatherpar.Pos = Vector(math.random(0,SceneMan.SceneWidth),0);
			weatherpar.Vel = self.particleVel;
			MovableMan:AddParticle(weatherpar);
		end
	end
end