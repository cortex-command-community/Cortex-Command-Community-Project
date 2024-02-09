function OnMessage(self, message)
	if message == "RefineryAssault_S4TankBlownUp" then
		self.blowUpTimer = Timer();
		self.randTime = math.random(600, 750);
	end
end

function Create(self)
	self.GibSound = nil;
	self.MissionCritical = true;
	
	self.doorsTable = {};
	for actor in MovableMan.Actors do
		if actor.PresetName == "Refinery Objective Blast Door Opening" then
			table.insert(self.doorsTable, actor);
		end
	end
end

function Update(self)
	if self.blowUpTimer and self.blowUpTimer:IsPastSimMS(self.randTime) then
		self.MissionCritical = false;
		self:GibThis();
		local activity = ActivityMan:GetActivity();
		activity:SendMessage("RefineryAssault_S4DoorsBlownUp");
		for k, door in pairs(self.doorsTable) do
			-- one of these 2 extra tanks will blow up earlier, we expect a validmo check to fail on one of em
			if MovableMan:ValidMO(door) then
				door:GibThis();
			end
		end
	end
end