function Create(self)
	self.mapWrapsX = SceneMan.SceneWrapsX;
	self.climbTimer = Timer();
	self.mouseClimbTimer = Timer();
	self.actionMode = 0;	-- 0 = start, 1 = flying, 2 = grab terrain, 3 = grab MO
	self.climb = 0;
	self.canRelease = false;

	self.tapTimer = Timer();
	self.tapCounter = 0;
	self.didTap = false;
	self.canTap = false;

	self.fireVel = 40;	-- This immediately overwrites the .ini FireVel
	self.maxLineLength = 500;
	self.setLineLength = 0;
	self.lineStrength = 40;	-- How much "force" the rope can take before breaking

	self.limitReached = false;
	self.stretchMode = false;	-- Alternative elastic pull mode a là Liero
	self.stretchPullRatio = 0.1;
	self.pieSelection = 0;	-- 0 is nothing, 1 is full retract, 2 is partial retract, 3 is partial extend, 4 is full extend

	self.climbDelay = 10;	-- MS time delay between "climbs" to keep the speed consistant
	self.tapTime = 150;	-- Maximum amount of time between tapping for claw to return
	self.tapAmount = 2;	-- How many times to tap to bring back rope
	self.mouseClimbLength = 250;	-- How long to climb per mouse wheel for mouse users
	self.climbInterval = 3.5;	-- How many pixels the rope retracts / extends at a time
	self.autoClimbIntervalA = 4.0;	-- How many pixels the rope retracts / extends at a time when auto-climbing (fast)
	self.autoClimbIntervalB = 2.0;	-- How many pixels the rope retracts / extends at a time when auto-climbing (slow)

	self.stickSound = CreateSoundContainer("Grapple Gun Claw Stick", "Base.rte");
	self.clickSound = CreateSoundContainer("Grapple Gun Click", "Base.rte");
	self.returnSound = CreateSoundContainer("Grapple Gun Return", "Base.rte");

	--TODO: Rewrite this junk
	for gun in MovableMan:GetMOsInRadius(self.Pos, 50) do
		if gun and gun.ClassName == "HDFirearm" and gun.PresetName == "Grapple Gun" and SceneMan:ShortestDistance(self.Pos, ToHDFirearm(gun).MuzzlePos, self.mapWrapsX):MagnitudeIsLessThan(5) then
			self.parentGun = ToHDFirearm(gun);
			self.parent = MovableMan:GetMOFromID(gun.RootID);
			if MovableMan:IsActor(self.parent) then
				self.parent = ToActor(self.parent);
				if IsAHuman(self.parent) then
					self.parent = ToAHuman(self.parent);
				elseif IsACrab(self.parent) then
					self.parent = ToACrab(self.parent);
				end

				self.Vel = (self.parent.Vel * 0.5) + Vector(self.fireVel, 0):RadRotate(self.parent:GetAimAngle(true));
				self.parentGun:RemoveNumberValue("GrappleMode");
				for part in self.parent.Attachables do
					local radcheck = SceneMan:ShortestDistance(self.parent.Pos, part.Pos, self.mapWrapsX).Magnitude + part.Radius;
					if self.parentRadius == nil or radcheck > self.parentRadius then
						self.parentRadius = radcheck;
					end
				end

				self.actionMode = 1;
			end
			break;
		end
	end

	if self.parentGun == nil then	-- Failed to find our gun, abort
		self.ToDelete = true;
	end
