function OnGlobalMessage(self, message, object)
	if message == self.deactivationMessage or message == "DEACTIVATEALLINFOCONSOLES" then
		self.Deactivated = true;
	elseif message == self.activationMessage or message == "ACTIVATEALLINFOCONSOLES" then
		self.Deactivated = false;
	end
end

function OnMessage(self, message, object)
	if message == self.deactivationMessage or message == "DEACTIVATEALLINFOCONSOLES" then
		self.Deactivated = true;
	elseif message == self.activationMessage or message == "ACTIVATEALLINFOCONSOLES" then
		self.Deactivated = false;
	end
end

function Create(self)
	self.Team = self:NumberValueExists("StartTeam") and self:GetNumberValue("StartTeam") or 0;
	
	self.deactivationMessage = self:GetStringValue("DeactivationMessage");
	self.activationMessage = self:GetStringValue("ActivationMessage");
	
	if self:GetNumberValue("Deactivated") == 1 then
		self.Deactivated = true;
	end
	
	self:RemoveNumberValue("Deactivated");
	
	self.actorCheckTimer = Timer();
	self.actorCheckDelay = 250;
	
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	self.Scene = SceneMan.Scene;
	if self:StringValueExists("SceneInfoArea") then
		if self.Scene:HasArea(self:GetStringValue("SceneInfoArea")) then
			self.infoArea = self.Scene:GetArea(self:GetStringValue("SceneInfoArea"));
		end
	end
	
	if self:NumberValueExists("Range") then
		self.Range = self:GetNumberValue("Range");
	end
	self.displayToAllTeams = self:GetNumberValue("DisplayToAllTeams") == 1 and true or false;
	self.messageString = self:GetStringValue("MessageToDisplay");
	
	self.newLines = 0;
	local stringIndex = 1;
	while true do
		if string.find(self.messageString, "/n", stringIndex) then
			stringIndex = string.find(self.messageString, "/n", stringIndex) + 2;
			self.newLines = self.newLines + 1;
		else
			break;
		end
	end
	-- Now replace all /n with real newlines
	self.messageToDisplay = string.gsub(self.messageString, [[/n]],
[[

]])	
	
end

function ThreadedUpdate(self)
	if self.actorCheckTimer:IsPastSimMS(self.actorCheckDelay) then
		self.actorCheckTimer:Reset();
		
		local validDetection = false;

		if self.infoArea then
			for box in self.infoArea.Boxes do
				for actor in MovableMan:GetMOsInBox(box, -1, true) do
					if IsActor(actor) then
						if ToActor(actor):IsPlayerControlled() then
							if self.displayToAllTeams then
								validDetection = true;
							elseif actor.Team == self.Team then
								validDetection = true;
							end
						end
					end
				end
			end
		else -- range
			local actorList = MovableMan:GetMOsInRadius(self.Pos, self.Range or self.Diameter, -1, true);
			for actor in actorList do
				if IsActor(actor) then
					if ToActor(actor):IsPlayerControlled() then
						if self.displayToAllTeams then
							validDetection = true;
						elseif actor.Team == self.Team then
							validDetection = true;
						end
					end
				end
			end
		end
		
		if validDetection then
			self.shouldDisplayMessage = true;
		else
			self.shouldDisplayMessage = false;
		end
		
		-- Also check how many newlines we have
		self.newLines = 0;
		local stringIndex = 1;
		while true do
			if string.find(self.messageString, "/n", stringIndex) then
				stringIndex = string.find(self.messageString, "/n", stringIndex) + 2;
				self.newLines = self.newLines + 1;
			else
				break;
			end
		end
		-- Now replace all /n with real newlines
		self.messageToDisplay = string.gsub(self.messageString, [[/n]],
[[

]])
	end
	
	if self.shouldDisplayMessage then
		PrimitiveMan:DrawTextPrimitive(self.Pos + Vector(0, -40) + Vector(0, -9 * self.newLines), self.messageToDisplay, true, 1);
	end

end

function OnSave(self)
	self:SetNumberValue("Deactivated", self.Deactivated and 1 or 0);
end