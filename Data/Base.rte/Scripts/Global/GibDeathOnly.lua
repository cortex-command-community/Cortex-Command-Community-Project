function GibDeathOnlyScript:StartScript()
	self:CheckAllActors(false);
end

function GibDeathOnlyScript:EndScript()
	self:CheckAllActors(true);
end

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
	end
	self:CheckAllActors(false);
end

function GibDeathOnlyScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function GibDeathOnlyScript:CheckActorAndContents(actor, reverse)
	self:MarkDamage(actor, reverse);
	for part in actor.Attachables do
		self:MarkDamage(part, reverse);
		for att in part.Attachables do
			self:MarkDamage(att, reverse);
		end
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end

function GibDeathOnlyScript:MarkDamage(object, reverse)
	if object:NumberValueExists("GibDeathOnlyScript") ~= reverse then
		return;
	end
	-- Multiplying by 0 can't be undone, so the original is what gets kept.
	if reverse then
		object.DamageMultiplier = object:GetNumberValue("GibDeathOnlyScript");
		object:RemoveNumberValue("GibDeathOnlyScript");
	else
		object:SetNumberValue("GibDeathOnlyScript", object.DamageMultiplier);
		object.DamageMultiplier = 0;
	end
end
