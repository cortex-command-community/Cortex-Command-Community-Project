function Create(self)
	self.lifeTimer = Timer();

	self.raylength = 200;
	self.rayPixSpace = 5;

	self.dots = math.floor(self.raylength/self.rayPixSpace);
end

function Update(self)
	if self.lifeTimer:IsPastSimMS(4000) then
		self:GibThis();
	end
	
	for i = 1, self.dots do
		local checkPos = self.Pos + Vector(self.Vel.X, self.Vel.Y):SetMagnitude((i/self.dots) * self.raylength);
		if SceneMan.SceneWrapsX then
			if checkPos.X > SceneMan.SceneWidth then
				checkPos = Vector(checkPos.X - SceneMan.SceneWidth, checkPos.Y);
			elseif checkPos.X < 0 then
				checkPos = Vector(SceneMan.SceneWidth + checkPos.X, checkPos.Y);
			end
		end

		local terrCheck = SceneMan:GetTerrMatter(checkPos.X, checkPos.Y);
		if terrCheck == rte.airID then
			local moCheck = SceneMan:GetMOIDPixel(checkPos.X,checkPos.Y);
			if moCheck ~= rte.NoMOID then
				local actor = MovableMan:GetMOFromID(MovableMan:GetMOFromID(moCheck).RootID);
				if actor.Team ~= self.Team then
					self:GibThis();
				end
			end
		else
			self:GibThis();
		end
	end
end