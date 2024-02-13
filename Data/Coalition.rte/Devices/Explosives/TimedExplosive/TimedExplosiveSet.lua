function Create(self)
	self.lifeTimer = Timer();
	self.blinkTimer = Timer();
	self.blipTimer = Timer();

	self.actionPhase = 0;
	self.changeCounter = 0;
	self.stuck = false;
	self.blipdelay = 1000;

	self.minBlipDelay = 100;
	self.medBlipDelay = 250;
	self.maxBlipDelay = 500;

	self.detonateDelay = self:NumberValueExists("DetonationDelay") and self:GetNumberValue("DetonationDelay") or 11000;
	self.Frame = 1;

	if TimedExplosiveTable == nil then
		TimedExplosiveTable = {};
	end

	self.tableNum = #TimedExplosiveTable + 1;
	TimedExplosiveTable[self.tableNum] = self;

	self.activateSound = CreateSoundContainer("Explosive Device Activate", "Base.rte");
	self.blipSound = CreateSoundContainer("Timed Explosive Blip", "Coalition.rte");

	TimedExplosiveStick(self);
end

function Update(self)
	if TimedExplosiveTable == nil then
		TimedExplosiveTable = {};
		TimedExplosiveTable[self.tableNum] = self;
	end

	TimedExplosiveStick(self);

	if self.stuck then
		if self.lifeTimer:IsPastSimMS(self.detonateDelay) then
			self:GibThis();
		else
			self.ToDelete = false;
			self.ToSettle = false;

			local number = math.ceil((self.detonateDelay - self.lifeTimer.ElapsedSimTimeMS) * 0.01) * 0.1;
			local text = "".. number;
			if number == math.ceil(number) then
				text = text ..".0";
			end
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do

				local screen = ActivityMan:GetActivity():ScreenOfPlayer(player);
				if screen ~= -1 and not SceneMan:IsUnseen(self.Pos.X, self.Pos.Y, ActivityMan:GetActivity():GetTeamOfPlayer(player)) then
					PrimitiveMan:DrawTextPrimitive(screen, self.Pos + Vector(-5, -self.Diameter), text, true, 0);
				end
			end
		end

		if self.blipTimer:IsPastSimMS(50) then
			self.Frame = 0;
		else
			self.Frame = 1;
		end

		if self.blipTimer:IsPastSimMS(self.blipdelay) then
			self.blipTimer:Reset();
			self.blinkTimer:Reset();
			self.blipSound:Play(self.Pos);

			if self.changeCounter == 0 and self.lifeTimer.ElapsedSimTimeMS > (self.detonateDelay * 0.85 - 5000) then
				self.changeCounter = 1;
				self.blipdelay = self.maxBlipDelay;
			end

			if self.changeCounter == 1 and self.lifeTimer.ElapsedSimTimeMS > (self.detonateDelay * 0.90 - 3000) then
				self.changeCounter = 2;
				self.blipdelay = self.medBlipDelay;
			end

			if self.changeCounter == 2 and self.lifeTimer.ElapsedSimTimeMS > (self.detonateDelay * 0.95 - 1000) then
				self.changeCounter = 3;
				self.blipdelay = self.minBlipDelay;
			end
		end
	end
	if self.Sharpness == 1 then
		self.ToDelete = true;
	end
end

function TimedExplosiveStick(self)
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

function Destroy(self)
	TimedExplosiveTable[self.tableNum] = nil;
end