function Create(self)

end

function Update(self)

	if UInputMan:KeyPressed(Key.P) then
	
		-- TODO: Get team from activity somehow
	
		local boss = CreateAHuman("Browncoat Boss Scripted", "Browncoats.rte");
		boss.Team = 0;
		boss.Vel = Vector(0, 10);
		boss.Pos = self.Pos + Vector(0, -35);
		MovableMan:AddActor(boss);
	
		self:GibThis();
	end
	
end