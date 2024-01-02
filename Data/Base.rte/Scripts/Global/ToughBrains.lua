function ToughBrainsScript:StartScript()
	self.multiplier = 3;
end

function ToughBrainsScript:UpdateScript()
	for actor in MovableMan.Actors do
		if actor:IsInGroup("Brains") and not actor:NumberValueExists("ToughBrainsScript") then
			actor:SetNumberValue("ToughBrainsScript", 1);
			actor.GibWoundLimit = actor.GibWoundLimit * self.multiplier;
			actor.GibImpulseLimit = actor.GibImpulseLimit * self.multiplier;
			for limb in actor.Attachables do
				limb.GibWoundLimit = limb.GibWoundLimit * self.multiplier;
				limb.JointStrength = limb.JointStrength * self.multiplier;
				for att in limb.Attachables do
					att.GibWoundLimit = att.GibWoundLimit * self.multiplier;
					att.JointStrength = att.JointStrength * self.multiplier;
				end
			end
			if IsACrab(actor) and ToACrab(actor).EquippedItem then
				local weapon = ToACrab(actor).EquippedItem;
				weapon.GibWoundLimit = weapon.GibWoundLimit * self.multiplier;
				weapon.JointStrength = weapon.JointStrength * self.multiplier;
			end
		end
	end
end