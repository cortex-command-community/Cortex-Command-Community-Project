function Update(self)
	if self.Foot then
		local footNameSuffix = "A";
		for attachable in self.Attachables do
			if attachable.PresetName:find("Leg Armour") then
				footNameSuffix = "B";
				break;
			end
		end
		self.Foot = CreateAttachable(string.sub(self.Foot.PresetName, 1, -2) .. footNameSuffix, "Uzira.rte");
	end
	
	self:DisableScript();
end