require("AI/PID");

function Create(self)
	---------------- AI variables start ----------------
	self.StableTimer = Timer();
	self.StuckTimer = Timer();
	self.DoorTimer = Timer();

	self.ObstacleTimer = Timer();
	self.ObstacleTimer:SetSimTimeLimitMS(100);

	self.PlayerInterferedTimer = Timer();
	self.PlayerInterferedTimer:SetSimTimeLimitMS(500);

	self.DeliveryState = ACraft.FALL;
	self.LastAIMode = Actor.AIMODE_NONE;
	self.groundDist = self.Radius/1.35;

	self.LZpos = SceneMan:MovePointToGround(self.Pos, self.groundDist, 9);
	self.velIntegrator = 0;

	function self:MoveLZ()
		local FuturePos = self.Pos + self.Vel * 7;

		-- Make sure FuturePos is inside the scene
		if FuturePos.X > SceneMan.SceneWidth then
			if SceneMan.SceneWrapsX then
				FuturePos.X = FuturePos.X - SceneMan.SceneWidth;
			else
				FuturePos.X = SceneMan.SceneWidth - self.Radius;
			end
		elseif FuturePos.X < 0 then
			if SceneMan.SceneWrapsX then
				FuturePos.X = FuturePos.X + SceneMan.SceneWidth;
			else
				FuturePos.X = self.Radius;
			end
		end

		if self.DeliveryState == ACraft.LAUNCH then
			self.LZpos.X = FuturePos.X;
		else
			local TestPos = Vector(FuturePos.X, math.min(FuturePos.Y, self.Pos.Y));
			self.LZpos = SceneMan:MovePointToGround(TestPos, self.groundDist, 5);
		end
	end

	-- The controllers
	self.AngPID = RegulatorPID:New{p=2.7, i=0.01, d=0.9, last_input=self.RotAngle, filter_leak=0.6, integral_max=150};
	self.XposPID = RegulatorPID:New{p=0.2, d=0.5, filter_leak=0.6, integral_max=100};
	self.YposPID = RegulatorPID:New{p=0.02, i=0.07, d=4, last_input=self.LZpos.Y, filter_leak=0.5, integral_max=30};

	-- Check if this team is controlled by a human
	if self.AIMode == Actor.AIMODE_DELIVER and self:IsInventoryEmpty() and ActivityMan:GetActivity():IsHumanTeam(self.Team) then
		self.AIMode = Actor.AIMODE_STAY;	-- Stop the craft from returning to orbit immediately
	end
	---------------- AI variables end ----------------
end

