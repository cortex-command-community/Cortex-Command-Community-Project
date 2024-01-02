function InvincibleCraftScript:UpdateScript()
	for actor in MovableMan.AddedActors do
		if not actor:NumberValueExists("InvincibleCraftScript") then
			actor:SetNumberValue("InvincibleCraftScript", 1);
			if IsACDropShip(actor) or IsACRocket(actor) then
				actor.HitsMOs = false;
				actor.GetsHitByMOs = false;
				actor.GibImpulseLimit = actor.GibImpulseLimit * 2;
				actor.ImpulseDamageThreshold = actor.ImpulseDamageThreshold * 2;
				for att in actor.Attachables do
					att.HitsMOs = false;
					att.GetsHitByMOs = false;
					att.JointStrength = 1000000;
				end
			end
		end
	end
end