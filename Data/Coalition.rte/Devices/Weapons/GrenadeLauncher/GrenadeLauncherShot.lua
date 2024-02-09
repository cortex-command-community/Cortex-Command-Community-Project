function Create(self)
	self.actionPhase = 0;
	self.stuck = false;
	self.lifeTimer = Timer();
	self.mode = self:GetStringValue("GrenadeMode");
	self.fuzeDelay = 3500;
	self.sticky = self.mode == "Remote" or self.mode == "Timed";
end

function Update(self)
	if self.mode == "Remote" then
		self.ToDelete = false;
		self.ToSettle = false;
		--Keep checking for triggers when in "Remote" status
		self.mode = self:GetStringValue("GrenadeMode");
	elseif self.mode == "Delete" then
		self.ToDelete = true;
	elseif self.mode == "Detonate" or self.lifeTimer:IsPastSimMS(self.fuzeDelay) then
		local explosion = CreateMOSRotating("Coalition Grenade Launcher Explosion", "Coalition.rte");
		explosion.Pos = self.Pos;
		explosion:GibThis();
		MovableMan:AddParticle(explosion);
		self.ToDelete = true;
	else
		self.ToDelete = false;
		self.ToSettle = false;
	end

	if self.sticky then
		if self.actionPhase == 0 then
			local rayHitPos = Vector();
			local rayHit = false;
			local trace = Vector(self.Vel.X, self.Vel.Y):SetMagnitude(self.Vel.Magnitude * rte.PxTravelledPerFrame + self.Radius);
			local dots = trace.Magnitude/2;
			for i = 1, dots do
				local checkPos = self.Pos + Vector(trace.X, trace.Y) * (i/dots);
				local checkPix = SceneMan:GetMOIDPixel(checkPos.X,checkPos.Y);
				if checkPix ~= rte.NoMOID and checkPix ~= self.ID and MovableMan:GetMOFromID(checkPix).Team ~= self.Team then
					checkPos = checkPos + SceneMan:ShortestDistance(checkPos, self.Pos, SceneMan.SceneWrapsX):SetMagnitude(3);
					self.target = MovableMan:GetMOFromID(checkPix);
					self.stickPosition = SceneMan:ShortestDistance(self.target.Pos, checkPos, SceneMan.SceneWrapsX);
					self.stickRotation = self.target.RotAngle;
					self.stickDirection = self.RotAngle;
					self.stuck = true;
					rayHit = true;
					break;
				end
			end
			
			if rayHit == true then
				self.actionPhase = 1;
			else
				if SceneMan:CastStrengthRay(self.Pos, trace, 0, rayHitPos, 0, rte.airID, SceneMan.SceneWrapsX) == true then
					self.Pos = rayHitPos + SceneMan:ShortestDistance(rayHitPos, self.Pos, SceneMan.SceneWrapsX):SetMagnitude(3);
					self.PinStrength = 1000;
					self.Vel = Vector();
					self.AngularVel = 0;
					self.stuck = true;
					self.actionPhase = 2;
					self.HitsMOs = false;
				end
			end
		elseif self.actionPhase == 1 then
			if self.target ~= nil and self.target.ID ~= rte.NoMOID then
				self.Pos = self.target.Pos + Vector(self.stickPosition.X, self.stickPosition.Y):RadRotate(self.target.RotAngle - self.stickRotation);
				self.RotAngle = self.stickDirection + (self.target.RotAngle - self.stickRotation);
				self.PinStrength = 1000;
				self.Vel = Vector();
				self.AngularVel = 0;
				self.HitsMOs = false;
			else
				self.PinStrength = 0;
				self.actionPhase = 0;
				self.HitsMOs = true;
			end
		end
	elseif self.mode == "Impact" and self.TravelImpulse:MagnitudeIsGreaterThan(self.Mass) then
		self.fuzeDelay = self.fuzeDelay - self.TravelImpulse.Magnitude * 30;
	end
end

function Destroy(self)
	ActivityMan:GetActivity():ReportDeath(self.Team, -1);
end