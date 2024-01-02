function FriendlyCollisionsScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		actor.IgnoresTeamHits = false;
		for passenger in actor.Inventory do
			if IsActor(passenger) then
				ToActor(passenger).IgnoresTeamHits = false;
			end
		end
	end
end