function GibDeathOnlyScript:UpdateScript()
	for actor in MovableMan.Actors do
		if actor.Health > 0 then
			if not IsADoor(actor) then
				actor.Health = actor.MaxHealth;
			else
				-- Only heal doors if the door itself is still there, otherwise door engine won't explode
				local door = ToADoor(actor);
				if door.Door then
					actor.Health = actor.MaxHealth;
				end
			end
		end
		if not actor:NumberValueExists("GibDeathOnlyScript") then
			actor:SetNumberValue("GibDeathOnlyScript", 1);
			actor.DamageMultiplier = 0;
			for part in actor.Attachables do
				part.DamageMultiplier = 0;
				for att in part.Attachables do
					att.DamageMultiplier = 0;
				end
			end
		end
	end
end