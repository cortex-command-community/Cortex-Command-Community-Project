function Create(self)
	-- TODO figure out something better than having all these mosrotatings in Actors
	self.auxiliaryTanksTable = {};
	for par in MovableMan.Particles do
		if par.PresetName == "Browncoat Refinery Fuel Tank Large Vertical S4 Auxiliary" then
			table.insert(self.auxiliaryTanksTable, par);
		end
	end

	self.doorsTable = {};
	for actor in MovableMan.Actors do
		if actor.PresetName == "Refinery Objective Blast Door Stuck" then
			table.insert(self.doorsTable, actor);
		end
	end
end

function Update(self)
	if self.blowUpTimer then
		if self.blowUpTimer:IsPastSimMS(1000) then
		
			self.MissionCritical = false;
			self:GibThis();
			CameraMan:AddScreenShake(30, self.Pos);
			
			for k, door in pairs(self.doorsTable) do
				if MovableMan:ValidMO(door) then
					door:GibThis();
				end
			end
			
			local activity = ActivityMan:GetActivity();
			activity:SendMessage("RefineryAssault_S4TankBlownUp");
			for k, tank in pairs(self.auxiliaryTanksTable) do
				if MovableMan:ValidMO(tank) then
					tank:SendMessage("RefineryAssault_S4TankBlownUp");
				end
			end
		end
	elseif self.WoundCount > 100 then
		self.blowUpTimer = Timer();
		local soundContainer = CreateSoundContainer("Yskely Refinery S4 Tank Burst");
		soundContainer:Play(self.Pos);
	end
end