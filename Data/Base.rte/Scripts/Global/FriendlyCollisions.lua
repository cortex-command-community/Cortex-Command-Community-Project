function FriendlyCollisionsScript:StartScript()
	self:CheckAllActors(false);
end

function FriendlyCollisionsScript:EndScript()
	self:CheckAllActors(true);
end

function FriendlyCollisionsScript:UpdateScript()
	self:CheckAllActors(false);
end

function FriendlyCollisionsScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function FriendlyCollisionsScript:CheckActorAndContents(actor, reverse)
	if reverse then
		if actor:NumberValueExists("FriendlyCollisionsScript") then
			actor.IgnoresTeamHits = actor:GetNumberValue("FriendlyCollisionsScript") == 1;
			actor:RemoveNumberValue("FriendlyCollisionsScript");
		end
	else
		if not actor:NumberValueExists("FriendlyCollisionsScript") then
			actor:SetNumberValue("FriendlyCollisionsScript", actor.IgnoresTeamHits and 1 or 0);
		end
		-- AHumans and ACrabs turn this back on whenever they're created, which includes being loaded.
		actor.IgnoresTeamHits = false;
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end
