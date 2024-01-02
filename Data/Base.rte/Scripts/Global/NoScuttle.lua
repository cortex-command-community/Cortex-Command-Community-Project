function NoScuttleScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if IsACDropShip(actor) or IsACRocket(actor) then
			ToACraft(actor).ScuttleOnDeath = false;
		end
	end
end