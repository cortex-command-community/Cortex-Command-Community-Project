function Create(self)
	--self.parentGunlastPos = self.Pos
	self.lastPos = self.Pos

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
	self.cablespring = 0.15; --0.3;
	self.ropemod = 17

	self.currentLineLength = 0;
	self.longestLineLength = 0;
	
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

	--DRAW CABLE LINES
	function self.drawline(a,b)
		local vect1 = Vector(self.apx[a],self.apy[a])
		local vect2 =Vector(self.apx[b],self.apy[b])
		PrimitiveMan:DrawLinePrimitive(vect1, vect2, 155) --165)
	end

	--VERLET COLLISIONS
	function self.verletcollide(h,nextX,nextY)
		local velX = nextX;
		local velY = nextY;
		--APPLY FRICTION TO INDIVIDUAL JOINTS
		local ray = Vector(velX,velY)
		local startpos = Vector(self.apx[h],self.apy[h])
		local rayvec = Vector();
		local rayvec2 = Vector()

		local rayl = SceneMan:CastObstacleRay(startpos,ray,rayvec,rayvec2,self.parent.ID,self.Team,rte.airID,0)
		if rayl >= 0 then
			--local hitpos = SceneMan:GetLastRayHitPos();
			local ud = SceneMan:ShortestDistance(rayvec, startpos, true)
			local angle2 = ud.AbsRadAngle
			local usemag = math.min(ray.Magnitude*1.1,1)
			local changevect = Vector(usemag,0):RadRotate(angle2)
			self.apx[h] = self.apx[h] + changevect.X
			self.apy[h] = self.apy[h] + changevect.Y
			self.lastX[h] = rayvec.X
			self.lastY[h] = rayvec.Y
		else
			self.apx[h] = self.apx[h]+nextX
			self.apy[h] = self.apy[h]+nextY
		end

	end

	--ESTABLISH LINE
	self.apx = {};
	self.apy = {};

	self.lastX = {};
	self.lastY = {}

	self.usefriction = 0.99;

	local px = self.Pos.X
	local py = self.Pos.Y
	for i = 0, 11 do
		self.apx[i] = px
		self.apy[i] = py
		self.lastX[i] = px
		self.lastY[i] = py
	end

	self.lastX[11] = px - self.Vel.X
	self.lastY[11] = py - self.Vel.Y
	--slots 0 and 11 are ANCHOR POINTS
	
	for i = 1, MovableMan:GetMOIDCount() - 1 do
		local gun = MovableMan:GetMOFromID(i);
		if gun and gun.ClassName == "HDFirearm" and gun.PresetName == "Grapple Gun" and SceneMan:ShortestDistance(self.Pos, ToHDFirearm(gun).MuzzlePos, self.mapWrapsX).Magnitude < 5 then
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
	if self.parent and IsMOSRotating(self.parent) then
		local tautlen = 50
		local controller;
		local startPos = self.parent.Pos;
		
		self.ToDelete = false;
		self.ToSettle = false;

		self.linelength = 0
		
		local cablelength = self.currentLineLength*0.0908 --THIS IS BASICALLY /11 FOR NUMBER OF SEGMENTS

	--HANDLE LINE
	local i = 11
	--HANDLE ALL LINE JOINTS
		while i > -1 do
			if i == 0 or (i == 11 and self.limitReached == false and (self.actionMode == 1 or self.actionMode > 1)) then
				if i == 0 then --POINT 0: ANCHOR TO GUN
					local usepos = self.parent.Pos --self.parentGun.Pos
					self.apx[i] = usepos.X
					self.apy[i] = usepos.Y
					self.lastX[i] = self.lastPos.X --self.parentGunlastPos.X
					self.lastY[i] = self.lastPos.Y --self.parentGunlastPos.Y
				else --POINT 11: ANCHOR TO GRAPPLE if IN FLIGHT
					local usepos = self.Pos
					self.apx[i] = usepos.X
					self.apy[i] = usepos.Y
					self.lastX[i] = usepos.X
					self.lastY[i] = usepos.Y
				end

			else
				if not (i == 11 and self.actionMode == 2) then
					--CALCULATE BASIC PHYSICS
					local accX = 0;
					local accY = 0.05;
			
					local velX = self.apx[i] - self.lastX[i];
					local velY = self.apy[i] - self.lastY[i];
				
					local ufriction = self.usefriction
					if i == 11 then 
						ufriction = 0.99;
						accY = 0.5;
					end

					local nextX = (velX + accX) * ufriction ;
					local nextY = (velY + accY) * ufriction ;
						
					self.lastX[i] = self.apx[i]
					self.lastY[i] = self.apy[i]

					self.verletcollide(i,nextX,nextY)
				end
				if i == 11 and self.actionMode == 3 then
					if self.target.ID ~= rte.NoMOID then
						local target = self.target;
						if target.ID ~= target.RootID then
							local mo = target:GetRootParent();
							if mo.ID ~= rte.NoMOID and IsAttachable(target) then
								-- It's best to apply all the forces to the parent instead of utilizing JointStiffness
								target = mo;
							end
						end
	
						self.lastX[i] = self.apx[i]-target.Vel.X
						self.lastY[i] = self.apy[i]-target.Vel.Y
					else	-- Our MO has been destroyed, return hook
						self.ToDelete = true;
					end
				end
				--self.apx[i] = nextX
				--self.apy[i] = nextY
			end

			local j = 0
			local diffadd = 0
			
			while j < 3 do
				if i < 11 then
					--FROM i+1
					-- calculate the distance
					local diffX = self.apx[i+1] - self.apx[i]
					local diffY = self.apy[i+1] - self.apy[i]
					local d = Vector(diffX,diffY).Magnitude --math.sqrt(math.abs((diffX * diffX) + (diffY * diffY)) )
					local dmin = math.max(d,0.000001)

					-- difference scalar
					local difference = (cablelength - d) / dmin;
					diffadd = diffadd+math.max(d,0)

					-- translation for each PointMass. They'll be pushed 1/2 the required distance to match their resting distances.
					local usespring = self.cablespring; --0.25 --0.45 --0.7 --0.4 --0.05 + cablespring * 2

					local translateX = diffX * usespring * difference * self.usefriction;
					local translateY = diffY * usespring * difference * self.usefriction;


					local nextX = translateX
					local nextY = translateY
					self.verletcollide(i,nextX,nextY)

					local nextX = -translateX
					local nextY = -translateY
					self.verletcollide(i+1,nextX,nextY)


					if i == 10 and self.actionMode == 3 then
						if self.target.ID ~= rte.NoMOID then
							local target = self.target;
							if target.ID ~= target.RootID then
								local mo = target:GetRootParent();
								if mo.ID ~= rte.NoMOID and IsAttachable(target) then
									-- It's best to apply all the forces to the parent instead of utilizing JointStiffness
									target = mo;
								end
							end
							self.Pos = self.target.Pos + Vector(self.stickPosition.X, self.stickPosition.Y):RadRotate(self.target.RotAngle - self.stickRotation);
							self.RotAngle = self.stickDirection + (self.target.RotAngle - self.stickRotation);
		
							if diffadd > tautlen then
								target:AddAbsForce(Vector(translateX,translateY)*math.max(diffadd*self.cablespring,0)*0.125*self.parent.Mass*self.parent.Vel.Magnitude, target.Pos);
							end
						else	-- Our MO has been destroyed, return hook
							self.ToDelete = true;
						end
					end



					--TO i+1
					-- calculate the distance
					local diffX = self.apx[i] - self.apx[i+1]
					local diffY = self.apy[i] -self. apy[i+1]
					local d = math.sqrt(math.abs((diffX * diffX) + (diffY * diffY)) )
					local dmin = math.max(d,0.000001)

					-- difference scalar
					local difference = (cablelength - d) / dmin;
					local usespring = self.cablespring; --0.25 --0.5 --0.85 -- 0.45 --0.05 + cablespring * 2

					local translateX = diffX * usespring * difference * self.usefriction;
					local translateY = diffY * usespring * difference * self.usefriction;


					local nextX = translateX
					local nextY = translateY
					self.verletcollide(i,nextX,nextY)


					local nextX = -translateX
					local nextY = -translateY
					self.verletcollide(i+1,nextX,nextY)

					--print(diffadd)
					if diffadd > tautlen and self.parent and i == 0 and self.actionMode > 1 then --CONTACT WITH TARGET: EXERT FORCE ON GUN
						--local ropelen = SceneMan:ShortestDistance(Vector(self.apx[i],self.apy[i]), Vector(self.apx[i+1],self.apy[i+1]), self.mapWrapsX).Magnitude;

						if self.actionMode == 3 and self.target.ID ~= rte.NoMOID then
							local target = self.target;
	
							self.parent:AddAbsForce(Vector(translateX,translateY)*math.max(diffadd*self.cablespring,0)*0.1*target.Mass*target.Vel.Magnitude, self.parent.Pos);
						else
							self.parent:AddAbsForce(Vector(translateX,translateY)*math.max(diffadd*self.cablespring,0)*0.1*self.parent.Mass*self.parent.Vel.Magnitude, self.parent.Pos);
						end
					end
			
				end
					j=j+1
			end

			--CONSTRAINTS
			local j = 0
			while j < 3 do
				if not i == 11 then
					local diffX = self.apx[i] - self.apx[i+1]
					local diffY = self.apy[i] - self.apy[i+1]
		
					local diffmag = Vector(diffX,diffY).Magnitude
					local diffFactor = (cablelength - diffmag) / diffmag * 0.5;
					local offset = Vector(diffX * diffFactor, diffY * diffFactor);
					
					--self.apx[i] = self.apx[i] + offset.X;
					--self.apy[i] = self.apy[i] + offset.Y;
					--self.apx[i+1] = self.apx[i+1] - offset.X;
					--self.apy[i+1] = self.apy[i+1] - offset.Y;

					self.verletcollide(i,offset.X,offset.Y)
					self.verletcollide(i+1,-offset.X,-offsest.Y)

					--self.lastX[i]=self.lastX[i] + offset.X;
					--self.lastY[i]=self.lastY[i] + offset.Y;
					--self.lastX[i+1]=self.lastX[i+1] - offset.X;
					--self.lastY[i_1]=self.lastY[i+1] - offset.Y;
		
					if self.parent then

						if diffadd > tautlen and self.parent and i == 0 and self.actionMode > 1 then --CONTACT WITH TARGET: EXERT FORCE ON GUN
							local ropelen = SceneMan:ShortestDistance(Vector(self.apx[i],self.apy[i]), Vector(self.apx[i+1],self.apy[i+1]), self.mapWrapsX).Magnitude;
							if ropelen >= tautlen then
								if self.target then
									self.parent:AddAbsForce(Vector(offset.X,offset.Y)*math.max(diffadd*self.cablespring,0)*0.1*target.Mass*target.Vel.Magnitude, self.parent.Pos);
								else
									self.parent:AddAbsForce(Vector(offset.X,offset.Y)*math.max(diffadd*self.cablespring,0)*0.1*self.parent.Mass*self.parent.Vel.Magnitude, self.parent.Pos);
								end
							end
						end

						if diffadd > tautlen and self.target and i == 10 and self.actionMode > 1 then --CONTACT WITH TARGET: EXERT FORCE ON GTARGET

							local ropelen = SceneMan:ShortestDistance(Vector(self.apx[i+1],self.apy[i+1]), Vector(self.apx[i],self.apy[i]), self.mapWrapsX).Magnitude;
							if ropelen >= tautlen then
								target:AddAbsForce(Vector(offset.X,offset.Y)*math.max(diffadd*self.cablespring,0)*0.1*self.parent.Mass*self.parent.Vel.Magnitude, target.Pos);
							end
						end
					end
				end
				j=j+1
			end
				i = i-1
		end

		--DRAW LINE
		--TRI 1
		self.drawline(0,1)
		--TRI 2
		self.drawline(1,2)
		--TRI 3
		self.drawline(2,3)
		--TRI 4
		self.drawline(3,4)
		--TRI 4
		self.drawline(4,5)
		--TRI 5
		self.drawline(5,6)
		--TRI 6
		self.drawline(6,7)
		--TRI 7
		self.drawline(7,8)
		--TRI 8
		self.drawline(8,9)
		--TRI 9
		self.drawline(9,10)
		--TRI 10
		self.drawline(10,11)

		--print(self.currentLineLength)
		--self.Pos = Vector(self.apx[11],self.apy[11])

		--DEFINE PRIMARY PLAYER MOVEMENT CONTROL
		self.lineVec = SceneMan:ShortestDistance(self.parent.Pos, self.Pos, self.mapWrapsX);
		self.lineLength = self.lineVec.Magnitude

		 local targetpos = Vector(self.apx[1],self.apy[1])
		 self.linetargetVec = SceneMan:ShortestDistance(self.parent.Pos, targetpos, self.mapWrapsX);
		-- if self.lineLength > self.currentLineLength then self.currentLineLength = self.lineLength end --EXTEND LINE LENGTH
		
		--REACH LONGEST LINE LENGTH
		if self.currentLineLength > self.maxLineLength then
			self.currentLineLength = self.currentLineLength - 1 --TIGHTEN ROPE
			if self.limitReached == false then
				self.limitReached = true;
				self.clickSound:Play(startPos);
			end
		end
		
		--HAVE AN OPERATOR?
		if self.parentGun and self.parentGun.ID ~= rte.NoMOID then
			self.parent = ToMOSRotating(MovableMan:GetMOFromID(self.parentGun.RootID));
			if self.parentGun.Magazine then
				self.parentGun.Magazine.Scale = 0;
			end
			startPos = self.parentGun.Pos;
			local flipAng = self.parent.HFlipped and 3.14 or 0;
			self.parentGun.RotAngle = self.linetargetVec.AbsRadAngle + flipAng;
			
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

		--MAINTAIN OPERATOR CONTROL
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
				if self.parent.EquippedItem and self.parent.EquippedItem.ID == self.parentGun.ID and (self.parent.Vel.Magnitude < 5 and controller:IsState(Controller.AIM_SHARP)) then
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


		--MAINTAIN EXTEND/RETRACT SOUNDS
		if MovableMan:IsParticle(self.crankSound) then
			self.crankSound.PinStrength = 1000;
			self.crankSound.ToDelete = false;
			self.crankSound.ToSettle = false;
			self.crankSound.Pos = startPos;
			if self.lastSetLineLength ~= self.currentlinelength then
				self.crankSound:EnableEmission(true);
			else
				self.crankSound:EnableEmission(false);
			end
		else
			self.crankSound = CreateAEmitter("Grapple Gun Sound Crank");
			self.crankSound.Pos = startPos;
			MovableMan:AddParticle(self.crankSound);
		end

	--BEGIN HOOK/SELF CONTROL
		self.lastSetLineLength = self.currentlinelength;

		--CONTROL for IN FLIGHT
		if self.actionMode == 1 then
			if self.limitReached == true then
				self.Pos.X = self.apx[11]
				self.Pos.Y = self.apy[11] 
			end
			--EXTEND LINE LENGTH
			if self.lineLength > self.currentLineLength and self.limitReached == false then 
				self.currentLineLength = self.lineLength 
			end 

			self.rayVec = Vector();

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
				--self.currentlinelength = math.floor(self.lineLength);
				self.Vel = Vector();
				self.PinStrength = 1000;
				self.Frame = 1;
			end



		--CONTROL for STUCK in SOMETHING
		elseif self.actionMode > 1 then

			self.apx[11] = self.Pos.X
			self.apy[11] = self.Pos.Y


			-- Control automatic extension and retraction
			if self.pieSelection ~= 0 and self.climbTimer:IsPastSimMS(self.climbDelay) then
				self.climbTimer:Reset();

				if self.pieSelection == 1 then
				
					if self.currentLineLength > 1 then --and self.terrcheck == false then
						self.currentLineLength=self.currentLineLength-1
					else
						self.pieSelection = 0;
					end
				elseif self.pieSelection == 2 then
					if self.currentLineLength < (self.maxLineLength - self.autoClimbIntervalB) then
						--self.setLineLength = self.setLineLength + self.autoClimbIntervalB;
						self.currentLineLength=self.currentLineLength + self.autoClimbIntervalB
					else
						self.pieSelection = 0;
					end
				end
			end

			-- Control the rope if the user is holding the gun
			if self.parentGun and self.parentGun.ID ~= rte.NoMOID and controller then

				-- Retract automatically by holding fire or control the rope through the pie menu
				if self.parentGun:IsActivated() and self.climbTimer:IsPastSimMS(self.climbDelay) then
					self.climbTimer:Reset();
					if self.pieSelection == 0 and self.parentGun:IsActivated() then

						if self.currentLineLength > 1 then --and self.terrcheck == false then
							--self.setLineLength = self.setLineLength - (self.autoClimbIntervalA/parentForces);
							self.currentLineLength=self.currentLineLength-1
						else
							self.parentGun:RemoveNumberValue("GrappleMode");
							self.pieSelection = 0;
		
						end
					elseif self.pieSelection == 2 then
						if self.currentlinelength < (self.maxLineLength - self.autoClimbIntervalB) then
							--self.setLineLength = self.setLineLength + self.autoClimbIntervalB;
							self.currentLineLength=self.currentLineLength+1
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
								if self.climb == 1 and self.currentLineLength > 1 then
									self.currentLineLength=self.currentLineLength-1
								elseif self.climb == 2 and self.currentLineLength < self.maxLineLength then
									self.currentLineLength=self.currentLineLength+1
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
								if self.climb == 3 and self.currentLineLength > 1 then
		
										--self.setLineLength = self.setLineLength - (self.climbInterval/parentForces);
										self.currentLineLength=self.currentLineLength-1
		
								elseif self.climb == 4 and self.currentLineLength < self.maxLineLength  then
									-- if (self.setLineLength+self.climbInterval) <= self.maxLineLength then
									-- 	self.setLineLength = self.setLineLength + self.climbInterval;
									self.currentLineLength = self.currentLineLength+1
									-- end
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
							self.climb = 1;

						end
						if controller:IsState(Controller.HOLD_DOWN) and self.setLineLength < (self.maxLineLength-self.climbInterval) then
							self.climb = 2;
						end
					end
					controller:SetState(Controller.AIM_UP, false);
					controller:SetState(Controller.AIM_DOWN, false);
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

	elseif self.parentGun and IsHDFirearm(self.parentGun) then
		self.parent = self.parentGun;
	else
		self.ToDelete = true;
	end

	if self.parentGun then
		--self.parentGunlastPos = self.parentGun.Pos
		self.lastPos = self.parent.Pos
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