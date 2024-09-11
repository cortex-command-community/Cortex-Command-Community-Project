function OnMessage(self, message, object)
	if message == "RemoteExplosive_Detonate" and object == self.alliedTeam then
		self.Vel = Vector(10, 0):RadRotate(self.RotAngle);
		self:GibThis();
	end
end

function Create(self)
	self.blinkTimer = Timer();

	self.actionPhase = 0;
	self.blink = false;
	self.stuck = false;

	self.alliedTeam = self.Sharpness;
	self.Team = self.alliedTeam;
	self.Sharpness = 0;
	self.autoDetonate = self:NumberValueExists("AutoDetonate");

	self.activateSound = CreateSoundContainer("Explosive Device Activate", "Base.rte");

	RemoteExplosiveStick(self);
end

function ThreadedUpdate(self)
	--TODO: Remove Sharpness hack!
	if self.Sharpness == 0 then
		self.ToDelete = false;
		self.ToSettle = false;
	elseif self.Sharpness == 1 then
		self.ToDelete = true;
	elseif self.Sharpness == 2 then	--Explode
		self.Vel = Vector(10, 0):RadRotate(self.RotAngle);
		self:GibThis();
	end

	RemoteExplosiveStick(self);

	if self.stuck then
		if self.blinkTimer:IsPastSimMS(500) then
			if self.autoDetonate then
				self.Sharpness = 2;
			end
			self.blinkTimer:Reset();
			if self.blink == false then
				self.blink = true;
				self.Frame = (self.alliedTeam + 1) * 2;
			else
				self.blink = false;
				self.Frame = ((self.alliedTeam + 1) * 2) + 1;
			end
		end
	else
		self.blinkTimer:Reset();
	end
end

function RemoteExplosiveStick(self)
	if self.actionPhase == 0 then
		local checkVec = Vector(self.Vel.X, self.Vel.Y + 1):SetMagnitude(math.max(self.Vel.Magnitude * rte.PxTravelledPerFrame, self.Radius));
		--Find a user to ignore hits with
		if not self.userID then
			self.userID = rte.NoMOID;
			local moCheck = SceneMan:CastMORay(self.Pos, checkVec * (-2), self.ID, -1, rte.airID, true, 1);
			if moCheck ~= rte.NoMOID then
				local rootID = MovableMan:GetMOFromID(moCheck).RootID;
				if rootID ~= rte.NoMOID then
					self.userID = rootID;
				end
			end
		end
		local rayHitPos = Vector();
		local rayHit = false;
		for i = 1, 2 do
			local checkPos = self.Pos + (checkVec/i);
			local checkPix = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
			if checkPix ~= rte.NoMOID and MovableMan:GetMOFromID(checkPix).RootID ~= self.userID then
				checkPos = checkPos + SceneMan:ShortestDistance(checkPos, self.Pos, SceneMan.SceneWrapsX):SetMagnitude(ToMOSprite(self):GetSpriteWidth() * 0.5 - 1);
				self.target = ToMOSRotating(MovableMan:GetMOFromID(checkPix));
				self.stickPosition = SceneMan:ShortestDistance(self.target.Pos, checkPos, SceneMan.SceneWrapsX);
				self.stickRotation = self.target.RotAngle;
				self.stickDirection = self.RotAngle;

				if self.activateSound then
					self.activateSound:Play(self.Pos);
				end
				self.stuck = true;
				rayHit = true;
				break;
			end
		end
		if rayHit then
			self.actionPhase = 1;
		elseif SceneMan:CastStrengthRay(self.Pos, checkVec, 0, rayHitPos, 1, rte.airID, SceneMan.SceneWrapsX) then
			self.Pos = rayHitPos + SceneMan:ShortestDistance(rayHitPos, self.Pos, SceneMan.SceneWrapsX):SetMagnitude(ToMOSprite(self):GetSpriteWidth() * 0.5 - 1);
			self.PinStrength = 1000;
			self.AngularVel = 0;
			self.stuck = true;
			self.actionPhase = 2;

			if self.activateSound then
				self.activateSound:Play(self.Pos);
			end
		end
	else
		self.Vel = Vector();
		self.AngularVel = 0;
		if self.actionPhase == 1 then
			if self.target and self.target.ID ~= rte.NoMOID and not self.target.ToDelete then
				self.Pos = self.target.Pos + Vector(self.stickPosition.X, self.stickPosition.Y):RadRotate(self.target.RotAngle - self.stickRotation);
				self.RotAngle = self.stickDirection + (self.target.RotAngle - self.stickRotation);
				self.PinStrength = 1000;
				self.Vel = Vector();
			else
				self.PinStrength = 0;
				self.actionPhase = 0;
			end
		end
	end
end