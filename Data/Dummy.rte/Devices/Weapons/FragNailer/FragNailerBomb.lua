function Create(self)
	self.toDetonate = false;
	self.detonateTimer = Timer();
	self.detonateDelay = 2000;

	self.actionPhase = 0;
	self.stuck = false;
end

function Update(self)
	self.ToDelete = false;

	if self.actionPhase == 0 then
		local rayHitPos = Vector();
		local rayHit = false;
		local dots = math.sqrt(self.Vel.Magnitude);
		local trace = Vector(self.Vel.X, self.Vel.Y):SetMagnitude(self.Vel.Magnitude * rte.PxTravelledPerFrame + 1);
		for i = 1, dots do
			local checkPos = self.Pos + trace * (i/dots);
			local checkPix = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
			if checkPix ~= rte.NoMOID and (self.ID == rte.NoMOID or checkPix ~= self.ID) and MovableMan:GetMOFromID(checkPix).Team ~= self.Team then
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
			if SceneMan:CastStrengthRay(self.Pos, trace, 0, rayHitPos, 1, rte.airID, SceneMan.SceneWrapsX) then
				self.Pos = rayHitPos + SceneMan:ShortestDistance(rayHitPos, self.Pos, SceneMan.SceneWrapsX):SetMagnitude(3);
				self.PinStrength = 1000;
				self.AngularVel = 0;
				self.stuck = true;
				self.actionPhase = 2;
				self.HitsMOs = false;
			end
		end
	elseif self.actionPhase == 1 then
		if self.target and self.target.ID ~= rte.NoMOID and not self.target.ToDelete then
			self.Pos = self.target.Pos + Vector(self.stickPosition.X, self.stickPosition.Y):RadRotate(self.target.RotAngle - self.stickRotation);
			self.RotAngle = self.stickDirection + (self.target.RotAngle - self.stickRotation);
			self.PinStrength = 1000;
			self.Vel = Vector();
			self.HitsMOs = false;
		else
			self.toDetonate = true;
		end
	end

	if self.detonateTimer:IsPastSimMS(self.detonateDelay) or self.toDetonate then
		local explosion = CreateMOSRotating("Particle Dummy Frag Nailer Explosion");
		explosion.Pos = self.Pos;
		explosion:GibThis();
		MovableMan:AddParticle(explosion);
		self.ToDelete = true;
	end
end