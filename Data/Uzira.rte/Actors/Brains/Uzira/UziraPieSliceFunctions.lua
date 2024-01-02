function EnableMinionSpawning(pieMenuOwner, pieMenu, pieSlice)
	pieMenuOwner:SetNumberValue("EnableMinionSpawning", 1);
	pieMenu:ReplacePieSlice(pieSlice, CreatePieSlice("DisableMinionSpawning", "Uzira.rte"));
end

function DisableMinionSpawning(pieMenuOwner, pieMenu, pieSlice)
	pieMenuOwner:SetNumberValue("EnableMinionSpawning", 0);
	pieMenu:ReplacePieSlice(pieSlice, CreatePieSlice("EnableMinionSpawning", "Uzira.rte"));
end

function MinionsGather(pieMenuOwner, pieMenu, pieSlice)
	pieMenuOwner:SetNumberValue("MinionsGather", 1);
	pieMenu:ReplacePieSlice(pieSlice, CreatePieSlice("MinionsStandby", "Uzira.rte"));
end

function MinionsStandby(pieMenuOwner, pieMenu, pieSlice)
	pieMenuOwner:SetNumberValue("MinionsGather", 0);
	pieMenu:ReplacePieSlice(pieSlice, CreatePieSlice("MinionsGather", "Uzira.rte"));
end

function MinionsFrenzy(pieMenuOwner, pieMenu, pieSlice)
	pieMenuOwner:SetNumberValue("MinionsFrenzy", 1);
	pieSlice.Enabled = false;
end