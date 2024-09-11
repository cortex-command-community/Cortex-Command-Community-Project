function OnGlobalMessage(self, message, team)

	if message == "Refinery_S10SpawnBoss" then
		local boss = CreateAHuman("Browncoat Boss Scripted", "Browncoats.rte");
		boss.Team = team;
		boss.Vel = Vector(0, 10);
		boss.Pos = self.Pos + Vector(0, -35);
		MovableMan:AddActor(boss);
	
		self:GibThis();
	end
end

function OnMessage(self, message, team)

	if message == "Refinery_S10SpawnBoss" then
		local boss = CreateAHuman("Browncoat Boss Scripted", "Browncoats.rte");
		boss.Team = team;
		boss.Vel = Vector(0, 10);
		boss.Pos = self.Pos + Vector(0, -35);
		MovableMan:AddActor(boss);
	
		self:GibThis();
	end
end