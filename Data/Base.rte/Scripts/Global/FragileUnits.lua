function FragileUnitsScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("FragileUnitsScript") then
			self:MakeFragile(actor);
			for item in actor.Inventory do
				self:MakeFragile(item);
			end
		end
	end
end

function FragileUnitsScript:MakeFragile(actor)
	if IsAHuman(actor) or IsACrab(actor) then
		actor = ToActor(actor);
		actor.MaxHealth = 1000;
		actor.Health = actor.MaxHealth;
		actor:SetNumberValue("FragileUnitsScript", 1);
		actor.GibWoundLimit = 1;
		for att in actor.Attachables do
			att.GibWoundLimit = 1;
			if not IsAEmitter(att) and not IsArm(att) and not IsTurret(att) then
				att.JointStrength = 1;
			end
		end
	end
end