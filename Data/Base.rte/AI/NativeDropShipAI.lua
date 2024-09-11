require("Constants")
require("AI/PID");

NativeDropShipAI = {};

function NativeDropShipAI:Create(Owner)
	local Members = {};

	Members.StuckTimer = Timer();
	Members.HatchTimer = Timer();

	Members.AvoidTimer = Timer();
	Members.AvoidTimer:SetSimTimeLimitMS(500);

	Members.PlayerInterferedTimer = Timer();
	Members.PlayerInterferedTimer:SetSimTimeLimitMS(500);

	Members.LastAIMode = Actor.AIMODE_NONE;
	
	if Owner.AIMode == nil or not Owner.AIMode == Actor.AIMODE_GOTO then

		local item = Owner:Inventory();
		if item and IsTDExplosive(item) then
			Members.AIMode = Actor.AIMODE_BOMB;
		else
			Owner.AIMode = Actor.AIMODE_DELIVER;
		end
		
	end

	-- The drop ship tries to hover this many pixels above the ground
	Members.savedHoverHeightModifier = Owner.HoverHeightModifier;
	if Members.AIMode == Actor.AIMODE_BRAINHUNT then
		Members.hoverAlt = Owner.Radius * 1.7 + Owner.HoverHeightModifier;
	elseif Members.AIMode == Actor.AIMODE_BOMB then
		Members.hoverAlt = Owner.Radius * 6 + Owner.HoverHeightModifier;
	else
		Members.hoverAlt = Owner.Diameter + Owner.HoverHeightModifier;
	end

	-- The controllers
	Members.XposPID = RegulatorPID:New{p=0.05, i=0.01, d=2.5, filter_leak=0.8, integral_max=50};
	Members.YposPID = RegulatorPID:New{p=0.1, d=2.5, filter_leak=0.6};

	-- Check if this team is controlled by a human
	if Owner.AIMode == Actor.AIMODE_DELIVER and Owner:IsInventoryEmpty() and ActivityMan:GetActivity():IsHumanTeam(Owner.Team) then
		Owner.AIMode = Actor.AIMODE_STAY; -- Stop the craft from returning to orbit immediately
	end

	setmetatable(Members, self);
	self.__index = self;
	return Members;
end