function ThreadedUpdateAI(self)
	self.Ctrl = self:GetController();

	if self.PlayerInterferedTimer:IsPastSimTimeLimit() then
		self.StuckTimer:Reset();
		self:MoveLZ();
	end

	self.PlayerInterferedTimer:Reset();

	if self.AIMode ~= self.LastAIMode then
		self.LastAIMode = self.AIMode;

		if self.AIMode == Actor.AIMODE_RETURN then
			self.DeliveryState = ACraft.LAUNCH;
			self.LZpos.Y = -10000;	-- Go to orbit
			self:MoveLZ();
		else
			self.DeliveryState = ACraft.FALL;
			self:MoveLZ();
		end
	end

	-- Reset StableTimer if not in a stable and upright state
	self.velIntegrator = self.velIntegrator * 0.8 + self.Vel.Magnitude * 0.2;
	if self.velIntegrator > 5 or math.abs(self.AngularVel) > 1 then
		self.StableTimer:Reset();
	else
		self.LZpos.X = self.Pos.X;
	end

	-- Delivery Sequence logic
	if self.DeliveryState == ACraft.FALL then
		-- Move Landing Zone closer to the rocket if it is about to collide with something
		if (self.LZpos.Y - self.Pos.Y) > self.Radius then
			local Speed = (self.Vel + Vector(0, 4)) * 15;
			if SceneMan:CastObstacleRay(self.Pos, Speed, Vector(), Vector(), self.ID, self.IgnoresWhichTeam, 0, 10) > -1 then
				self:MoveLZ();
			end
		end

		-- Check for something in the way of our descent, and move to the side to avoid it
		if self.ObstacleTimer:IsPastSimTimeLimit() then
			self.ObstacleTimer:Reset();

			local Trace = Vector(self.Vel.X + RangeRand(-1, 1), math.max(self.Vel.Y, 4)) * 40;
			local obstID = SceneMan:CastMORay(self.Pos, Trace, self.ID, self.IgnoresWhichTeam, 0, false, 5);
			if obstID ~= rte.NoMOID then
				local MO = MovableMan:GetMOFromID(MovableMan:GetRootMOID(obstID));
				if MO.ClassName == "ACDropShip" or MO.ClassName == "ACRocket" then
					self.LZpos:SetXY(MO.Pos.X + MO.Radius * 3, math.max(self.Pos.Y - 200, 0));
					self.ObstacleTimer:SetSimTimeLimitMS(750);
					self.obstacle = true;
				elseif MovableMan:IsActor(MO) and MO.Team == self.Team then
					local newLZx = MO.Pos.X + MO.Diameter + self.Diameter;

					-- Make sure newLZx is inside the scene
					if newLZx > SceneMan.SceneWidth then
						if SceneMan.SceneWrapsX then
							newLZx = newLZx - SceneMan.SceneWidth;
						else
							newLZx = MO.Pos.X - (MO.Diameter + self.Diameter);
						end
					end

					self.LZpos:SetXY(newLZx, math.max(MO.Pos.Y - 200, 0));
				end
			elseif self.obstacle then
				self.obstacle = false;
				self:MoveLZ();
				self.ObstacleTimer:SetSimTimeLimitMS(100);
			end
		end

		if self.AIMode == Actor.AIMODE_DELIVER and self:IsInventoryEmpty() then
			self.AIMode = Actor.AIMODE_RETURN;
			self.DeliveryState = ACraft.LAUNCH;	-- Don't descend if we have nothing to deliver
			self.LZpos.Y = -10000;	-- Go to orbit
		else
			if self.StableTimer:IsPastSimMS(500) then	-- Move LZ if stable
				self.LZpos = SceneMan:MovePointToGround(self.Pos, self.groundDist, 6);
			end

			if self.AIMode ~= Actor.AIMODE_STAY and SceneMan:ShortestDistance(self.Pos, self.LZpos, false):MagnitudeIsLessThan(25) then
				self.DeliveryState = ACraft.UNLOAD;
			end
		end
	elseif self.DeliveryState == ACraft.UNLOAD then
		if self:IsInventoryEmpty() and self.AIMode ~= Actor.AIMODE_STAY then	-- Return to orbit if empty
			if self.DoorTimer:IsPastSimMS(750) then	-- Pause before returning to orbit
				self.AIMode = Actor.AIMODE_RETURN;
				self.DeliveryState = ACraft.LAUNCH;
				self.LZpos.Y = -10000;	-- Go to orbit
				if self.HatchState == ACraft.OPEN then
					self:CloseHatch();
				end
			end
		elseif self.StableTimer:IsPastSimMS(300) and self.HatchState == ACraft.CLOSED then
			self:OpenHatch();
			self.DoorTimer:Reset();
		end
	elseif self.DeliveryState == ACraft.LAUNCH then
		-- Check for something in the way of our ascent, and move to the side to avoid it
		if self.ObstacleTimer:IsPastSimTimeLimit() then
			self.ObstacleTimer:Reset();

			local Trace = Vector(self.Vel.X + RangeRand(-1, 1), math.min(self.Vel.Y, -1)) * 50;
			local obstID = SceneMan:CastMORay(self.Pos, Trace, self.ID, self.IgnoresWhichTeam, 0, false, 5);
			if obstID ~= rte.NoMOID then
				local MO = MovableMan:GetMOFromID(MovableMan:GetRootMOID(obstID));
				if MO.ClassName == "ACDropShip" or MO.ClassName == "ACRocket" then
					self.LZpos:SetXY(MO.Pos.X - (MO.Diameter + self.Diameter), self.Pos.Y + self.Vel.Y);
					self.ObstacleTimer:SetSimTimeLimitMS(750);
					self.obstacle = true;
				end
			elseif self.obstacle then
				self.obstacle = false;
				self.LZpos.Y = -10000;	-- Go to orbit
				self:MoveLZ();
				self.ObstacleTimer:SetSimTimeLimitMS(100);
			else
				self:MoveLZ();	-- move the exit point closer towards us
			end
		end
	end
	
	if self.AIMode == Actor.AIMODE_GOTO then
		self.LZpos = self:GetLastAIWaypoint();
		self.DeliveryState = ACraft.LAUNCH;
	end
	
	local dist
	local change

	-- Control up/down movement
	if self.DeliveryState ~= ACraft.UNLOAD then
		dist = SceneMan:ShortestDistance(self.Pos+self.Vel, self.LZpos, false);
		change = self.YposPID:Update(dist.Y, 0);
		if math.abs(self.RotAngle) < 0.9 then
			if self.AIMode == Actor.AIMODE_GOTO then
				if change < 0 then
					self.burstUp = nil;
					self.Ctrl:SetState(Controller.MOVE_UP, true);
				end
			else
				if self.DeliveryState == ACraft.LAUNCH and change < -7 then
					self.burstUp = nil;
					self.Ctrl:SetState(Controller.MOVE_UP, true);	-- Don't burst when returning to orbit
				else
					if change < -2.2 and not self.burstUp then
						self.burstUp = math.max(9 - change, 2); -- Wait n frames until next burst (lower -> better control)
						if change < -6 then
							self.Ctrl:SetState(Controller.MOVE_UP, true);
							self.burstUp = nil;
						end
					elseif change > 20 then
						self.burstUp = nil;
						self.Ctrl:SetState(Controller.MOVE_DOWN, true);
					end
				end
			end
		elseif self.RotAngle > 2.14 and self.RotAngle < 4.14 then	-- Upside down
			self.Ctrl:SetState(Controller.MOVE_DOWN, true);
		end
	end

	-- Control right/left movement (the rocket will move sideways if rotated to the side)
	dist = SceneMan:ShortestDistance(self.Pos + self.Vel * 20, self.LZpos, false).X;
	change = self.XposPID:Update(dist, 0);
	local targetAng = 0;
	if self.Vel.Y > 0 then
		if change < -4 then
			targetAng = -math.max(change/40, -0.5);
		elseif change > 4 then
			targetAng = -math.min(change/40, 0.5);
		end
	else
		if change > 4 then
			targetAng = -math.max(change/40, -0.5);
		elseif change < -4 then
			targetAng = -math.min(change/40, 0.5);
		end
	end

	-- Control angle

	change = self.AngPID:Update(self.RotAngle + self.AngularVel, targetAng);
	-- Don't burst side thrusters from minimal tilt
	local tilted = math.floor(math.abs(self.RotAngle) + 0.9);
	if tilted ~= 0 then
		if change > 1.1 and not self.burstRight then
			self.burstRight = math.max(5 - change, 2); -- Wait n frames until next burst (lower -> better control)
		elseif change < -1.1 and not self.burstLeft then
			self.burstLeft = math.max(5 + change, 2);
		end
	end

	-- Trigger bursts
	if self.burstRight then
		self.burstRight = self.burstRight - SettingsMan.AIUpdateInterval;
		if self.burstRight < 0 then
			self.Ctrl:SetState(Controller.MOVE_RIGHT, true);
			if self.burstRight < -4 then	-- Fire for -n frames (higher -> better control)
				self.burstRight = nil;	-- Allow a new burst
			end
		end
	end

	if self.burstLeft then
		self.burstLeft = self.burstLeft - SettingsMan.AIUpdateInterval;
		if self.burstLeft < 0 then
			self.Ctrl:SetState(Controller.MOVE_LEFT, true);
			if self.burstLeft < -4 then
				self.burstLeft = nil;
			end
		end
	end

	if self.burstUp then
		self.burstUp = self.burstUp - SettingsMan.AIUpdateInterval;
		if self.burstUp < 0 then
			self.Ctrl:SetState(Controller.MOVE_UP, true);
			if self.burstUp < -8 then
				self.burstUp = nil;
			end
		end
	end

	if self.AIMode == Actor.AIMODE_STAY then
		self.StuckTimer:Reset();
		-- Don't burst main thruster when stationary, as it results in hatches closing
		if self.Vel.Largest < 6 and self.AIMode == Actor.AIMODE_STAY then
			self.Ctrl:SetState(Controller.MOVE_UP, false);
		end
	elseif self.Vel.Largest > 3 then
		self.StuckTimer:Reset();
	-- If we are hopelessly stuck, self destruct
	elseif self.AIMode == Actor.AIMODE_SCUTTLE or self.StuckTimer:IsPastSimMS(40000) then
		self:GibThis();
	end
end