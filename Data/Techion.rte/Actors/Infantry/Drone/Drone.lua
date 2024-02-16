
dofile("Base.rte/Constants.lua")
require("AI/NativeCrabAI")	--dofile("Base.rte/AI/NativeCrabAI.lua")


function Create(self)

	self.c = self:GetController();
	self.setalt = 35; --64;	

	self.AI = NativeCrabAI:Create(self)

end


function Update(self)

if self.Health > 0 then
self:MoveOutOfTerrain(1);

	--propulsion, stabilisation

	self.RotAngle = 0
	
	self.alt = self:GetAltitude(0,1);
	
	if self.alt < self.setalt and self.Vel.Y > -5 then
		self.Vel.Y = self.Vel.Y - 0.5;
	end
	
	if self.Vel.Y < -1 then
		self.Vel.Y = self.Vel.Y + 0.05;
	elseif self.Vel.Y > 1 then	
		self.Vel.Y = self.Vel.Y - 0.05;
	end
	self.Vel.Y = 0.985*self.Vel.Y;
	
	
	if self.Vel.X < 15 and self.c:IsState(3) then
		self.Vel.X = self.Vel.X + (math.cos(self.RotAngle) * 0.1);
		self.Vel.Y = self.Vel.Y - (math.sin(self.RotAngle) * 0.1);
	elseif self.Vel.X > -15 and self.c:IsState(4) then
		self.Vel.X = self.Vel.X - (math.cos(self.RotAngle) * 0.1);
		self.Vel.Y = self.Vel.Y + (math.sin(self.RotAngle) * 0.1);
 	else
 	    self.Vel.X = self.Vel.X*0.80;
	end

	if self.setalt < 95 and self.c:IsState(5) then
		self.setalt = self.setalt + 0.75;
	elseif self.setalt > 45 and self.c:IsState(6) then
		self.setalt = self.setalt - 0.75;
	end
	


	if self:IsPlayerControlled() and UInputMan:KeyHeld(24) then
		self.RotAngle = self.RotAngle - 0.1;
		if self.AngularVel > 30 then
			self.angularVel = self.AngularVel - 3;
		end
		if self.angularVel < -30 then
			self.AngularVel = self.AngularVel + 3;
		end
	end



	if self:IsPlayerControlled() and UInputMan:KeyHeld(26) then
		self.RotAngle = self.RotAngle + 0.1;
		if self.AngularVel > 30 then
			self.angularVel = self.AngularVel - 3;
		end
		if self.angularVel < -30 then
			self.AngularVel = self.AngularVel + 3;
		end
	end

end
end

function UpdateAI(self)
	self.AI:Update(self)
end