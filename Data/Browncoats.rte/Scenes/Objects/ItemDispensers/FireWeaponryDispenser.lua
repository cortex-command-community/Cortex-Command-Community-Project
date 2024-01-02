function OnGlobalMessage(self, message)

	if message == "Captured_RefineryS3FireWeaponryConsole" then
		self:SendMessage("ACTIVATEALLITEMDISPENSERS");
	end

end

function Create(self)

	self:SendMessage("DEACTIVATEALLITEMDISPENSERS");
	
end