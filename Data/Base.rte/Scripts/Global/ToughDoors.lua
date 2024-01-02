function ToughDoorsScript:StartScript()
	self.multiplier = 4;
end

function ToughDoorsScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("ToughDoorsScript") then
			actor:SetNumberValue("ToughDoorsScript", 1);
			if IsADoor(actor) then
				local door = ToADoor(actor);
				door.GibWoundLimit = door.GibWoundLimit * self.multiplier;
				door.GibImpulseLimit = door.GibImpulseLimit * self.multiplier;
				if door.Door then
					door.Door.GibWoundLimit = door.Door.GibWoundLimit * self.multiplier;
					door.Door.GibImpulseLimit = door.Door.GibImpulseLimit * self.multiplier;
					door.Door.JointStrength = door.Door.JointStrength * self.multiplier;
				end
			end
		end
	end
end