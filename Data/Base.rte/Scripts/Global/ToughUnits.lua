function ToughUnitsScript:StartScript()
	self.multiplier = 2;
end

function ToughUnitsScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("ToughUnitsScript") then
			if IsAHuman(actor) or IsACrab(actor) then
				self:Buff(actor);
			else
				for item in actor.Inventory do
					if IsActor(item) then
						self:Buff(ToActor(item));
					end
				end
			end
		end
	end
end

function ToughUnitsScript:Buff(actor)
	actor:SetNumberValue("ToughUnitsScript", 1);
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
end