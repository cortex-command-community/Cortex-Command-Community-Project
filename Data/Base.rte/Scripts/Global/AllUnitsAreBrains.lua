function AllUnitsAreBrainsScript:StartScript()
	self:CheckAllActors(false);
end

function AllUnitsAreBrainsScript:EndScript()
	self:CheckAllActors(true);
end

function AllUnitsAreBrainsScript:UpdateScript()
	self:CheckAllActors(false);
end

function AllUnitsAreBrainsScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function AllUnitsAreBrainsScript:CheckActorAndContents(actor, reverse)
	if IsAHuman(actor) or IsACrab(actor) then
		-- Only the units this made into brains stop being brains again.
		if reverse then
			if actor:NumberValueExists("AllUnitsAreBrainsScript") then
				actor:RemoveFromGroup("Brains");
				actor:RemoveNumberValue("AllUnitsAreBrainsScript");
			end
		elseif not actor:IsInGroup("Brains") then
			actor:AddToGroup("Brains");
			actor:SetNumberValue("AllUnitsAreBrainsScript", 1);
		end
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end
