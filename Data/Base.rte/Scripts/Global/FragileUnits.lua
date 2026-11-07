function FragileUnitsScript:StartScript()
	self:CheckAllActors(false);
end

function FragileUnitsScript:EndScript()
	self:CheckAllActors(true);
end

function FragileUnitsScript:UpdateScript()
	self:CheckAllActors(false);
end

function FragileUnitsScript:CheckAllActors(reverse)
	for actor in MovableMan.Actors do
		self:CheckActorAndContents(actor, reverse);
	end
	for actor in MovableMan.AddedActors do
		self:CheckActorAndContents(actor, reverse);
	end
end

function FragileUnitsScript:CheckActorAndContents(actor, reverse)
	if IsAHuman(actor) or IsACrab(actor) then
		self:MarkBody(actor, reverse);
		for att in actor.Attachables do
			self:MarkPart(att, reverse);
		end
	end
	for item in actor.Inventory do
		if IsActor(item) then
			self:CheckActorAndContents(ToActor(item), reverse);
		end
	end
end

function FragileUnitsScript:MarkBody(actor, reverse)
	if actor:NumberValueExists("FragileUnitsScript") ~= reverse then
		return;
	end
	if reverse then
		local maxHealth = actor:GetNumberValue("FragileUnitsScript_MaxHealth");
		actor.Health = actor.Health / actor.MaxHealth * maxHealth;
		actor.MaxHealth = maxHealth;
		actor.GibWoundLimit = actor:GetNumberValue("FragileUnitsScript_GibWoundLimit");
		actor:RemoveNumberValue("FragileUnitsScript_MaxHealth");
		actor:RemoveNumberValue("FragileUnitsScript_GibWoundLimit");
		actor:RemoveNumberValue("FragileUnitsScript");
	else
		actor:SetNumberValue("FragileUnitsScript", 1);
		actor:SetNumberValue("FragileUnitsScript_MaxHealth", actor.MaxHealth);
		actor:SetNumberValue("FragileUnitsScript_GibWoundLimit", actor.GibWoundLimit);
		actor.MaxHealth = 1000;
		actor.Health = actor.MaxHealth;
		actor.GibWoundLimit = 1;
	end
end

function FragileUnitsScript:MarkPart(att, reverse)
	if att:NumberValueExists("FragileUnitsScript") ~= reverse then
		return;
	end
	local weakensJoint = not IsAEmitter(att) and not IsArm(att) and not IsTurret(att);
	if reverse then
		att.GibWoundLimit = att:GetNumberValue("FragileUnitsScript_GibWoundLimit");
		att:RemoveNumberValue("FragileUnitsScript_GibWoundLimit");
		if weakensJoint then
			att.JointStrength = att:GetNumberValue("FragileUnitsScript_JointStrength");
			att:RemoveNumberValue("FragileUnitsScript_JointStrength");
		end
		att:RemoveNumberValue("FragileUnitsScript");
	else
		att:SetNumberValue("FragileUnitsScript", 1);
		att:SetNumberValue("FragileUnitsScript_GibWoundLimit", att.GibWoundLimit);
		att.GibWoundLimit = 1;
		if weakensJoint then
			att:SetNumberValue("FragileUnitsScript_JointStrength", att.JointStrength);
			att.JointStrength = 1;
		end
	end
end
