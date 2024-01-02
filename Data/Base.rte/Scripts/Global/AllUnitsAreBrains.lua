function AllUnitsAreBrainsScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if (IsAHuman(actor) or IsACrab(actor)) and not actor:IsInGroup("Brains") then
			actor:AddToGroup("Brains");
		else
			for item in actor.Inventory do
				if (IsAHuman(item) or IsACrab(item)) and not item:IsInGroup("Brains") then
					ToActor(item):AddToGroup("Brains");
				end
			end
		end
	end
end