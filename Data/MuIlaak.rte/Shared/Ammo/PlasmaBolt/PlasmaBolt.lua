function Create(self)
	self.speed = 35;
	self.TraceValue = 120

	local raylength = 100;
	local rayPixSpace = 15;

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

	self.moveTimer = Timer();
	self.lifeTimer = Timer();

end

function Update(self)

	if self.target ~= nil and self.target.ID ~= rte.NoMOID then
		self.tPos = Vector(self.target.Pos.X, self.target.Pos.Y);
		useAdjust = self.targetingAdjustmentAmount;
		self.seekerDelay = 0;
	else
		self.target = nil;
	end

	if self.tPos ~= nil then
		local checkDirB = SceneMan:ShortestDistance(self.Pos, self.tPos, SceneMan.SceneWrapsX);
		local checkDir = checkDirB:SetMagnitude(self.TraceValue * (self.moveTimer.ElapsedSimTimeMS/1000));
		self.Vel = Vector(self.Vel.X + checkDir.X, self.Vel.Y + checkDir.Y):SetMagnitude(self.speed);
	end

	if self.lifeTimer:IsPastSimMS(3000) then
		self:GibThis();
	end
	self.moveTimer:Reset();
end
