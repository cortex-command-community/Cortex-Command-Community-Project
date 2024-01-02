function ToughCraftScript:StartScript()
	self.multiplier = 4;
end

function ToughCraftScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("ToughCraftScript") then
			actor:SetNumberValue("ToughCraftScript", 1);
			if IsACDropShip(actor) or IsACRocket(actor) then
				actor.GibWoundLimit = actor.GibWoundLimit * self.multiplier;
				actor.GibImpulseLimit = actor.GibImpulseLimit * self.multiplier;
				for att in actor.Attachables do
					att.GibWoundLimit = att.GibWoundLimit * self.multiplier;
					att.JointStrength = att.JointStrength * self.multiplier;
				end
			end
		end
	end
end