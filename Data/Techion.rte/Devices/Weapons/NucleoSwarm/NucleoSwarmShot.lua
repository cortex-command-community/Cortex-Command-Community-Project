function Create(self)
	self.speed = 10;
	self.adjustmentAmount = 70;
	self.targetingAdjustmentAmount = 70;

	local raylength = 800;
	local rayPixSpace = 5;

	local dots = math.floor(raylength/rayPixSpace);

	for i = 1, dots do
		local checkPos = self.Pos + Vector(self.Vel.X, self.Vel.Y):SetMagnitude((i/dots) * raylength);
		if SceneMan.SceneWrapsX == true then
			if checkPos.X > SceneMan.SceneWidth then
				checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
			elseif checkPos.X < 0 then
				checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
			end
		end
		if SceneMan:GetTerrMatter(checkPos.X, checkPos.Y) == rte.airID then
			local moCheck = SceneMan:GetMOIDPixel(checkPos.X, checkPos.Y);
			if moCheck ~= rte.NoMOID then
				local actor = MovableMan:GetMOFromID(MovableMan:GetMOFromID(moCheck).RootID);
				if actor and actor.Team ~= self.Team then
					self.target = actor;
					break;
				end
			end
		else
			self.tPos = checkPos;
			break;
		end
		if i == dots then
			self.tPos = checkPos;
		end
	end

	self.Vel = Vector(self.Vel.X, self.Vel.Y):DegRotate(math.random(-45, 45));

	self.seekerTimer = Timer();
	self.moveTimer = Timer();
	self.lifeTimer = Timer();

	self.seekerDelay = 1000 - math.random(1000);

	self.disintegrationStrength = 300;
end

function Update(self)
	local useAdjust = self.adjustmentAmount;

	if self.target ~= nil and self.target.ID ~= rte.NoMOID then
		self.tPos = Vector(self.target.Pos.X, self.target.Pos.Y);
		useAdjust = self.targetingAdjustmentAmount;
		self.seekerDelay = 0;
	else
		self.target = nil;
	end

	if self.tPos ~= nil then
		local checkDirB = SceneMan:ShortestDistance(self.Pos, self.tPos, SceneMan.SceneWrapsX);
		local checkDir = checkDirB:SetMagnitude(useAdjust * (self.moveTimer.ElapsedSimTimeMS/1000));
		self.Vel = Vector(self.Vel.X + checkDir.X, self.Vel.Y + checkDir.Y):SetMagnitude(self.speed);

		if self.seekerTimer:IsPastSimMS(self.seekerDelay) and self.target == nil then
			self.seekerTimer:Reset();
			self.seekerDelay = 1000 - math.random(1000);
			for actor in MovableMan.Actors do
				if actor.Team ~= self.Team then
					self.potentialtargetdist = SceneMan:ShortestDistance(self.Pos, actor.Pos, SceneMan.SceneWrapsX);
					if (self.lastdist == nil or (self.lastdist ~= nil and self.potentialtargetdist:MagnitudeIsLessThan(self.lastdist))) and not self.potentialtargetdist:MagnitudeIsGreaterThan(500) and SceneMan:CastStrengthRay(self.Pos, self.potentialtargetdist:SetMagnitude(self.potentialtargetdist.Magnitude - actor.Radius), 0, Vector(), 5, rte.airID, SceneMan.SceneWrapsX) == false then
						self.lastdist = self.potentialtargetdist.Magnitude;
						self.target = actor;
						self.lasttargetpos = Vector(self.target.Pos.X, self.target.Pos.Y);
					end
				end
			end
		end
	end

	if self.lifeTimer:IsPastSimMS(8000) then
		self:GibThis();
	end
	
	--TODO: Add wounds through Lua like the other disintegrator weapons
	if SceneMan:GetTerrMatter(self.Pos.X, self.Pos.Y) == rte.airID then
		local moCheck = SceneMan:GetMOIDPixel(self.Pos.X, self.Pos.Y);
		if moCheck ~= rte.NoMOID then
			local actor = MovableMan:GetMOFromID(MovableMan:GetMOFromID(moCheck).RootID);
			if actor and actor.Team ~= self.Team then
				self:GibThis();
				if IsActor(actor) then
					local melter = CreateMOPixel("Disintegrator");
					melter.Pos = self.Pos;
					melter.Team = self.Team;
					melter.Sharpness = ToActor(actor).ID;
					melter.PinStrength = self.disintegrationStrength;
					MovableMan:AddMO(melter);
				end
			end
		end
	else
		self:GibThis();
	end

	self.moveTimer:Reset();
end