function OnGlobalMessage(self, message, team)

	if message == "Refinery_S10SpawnBoss" then
		local boss = CreateAHuman("Browncoat Boss Scripted", "Browncoats.rte");
		boss.Team = team;
		boss.Vel = Vector(0, 10);
		boss.Pos = self.Pos + Vector(0, -35);
		MovableMan:AddActor(boss);
		
		local activity = ToGameActivity(ActivityMan:GetActivity());
		activity:SendMessage("Refinery_S10BossUniqueIDReturn", boss.UniqueID);
	
		self:GibThis();
	end
end