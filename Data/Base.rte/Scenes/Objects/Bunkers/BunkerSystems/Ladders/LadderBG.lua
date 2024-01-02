function Create(self)
	self.skipFrames = 2;
	self.checkDelay = math.ceil(self.skipFrames * TimerMan.DeltaTimeMS);
	self.idleDelay = self.checkDelay * 5;
	self.checkTimer = Timer();
	self.checkTimer:SetSimTimeLimitMS(self.idleDelay);
	self.checkCounter = 0;
	self.idleCheckLimit = 50;

	self.checkPos = self.Pos;

	self.width = (ToMOSprite(self):GetSpriteWidth()/5) - 1;
	self.height = (ToMOSprite(self):GetSpriteHeight()/3) - 1;

	self.gib = CreateTerrainObject("Destroyed Background Ladder");
	self.gib.Pos = Vector(self.Pos.X, self.Pos.Y);
end

function Update(self)
	if self.PinStrength ~= 0 then
		if self.checkTimer:IsPastSimTimeLimit() then
			self.checkTimer:Reset();
			self.Vel, self.AngularVel = Vector(), 0;
			local moCheck = SceneMan:GetMOIDPixel(self.checkPos.X, self.checkPos.Y);
			local actor;
			if moCheck ~= rte.NoMOID then
				local mo = MovableMan:GetMOFromID(MovableMan:GetMOFromID(moCheck).RootID);
				if mo and IsAHuman(mo) then
					actor = ToAHuman(MovableMan:GetMOFromID(mo.RootID));
					local controller = actor:GetController();
					local limbs = actor.FGLeg or actor.BGLeg or actor.FGArm or actor.BGArm;
					if limbs and actor.Status == Actor.STABLE and not controller:IsState(Controller.BODY_JUMP) then
						local velFactor = 1 + actor.Vel.Magnitude * 0.3;
						local aimAngle = actor:GetAimAngle(false);
						local climb = false;
						--Climb by looking up/down or moving
						if (aimAngle > 1.5 and controller:IsState(Controller.MOVE_UP)) or (aimAngle < -1.5 and controller:IsState(Controller.MOVE_DOWN)) then
							climb = true;
						end
						local gravity = SceneMan.GlobalAcc * TimerMan.DeltaTimeSecs;
						if climb or (controller:IsState(Controller.MOVE_LEFT) or controller:IsState(Controller.MOVE_RIGHT)) then
							local speed = actor:GetLimbPathSpeed(1)/velFactor;
							actor.Vel = actor.Vel * (1 - 1/velFactor) + Vector(speed, 0):RadRotate(actor:GetAimAngle(true)) - gravity * (0.4 + self.skipFrames/velFactor);
						elseif actor.Vel:MagnitudeIsLessThan(5) then
							--Counter gravity to keep actor still
							actor.Vel = Vector() - gravity * (0.2 + self.skipFrames/velFactor);
						end
					end
				end
			else
				self.checkPos = self.Pos + Vector(math.random(-self.width, self.width), math.random(-self.height, self.height));
			end
			--Go into a less frequent "idle" mode after enough empty checks
			if actor then
				self.checkCounter = self.idleCheckLimit;
				self.checkTimer:SetSimTimeLimitMS(self.checkDelay);
			elseif self.checkCounter ~= 0 then
				self.checkCounter = self.checkCounter - 1;
				if self.checkCounter <= 0 then
					self.checkCounter = 0;
					self.checkTimer:SetSimTimeLimitMS(self.idleDelay);
				end
			end
		end
	else
		self.ToDelete = true;
	end
end

function Destroy(self)
	--If this MO is somehow deleted, a new background sprite will indicate the destruction of the ladder
	if self.gib then
		SceneMan:AddSceneObject(self.gib);
	end
end