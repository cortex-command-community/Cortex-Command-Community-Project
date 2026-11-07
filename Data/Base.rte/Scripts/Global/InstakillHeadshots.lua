function InstakillHeadshotsScript:StartScript()
	self:CheckAllActors(false);
end

function InstakillHeadshotsScript:EndScript()
	self:CheckAllActors(true);
end

function InstakillHeadshotsScript:UpdateScript()
	self:CheckAllActors(false);
end

function InstakillHeadshotsScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function InstakillHeadshotsScript:CheckActorAndContents(actor, reverse)
	if IsAHuman(actor) then
		self:MarkHead(ToAHuman(actor).Head, actor.MaxHealth, reverse);
	elseif IsACrab(actor) then
		self:MarkHead(ToACrab(actor).Turret, actor.MaxHealth, reverse);
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end

function InstakillHeadshotsScript:MarkHead(head, maxHealth, reverse)
	if not head or head:NumberValueExists("InstakillHeadshotsScript") ~= reverse then
		return;
	end
	if reverse then
		head.GibWoundLimit = head:GetNumberValue("InstakillHeadshotsScript_GibWoundLimit");
		head.DamageMultiplier = head:GetNumberValue("InstakillHeadshotsScript_DamageMultiplier");
		head:RemoveNumberValue("InstakillHeadshotsScript_GibWoundLimit");
		head:RemoveNumberValue("InstakillHeadshotsScript_DamageMultiplier");
		head:RemoveNumberValue("InstakillHeadshotsScript");
	else
		head:SetNumberValue("InstakillHeadshotsScript", 1);
		head:SetNumberValue("InstakillHeadshotsScript_GibWoundLimit", head.GibWoundLimit);
		head:SetNumberValue("InstakillHeadshotsScript_DamageMultiplier", head.DamageMultiplier);
		head.GibWoundLimit = 1;
		head.DamageMultiplier = maxHealth;
	end
end