function NativeDropShipAI:Update(Owner)
	local Ctrl = Owner:GetController();

	local hoverHeightModifierChanged = self.savedHoverHeightModifier ~= Owner.HoverHeightModifier;
	if hoverHeightModifierChanged then
		self.savedHoverHeightModifier = Owner.HoverHeightModifier;
		if Owner.AIMode == Actor.AIMODE_BRAINHUNT then
			self.hoverAlt = Owner.Radius * 1.7 + Owner.HoverHeightModifier;
		elseif Owner.AIMode == Actor.AIMODE_BOMB then
			self.hoverAlt = Owner.Radius * 6 + Owner.HoverHeightModifier;
		else
			self.hoverAlt = Owner.Diameter + Owner.HoverHeightModifier;
		end
	end

	if hoverHeightModifierChanged or Owner.AIMode ~= self.LastAIMode then
	
		Owner:UpdateMovePath();
		
		self.LastAIMode = Owner.AIMode;

		if Owner.AIMode == Actor.AIMODE_RETURN then
			self.DeliveryState = ACraft.LAUNCH;
			if SceneMan.SceneOrbitDirection == 0 then -- Go to orbit, whichever way it is
				self.Waypoint = Vector(Owner.Pos.X, -5000);
			elseif SceneMan.SceneOrbitDirection == 1 then
				self.Waypoint = Vector(Owner.Pos.X, SceneMan.SceneHeight + 5000);
			elseif SceneMan.SceneOrbitDirection == 2 then
				self.Waypoint = Vector(-5000, Owner.Pos.Y);
			elseif SceneMan.SceneOrbitDirection == 3 then
				self.Waypoint = Vector(SceneMan.SceneWidth + 5000, Owner.Pos.Y);
			end
		elseif Owner.AIMode == Actor.AIMODE_GOTO then
			self.Waypoint = nil;
		elseif Owner.AIMode == Actor.AIMODE_SENTRY then
			self.Waypoint = Owner.Pos;
			self.DeliveryState = ACraft.STANDBY;
		else -- Deliver, most likely
			local FuturePos = Owner.Pos + Owner.Vel*20;

			-- Make sure FuturePos is inside the scene
			if FuturePos.X > SceneMan.SceneWidth then
				if SceneMan.SceneWrapsX then
					FuturePos.X = FuturePos.X - SceneMan.SceneWidth;
				else
					FuturePos.X = SceneMan.SceneWidth - Owner.Radius;
				end
			elseif FuturePos.X < 0 then
				if SceneMan.SceneWrapsX then
					FuturePos.X = FuturePos.X + SceneMan.SceneWidth;
				else
					FuturePos.X = Owner.Radius;
				end
			end

			local startingHeight;
			if SceneMan.SceneOrbitDirection == 0 then
				startingHeight = hoverHeightModifierChanged and Owner.Radius * 1.25 or math.max(Owner.Radius * 1.25, Owner.Pos.Y);
			else
				startingHeight = Owner.Pos.Y;
			end
			local WptL = SceneMan:MovePointToGround(Vector(Owner.Pos.X - Owner.Radius, startingHeight), self.hoverAlt, 12);
			local WptC = SceneMan:MovePointToGround(Vector(Owner.Pos.X, startingHeight), self.hoverAlt, 12);
			local WptR = SceneMan:MovePointToGround(Vector(Owner.Pos.X + Owner.Radius, startingHeight), self.hoverAlt, 12);
			self.Waypoint = Vector(Owner.Pos.X, math.min(WptL.Y, WptC.Y, WptR.Y));

			self.DeliveryState = ACraft.FALL;
		end
	end

	if self.PlayerInterferedTimer:IsPastSimTimeLimit() then
		self.StuckTimer:Reset();

		local FuturePos = Owner.Pos + Owner.Vel*20;

		-- Make sure FuturePos is inside the scene
		if FuturePos.X > SceneMan.SceneWidth then
			if SceneMan.SceneWrapsX then
				FuturePos.X = FuturePos.X - SceneMan.SceneWidth;
			else
				FuturePos.X = SceneMan.SceneWidth - Owner.Radius;
			end
		elseif FuturePos.X < 0 then
			if SceneMan.SceneWrapsX then
				FuturePos.X = FuturePos.X + SceneMan.SceneWidth;
			else
				FuturePos.X = Owner.Radius;
			end
		end
		
		if self.Waypoint then
			local Dist = SceneMan:ShortestDistance(FuturePos, self.Waypoint, false);
			if math.abs(Dist.X) > 100 then
				if self.DeliveryState == ACraft.LAUNCH then
					self.Waypoint.X = FuturePos.X;
					self.Waypoint.Y = -500;
				else
					local startingHeight = math.max(Owner.Radius * 1.25, Owner.Pos.Y);
					local WptL = SceneMan:MovePointToGround(Vector(Owner.Pos.X - Owner.Radius, startingHeight), self.hoverAlt, 12);
					local WptC = SceneMan:MovePointToGround(Vector(Owner.Pos.X, startingHeight), self.hoverAlt, 12);
					local WptR = SceneMan:MovePointToGround(Vector(Owner.Pos.X + Owner.Radius, startingHeight), self.hoverAlt, 12);
					self.Waypoint = Vector(Owner.Pos.X, math.min(WptL.Y, WptC.Y, WptR.Y));
				end
			end
		end
	end

	self.PlayerInterferedTimer:Reset();
	
	if Owner.AIMode == Actor.AIMODE_GOTO then
	
		if Owner.IsWaitingOnNewMovePath then
			self.reachedWaypoint = false;
			self.Waypoint = nil;
			return;
		end

		if self.Waypoint == nil or self.reachedWaypoint then
			self.reachedWaypoint = false;
			for Wpt in Owner.MovePath do
				self.Waypoint = Wpt;
			end
		else
			local Dist = SceneMan:ShortestDistance(Owner.Pos, self.Waypoint, false);
			if Dist.Magnitude < 20 then
				if Owner:GetWaypointListSize() == 0 then
					Owner.AIMode = Actor.AIMODE_SENTRY;
					self.Waypoint = Owner.Pos;
				else
					Owner:ClearMovePath();
					Owner:UpdateMovePath();
					self.reachedWaypoint = true;
				end
			end
		end

	end

	-- Control right/left movement
	local Dist = SceneMan:ShortestDistance(Owner.Pos+Owner.Vel*30, self.Waypoint, false);
	local change = self.XposPID:Update(Dist.X, 0);
	if change > 0.6 then
		Ctrl.AnalogMove = Vector(change/8, 0);
	elseif change < -0.6 then
		Ctrl.AnalogMove = Vector(change/8, 0);
	end

	-- Control up/down movement
	Dist = SceneMan:ShortestDistance(Owner.Pos+Owner.Vel*5, self.Waypoint, false);
	change = self.YposPID:Update(Dist.Y, 0);
	if change > 2 then
		self.AltitudeMoveState = ACraft.DESCEND;
	elseif change < -2 then
		self.AltitudeMoveState = ACraft.ASCEND;
	end

	-- Delivery Sequence logic
	if Owner.AIMode == Actor.AIMODE_STAY or Owner.AIMode == Actor.AIMODE_DELIVER then
		if self.DeliveryState == ACraft.FALL then
			-- Don't descend if we have nothing to deliver
			if Owner:IsInventoryEmpty() and Owner.AIMode ~= Actor.AIMODE_BRAINHUNT then
				if Owner.AIMode ~= Actor.AIMODE_STAY then
					self.DeliveryState = ACraft.LAUNCH;
					self.HatchTimer:Reset();
					if SceneMan.SceneOrbitDirection == 0 then -- Go to orbit, whichever way it is
						self.Waypoint = Vector(Owner.Pos.X, -5000);
					elseif SceneMan.SceneOrbitDirection == 1 then
						self.Waypoint = Vector(Owner.Pos.X, SceneMan.SceneHeight + 5000);
					elseif SceneMan.SceneOrbitDirection == 2 then
						self.Waypoint = Vector(-5000, Owner.Pos.Y);
					elseif SceneMan.SceneOrbitDirection == 3 then
						self.Waypoint = Vector(SceneMan.SceneWidth + 5000, Owner.Pos.Y);
					end
				end
			else
				local dist = SceneMan:ShortestDistance(Owner.Pos, self.Waypoint, false);
				if dist:MagnitudeIsLessThan(Owner.Radius) and math.abs(change) < 3 and math.abs(Owner.Vel.X) < 4 then	-- If we passed the hover check, check if we can start unloading
					local WptL = SceneMan:MovePointToGround(Owner.Pos+Vector(-Owner.Radius, -Owner.Radius), self.hoverAlt, 12);
					local WptC = SceneMan:MovePointToGround(Owner.Pos+Vector(0, -Owner.Radius), self.hoverAlt, 12);
					local WptR = SceneMan:MovePointToGround(Owner.Pos+Vector(Owner.Radius, -Owner.Radius), self.hoverAlt, 12);
					self.Waypoint = Vector(Owner.Pos.X, math.min(WptL.Y, WptC.Y, WptR.Y));

					dist = SceneMan:ShortestDistance(Owner.Pos, self.Waypoint, false);
					if dist:MagnitudeIsLessThan(Owner.Diameter) then
						-- We are close enough to our waypoint
						if Owner.AIMode == Actor.AIMODE_STAY then
							self.DeliveryState = ACraft.STANDBY;
						else
							self.DeliveryState = ACraft.UNLOAD;
							self.HatchTimer:Reset();
						end
					end
				else
					-- Check for something in the way of our descent, and hover to the side to avoid it
					if self.AvoidTimer:IsPastSimTimeLimit() then
						self.AvoidTimer:Reset();

						self.search = not self.search; -- Search every second update
						if self.search then
							local obstID = Owner:DetectObstacle(Owner.Diameter + Owner.Vel.Magnitude * 70);
							if obstID > 0 and obstID ~= rte.NoMOID then
								local MO = MovableMan:GetMOFromID(MovableMan:GetRootMOID(obstID));
								if MO.ClassName == "ACDropShip" or MO.ClassName == "ACRocket" then
									self.AvoidMoveState = ACraft.HOVER;
									self.Waypoint.X = self.Waypoint.X + Owner.Diameter * 2;

									-- Make sure the LZ is inside the scene
									if self.Waypoint.X > SceneMan.SceneWidth then
										if SceneMan.SceneWrapsX then
											self.Waypoint.X = self.Waypoint.X - SceneMan.SceneWidth;
										else
											self.Waypoint.X = SceneMan.SceneWidth - Owner.Radius;
										end
									end
								end
							else
								self.AvoidMoveState = nil;
							end
						else	-- Avoid terrain
							local Free = Vector();
							local Start = Owner.Pos + Vector(Owner.Radius, 0);
							local Trace = Owner.Vel * (Owner.Radius/2) + Vector(0,50);
							if PosRand() < 0.5 then
								Start.X = Start.X - Owner.Diameter;
							end

							if SceneMan:CastStrengthRay(Start, Trace, 0, Free, 4, 0, true) then
								self.Waypoint.X = Owner.Pos.X;
								self.Waypoint.Y = Free.Y - self.hoverAlt;
							end
						end
					end

					if self.AvoidMoveState then
						self.AltitudeMoveState = self.AvoidMoveState;
					end
				end
			end
		elseif self.DeliveryState == ACraft.UNLOAD then
			if self.HatchTimer:IsPastSimMS(500) then	-- Start unloading if there's something to unload
				self.HatchTimer:Reset();
				Owner:OpenHatch();

				if Owner.AIMode == Actor.AIMODE_BRAINHUNT and Owner:HasObjectInGroup("Brains") then
					Owner.AIMode = Actor.AIMODE_RETURN;
				else
					self.DeliveryState = ACraft.FALL;
				end
			end
		elseif self.DeliveryState == ACraft.LAUNCH then
			if self.HatchTimer:IsPastSimMS(1000) then
				self.HatchTimer:Reset();
				Owner:CloseHatch();
			end

			-- Check for something in the way of our ascent, and hover to the side to avoid it
			if self.AvoidTimer:IsPastSimTimeLimit() then
				self.AvoidTimer:Reset();

				local obstID = Owner:DetectObstacle(Owner.Diameter + Owner.Vel.Magnitude * 70);
				if obstID > 0 and obstID ~= rte.NoMOID then
					local MO = MovableMan:GetMOFromID(MovableMan:GetRootMOID(obstID));
					if MO.ClassName == "ACDropShip" or MO.ClassName == "ACRocket" then
						self.AvoidMoveState = ACraft.HOVER;
						self.Waypoint.X = self.Waypoint.X - Owner.Diameter * 2;

						-- Make sure the LZ is inside the scene
						if self.Waypoint.X < 0 then
							if SceneMan.SceneWrapsX then
								self.Waypoint.X = self.Waypoint.X + SceneMan.SceneWidth;
							else
								self.Waypoint.X = Owner.Radius;
							end
						end
					end
				else
					self.AvoidMoveState = nil;
				end
			end

			if self.AvoidMoveState then
				self.AltitudeMoveState = self.AvoidMoveState;
			end
		end
	else
		self.DeliveryState = ACraft.FALL;
	end

	-- Input translation
	if self.AltitudeMoveState == ACraft.ASCEND then
		Ctrl:SetState(Controller.MOVE_UP, true);
	elseif self.AltitudeMoveState == ACraft.DESCEND then
		Ctrl:SetState(Controller.MOVE_DOWN, true);
	else
		Ctrl:SetState(Controller.MOVE_UP, false);
		Ctrl:SetState(Controller.MOVE_DOWN, false);
	end
	
	if Owner.LeftEngine == nil and Owner.RightEngine == nil and self.StuckTimer.ElapsedSimTimeMS < 35000 then
		self.StuckTimer.ElapsedSimTimeMS = 35000;
	end

	-- If we are hopelessly stuck, self destruct
	if Owner.Vel.Largest > 3 or Owner.AIMode == Actor.AIMODE_STAY or Owner.AIMode == Actor.AIMODE_SENTRY or Owner.AIMode == Actor.AIMODE_GOTO then
		self.StuckTimer:Reset();
	elseif Owner.AIMode == Actor.AIMODE_SCUTTLE or self.StuckTimer:IsPastSimMS(40000) then
		Owner:GibThis();
	end
end
