function OnAttach(self, newParent)
	local actor = newParent:GetRootParent();
	ToGameActivity(ActivityMan:GetActivity()):SendMessage("RefineryAssault_KeycardPickedUp", actor.UniqueID);
	if actor:IsInGroup("Brains") and actor.Team == 0 then
		self.ToDelete = true;
	else
		ToActor(actor):GetController():SetState(Controller.WEAPON_DROP, true);
	end
end