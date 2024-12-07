function OnGlobalMessage(self, message, object)
	if message == "Captured_RefineryOptionalDoorConsole" .. self:GetStringValue("DoorIdentifier") then
		self.Team = 0;
	end
end