end
function Update(self)
	if self.parent and IsMOSRotating(self.parent) and self.parent:HasObject("Grapple Gun") then
		local controller;
		local startPos = self.parent.Pos;

		self.ToDelete = false;
		self.ToSettle = false;

		self.lineVec = SceneMan:ShortestDistance(self.parent.Pos, self.Pos, self.mapWrapsX);
		self.lineLength = self.lineVec.Magnitude;

		if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
			self.parent = ToMOSRotating(MovableMan:GetMOFromID(self.parentGun.RootID));

			if self.parentGun.Magazine then
				self.parentGun.Magazine.Scale = 0;
			end
			startPos = self.parentGun.Pos;
			local flipAng = self.parent.HFlipped and 3.14 or 0;
			self.parentGun.RotAngle = self.lineVec.AbsRadAngle + flipAng;

			local mode = self.parentGun:GetNumberValue("GrappleMode");

			if mode ~= 0 then
				self.pieSelection = mode;
				self.parentGun:RemoveNumberValue("GrappleMode");
			end

			if self.parentGun.FiredFrame then
				if self.actionMode == 1 then
					self.ToDelete = true;
				else
					self.canRelease = true;
				end
			end

			if self.parentGun.FiredFrame and self.canRelease and (Vector(self.parentGun.Vel.X, self.parentGun.Vel.Y) ~= Vector(0, -1) or self.parentGun:IsActivated()) then
				self.ToDelete = true;
			end
		end

		if IsAHuman(self.parent) then
			self.parent = ToAHuman(self.parent);
			-- We now have a user that controls this grapple
			controller = self.parent:GetController();
			-- Point the gun towards the hook if our user is holding it
			if (self.parentGun and self.parentGun.ID ~= rte.NoMOID) and (self.parentGun:GetRootParent().ID == self.parent.ID) then
				if self.parent:IsPlayerControlled() then
					if controller:IsState(Controller.WEAPON_RELOAD) then
						self.ToDelete = true;
					end
					if self.parentGun.Magazine then
						self.parentGun.Magazine.RoundCount = 0;
					end
				end
				local offset = Vector(self.lineLength, 0):RadRotate(self.parent.FlipFactor * (self.lineVec.AbsRadAngle - self.parent:GetAimAngle(true)));
				self.parentGun.StanceOffset = offset;
				if self.parent.EquippedItem and self.parent.EquippedItem.ID == self.parentGun.ID and (self.parent.Vel:MagnitudeIsLessThan(5) and controller:IsState(Controller.AIM_SHARP)) then
					self.parentGun.RotAngle = self.parent:GetAimAngle(false) * self.parentGun.FlipFactor;
					startPos = self.parent.Pos;
				else
					self.parentGun.SharpStanceOffset = offset;
				end
			end
			-- Prevent the user from spinning like crazy
			if self.parent.Status > Actor.STABLE then
				self.parent.AngularVel = self.parent.AngularVel/(1 + math.abs(self.parent.AngularVel) * 0.01);
			end
		else	-- If the gun is by itself, hide the HUD
			self.parentGun.HUDVisible = false;
		end
		-- Add sound when extending / retracting
		if MovableMan:IsParticle(self.crankSound) then
			self.crankSound.PinStrength = 1000;
			self.crankSound.ToDelete = false;
			self.crankSound.ToSettle = false;
			self.crankSound.Pos = startPos;
			if self.lastSetLineLength ~= self.setLineLength then
				self.crankSound:EnableEmission(true);
			else
				self.crankSound:EnableEmission(false);
			end
		else
			self.crankSound = CreateAEmitter("Grapple Gun Sound Crank");
			self.crankSound.Pos = startPos;
			MovableMan:AddParticle(self.crankSound);
		end

		self.lastSetLineLength = self.setLineLength;

		if self.actionMode == 1 then	-- Hook is in flight
			self.rayVec = Vector();
			-- Stretch mode: gradually retract the hook for a return hit
			if self.stretchMode then
				self.Vel = self.Vel - Vector(self.lineVec.X, self.lineVec.Y):SetMagnitude(math.sqrt(self.lineLength) * self.stretchPullRatio/2);
			end
			local length = math.sqrt(self.Diameter + self.Vel.Magnitude);
			-- Detect terrain and stick if found
			local ray = Vector(length, 0):RadRotate(self.Vel.AbsRadAngle);
			if SceneMan:CastStrengthRay(self.Pos, ray, 0, self.rayVec, 0, rte.airID, self.mapWrapsX) then
				self.actionMode = 2;
			else	-- Detect MOs and stick if found
				local moRay = SceneMan:CastMORay(self.Pos, ray, self.parent.ID, -2, rte.airID, false, 0);
				if moRay ~= rte.NoMOID then
					self.target = MovableMan:GetMOFromID(moRay);
					-- Treat pinned MOs as terrain
					if self.target.PinStrength > 0 then
						self.actionMode = 2;
					else
						self.stickPosition = SceneMan:ShortestDistance(self.target.Pos, self.Pos, self.mapWrapsX);
						self.stickRotation = self.target.RotAngle;
						self.stickDirection = self.RotAngle;
						self.actionMode = 3;
					end
					-- Inflict damage
					local part = CreateMOPixel("Grapple Gun Damage Particle");
					part.Pos = self.Pos;
					part.Vel = SceneMan:ShortestDistance(self.Pos, self.target.Pos, self.mapWrapsX):SetMagnitude(self.Vel.Magnitude);
					MovableMan:AddParticle(part);
				end
			end

			if self.actionMode > 1 then
				self.stickSound:Play(self.Pos);
				self.setLineLength = math.floor(self.lineLength);
				self.Vel = Vector();
				self.PinStrength = 1000;
				self.Frame = 1;
			end

			if self.lineLength > self.maxLineLength then
				if self.limitReached == false then
					self.limitReached = true;
					self.clickSound:Play(startPos);
				end
				local movetopos = self.parent.Pos + (self.lineVec):SetMagnitude(self.maxLineLength);
				if self.mapWrapsX == true then
					if movetopos.X > SceneMan.SceneWidth then
						movetopos = Vector(movetopos.X - SceneMan.SceneWidth, movetopos.Y);
					elseif movetopos.X < 0 then
						movetopos = Vector(SceneMan.SceneWidth + movetopos.X, movetopos.Y);
					end
				end
				self.Pos = movetopos;

				local pullamountnumber = math.abs(-self.lineVec.AbsRadAngle + self.Vel.AbsRadAngle)/6.28;
				self.Vel = self.Vel - self.lineVec:SetMagnitude(self.Vel.Magnitude * pullamountnumber);
			end
		elseif self.actionMode > 1 then	-- Hook has stuck
			-- Actor mass and velocity affect pull strength negatively, rope length affects positively (diminishes the former)
			local parentForces = 1 + (self.parent.Vel.Magnitude * 10 + self.parent.Mass)/(1 + self.lineLength);
			local terrVector = Vector();
			-- Check if there is terrain between the hook and the user
			if self.parentRadius ~= nil then
				self.terrcheck = SceneMan:CastStrengthRay(self.parent.Pos, self.lineVec:SetMagnitude(self.parentRadius), 0, terrVector, 2, rte.airID, self.mapWrapsX);
			else
				self.terrcheck = false;
			end

			-- Control automatic extension and retraction
			if self.pieSelection ~= 0 and self.climbTimer:IsPastSimMS(self.climbDelay) then
				self.climbTimer:Reset();

				if self.pieSelection == 1 then

					if self.setLineLength > self.autoClimbIntervalA and self.terrcheck == false then
						self.setLineLength = self.setLineLength - (self.autoClimbIntervalA/parentForces);
					else
						self.pieSelection = 0;
					end
				elseif self.pieSelection == 2 then
					if self.setLineLength < (self.maxLineLength - self.autoClimbIntervalB) then
						self.setLineLength = self.setLineLength + self.autoClimbIntervalB;
					else
						self.pieSelection = 0;
					end
				end
			end

			-- Control the rope if the user is holding the gun
			if self.parentGun and self.parentGun.ID ~= rte.NoMOID and controller then
				-- These forces are to help the user nudge across obstructing terrain
				local nudge = math.sqrt(self.lineVec.Magnitude + self.parent.Radius)/(10 + self.parent.Vel.Magnitude);
				-- Retract automatically by holding fire or control the rope through the pie menu
				if self.parentGun:IsActivated() and self.climbTimer:IsPastSimMS(self.climbDelay) then
					self.climbTimer:Reset();
					if self.pieSelection == 0 and self.parentGun:IsActivated() then

						if self.setLineLength > self.autoClimbIntervalA and self.terrcheck == false then
							self.setLineLength = self.setLineLength - (self.autoClimbIntervalA/parentForces);
						else
							self.parentGun:RemoveNumberValue("GrappleMode");
							self.pieSelection = 0;
							if self.terrcheck ~= false then
								-- Try to nudge past terrain
								local aimvec = Vector(self.lineVec.Magnitude, 0):SetMagnitude(nudge):RadRotate((self.lineVec.AbsRadAngle + self.parent:GetAimAngle(true))/2 + self.parent.FlipFactor * 0.7);
								self.parent.Vel = self.parent.Vel + aimvec;
							end
						end
					elseif self.pieSelection == 2 then
						if self.setLineLength < (self.maxLineLength - self.autoClimbIntervalB) then
							self.setLineLength = self.setLineLength + self.autoClimbIntervalB;
						else
							self.parentGun:RemoveNumberValue("GrappleMode");
							self.pieSelection = 0;
						end
					end
				end

				-- Hold crouch to control rope manually
				if controller:IsState(Controller.BODY_CROUCH) then
					if self.climb == 1 or self.climb == 2 then
						if self.climbTimer:IsPastSimMS(self.climbDelay) then
							self.climbTimer:Reset();
							if self.pieSelection == 0 then
								if self.climb == 1 then
									self.setLineLength = self.setLineLength - (self.climbInterval/parentForces);
								elseif self.climb == 2 then
									self.setLineLength = self.setLineLength + self.climbInterval;
								end
							end

							self.climb = 0;
						end
					elseif self.climb == 3 or self.climb == 4 then
						if self.climbTimer:IsPastSimMS(self.mouseClimbLength) then
							self.climbTimer:Reset();
							self.mouseClimbTimer:Reset();
							self.climb = 0;
						else
							if self.mouseClimbTimer:IsPastSimMS(self.climbDelay) then
								self.mouseClimbTimer:Reset();
								if self.climb == 3 then
									if (self.setLineLength-self.climbInterval) >= 0 and self.terrcheck == false then
										self.setLineLength = self.setLineLength - (self.climbInterval/parentForces);

									elseif self.terrcheck ~= false then
										-- Try to nudge past terrain
										local aimvec = Vector(self.lineVec.Magnitude, 0):SetMagnitude(nudge):RadRotate((self.lineVec.AbsRadAngle + self.parent:GetAimAngle(true))/2 + self.parent.FlipFactor * 0.7);
										self.parent.Vel = self.parent.Vel + aimvec;
									end
								elseif self.climb == 4 then
									if (self.setLineLength+self.climbInterval) <= self.maxLineLength then
										self.setLineLength = self.setLineLength + self.climbInterval;
									end
								end
							end
						end
					end

					if controller:IsMouseControlled() then
						controller:SetState(Controller.WEAPON_CHANGE_NEXT, false);
						controller:SetState(Controller.WEAPON_CHANGE_PREV, false);
						if controller:IsState(Controller.SCROLL_UP) then
							self.climbTimer:Reset();
							self.climb = 3;
						end

						if controller:IsState(Controller.SCROLL_DOWN) then
							self.climbTimer:Reset();
							self.climb = 4;
						end
					elseif controller:IsMouseControlled() == false then
						if controller:IsState(Controller.HOLD_UP) then
							if self.setLineLength > self.climbInterval and self.terrcheck == false then
								self.climb = 1;
							elseif self.terrcheck ~= false then
								-- Try to nudge past terrain
								local aimvec = Vector(self.lineVec.Magnitude, 0):SetMagnitude(nudge):RadRotate((self.lineVec.AbsRadAngle + self.parent:GetAimAngle(true))/2 + self.parent.FlipFactor * 0.7);
								self.parent.Vel = self.parent.Vel + aimvec;
							end
						end

						if controller:IsState(Controller.HOLD_DOWN) and self.setLineLength < (self.maxLineLength-self.climbInterval) then
							self.climb = 2;
						end
					end
					controller:SetState(Controller.AIM_UP, false);
					controller:SetState(Controller.AIM_DOWN, false);
				end
			end

			if self.actionMode == 2 then	-- Stuck terrain
				if self.stretchMode then
					local pullVec = self.lineVec:SetMagnitude(0.15 * math.sqrt(self.lineLength)/parentForces);
					self.parent.Vel = self.parent.Vel + pullVec;
				elseif self.lineLength > self.setLineLength then
					local hookVel = SceneMan:ShortestDistance(Vector(self.PrevPos.X, self.PrevPos.Y), Vector(self.Pos.X, self.Pos.Y), self.mapWrapsX);

					local pullAmountNumber = self.lineVec.AbsRadAngle - self.parent.Vel.AbsRadAngle;
					if pullAmountNumber < 0 then
						pullAmountNumber = pullAmountNumber * -1;
					end

					pullAmountNumber = pullAmountNumber/6.28;
					self.parent:AddAbsForce(self.lineVec:SetMagnitude(((self.lineLength - self.setLineLength)^3) * pullAmountNumber) + hookVel:SetMagnitude(math.pow(self.lineLength - self.setLineLength, 2) * 0.8), self.parent.Pos);

					local moveToPos = self.Pos + (self.lineVec * -1):SetMagnitude(self.setLineLength);
					if self.mapWrapsX == true then
						if moveToPos.X > SceneMan.SceneWidth then
							moveToPos = Vector(moveToPos.X - SceneMan.SceneWidth, moveToPos.Y);
						elseif moveToPos.X < 0 then
							moveToPos = Vector(SceneMan.SceneWidth + moveToPos.X, moveToPos.Y);
						end
					end

					self.parent.Pos = moveToPos;

					local pullAmountNumber = math.abs(self.lineVec.AbsRadAngle - self.parent.Vel.AbsRadAngle)/6.28;
					-- Break the rope if the forces are too high
					if (self.parent.Vel - self.lineVec:SetMagnitude(self.parent.Vel.Magnitude * pullAmountNumber)):MagnitudeIsGreaterThan(self.lineStrength) then
						self.ToDelete = true;
					end

					self.parent.Vel = self.parent.Vel + self.lineVec;
				end

			elseif self.actionMode == 3 then	-- Stuck MO
				if self.target.ID ~= rte.NoMOID then
					self.Pos = self.target.Pos + Vector(self.stickPosition.X, self.stickPosition.Y):RadRotate(self.target.RotAngle - self.stickRotation);
					self.RotAngle = self.stickDirection + (self.target.RotAngle - self.stickRotation);

					local jointStiffness;
					local target = self.target;
					if target.ID ~= target.RootID then
						local mo = target:GetRootParent();
						if mo.ID ~= rte.NoMOID and IsAttachable(target) then
							-- It's best to apply all the forces to the parent instead of utilizing JointStiffness
							target = mo;
						end
					end

					if self.stretchMode then
						local pullVec = self.lineVec:SetMagnitude(self.stretchPullRatio * math.sqrt(self.lineLength)/parentForces);
						self.parent.Vel = self.parent.Vel + pullVec;

						local targetForces = 1 + (target.Vel.Magnitude * 10 + target.Mass)/(1 + self.lineLength);
						target.Vel = target.Vel - (pullVec) * parentForces/targetForces;
					elseif self.lineLength > self.setLineLength then
						-- Take wrapping to account, treat all distances relative to hook
						local parentPos = target.Pos + SceneMan:ShortestDistance(target.Pos, self.parent.Pos, self.mapWrapsX);
						-- Add forces to both user and the target MO
						local hookVel = SceneMan:ShortestDistance(Vector(self.PrevPos.X, self.PrevPos.Y), Vector(self.Pos.X, self.Pos.Y), self.mapWrapsX);

						local pullAmountNumber = self.lineVec.AbsRadAngle - self.parent.Vel.AbsRadAngle;
						if pullAmountNumber < 0 then
							pullAmountNumber = pullAmountNumber * -1;
						end
						pullAmountNumber = pullAmountNumber/6.28;
						self.parent:AddAbsForce(self.lineVec:SetMagnitude(((self.lineLength - self.setLineLength)^3) * pullAmountNumber) + hookVel:SetMagnitude(math.pow(self.lineLength - self.setLineLength, 2) * 0.8), self.parent.Pos);

						pullAmountNumber = (self.lineVec * -1).AbsRadAngle - (hookVel).AbsRadAngle;
						if pullAmountNumber < 0 then
							pullAmountNumber = pullAmountNumber * -1;
						end
						pullAmountNumber = pullAmountNumber/6.28;
						local targetforce = ((self.lineVec * -1):SetMagnitude(((self.lineLength - self.setLineLength)^3) * pullAmountNumber) + (self.lineVec * -1):SetMagnitude(math.pow(self.lineLength - self.setLineLength, 2) * 0.8));

						target:AddAbsForce(targetforce, self.Pos);--target.Pos + SceneMan:ShortestDistance(target.Pos, self.Pos, self.mapWrapsX));
						target.AngularVel = target.AngularVel * 0.99;
					end
				else	-- Our MO has been destroyed, return hook
					self.ToDelete = true;
				end
			end
		end

		-- Double tapping crouch retrieves the hook
		if controller and controller:IsState(Controller.BODY_CROUCH) then
			self.pieSelection = 0;
			if self.canTap == true then
				controller:SetState(Controller.BODY_CROUCH, false);
				self.climb = 0;
				if self.parentGun ~= nil and self.parentGun.ID ~= rte.NoMOID then
					self.parentGun:RemoveNumberValue("GrappleMode");
				end

				self.tapTimer:Reset();
				self.didTap = true;
				self.canTap = false;
				self.tapCounter = self.tapCounter + 1;
			end
		else
			self.canTap = true;
		end

		if self.tapTimer:IsPastSimMS(self.tapTime) then
			self.tapCounter = 0;
		else
			if self.tapCounter >= self.tapAmount then
				self.ToDelete = true;
			end
		end

		-- Fine tuning: take the seam into account when drawing the rope
		local drawPos = self.parent.Pos + self.lineVec:SetMagnitude(self.lineLength);
		if self.ToDelete == true then
			drawPos = self.parent.Pos + (self.lineVec * 0.5);
			if self.parentGun and self.parentGun.Magazine then
				-- Show the magazine as if the hook is being retracted
				self.parentGun.Magazine.Pos = drawPos;
				self.parentGun.Magazine.Scale = 1;
				self.parentGun.Magazine.Frame = 0;
			end
			self.returnSound:Play(drawPos);
		end
		
		PrimitiveMan:DrawLinePrimitive(startPos, drawPos, 249);
	elseif self.parentGun and IsHDFirearm(self.parentGun) then
		self.parent = self.parentGun;
	else
		self.ToDelete = true;
	end
end
function Destroy(self)
	if MovableMan:IsParticle(self.crankSound) then
		self.crankSound.ToDelete = true;
	end
	if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
		self.parentGun.HUDVisible = true;
		self.parentGun:RemoveNumberValue("GrappleMode");
	end
end