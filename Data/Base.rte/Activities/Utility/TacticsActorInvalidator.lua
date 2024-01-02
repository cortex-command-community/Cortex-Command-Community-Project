function Create(self)

	if self:StringValueExists("tacticsActorInvalidatorSquadInfo") then
		--print("tried to create load serialized invalidator")
		--print(self:LoadString("tacticsActorInvalidatorSquadInfo"))
		self.tacticsHandlerSquadInfo = loadstring("return " .. self:GetEncodedStringValue("tacticsActorInvalidatorSquadInfo"))()
	end
	
end

function OnMessage(self, message, squadInfo)

	if message == "TacticsHandler_InitSquadInfo" then
		-- a table of three values: team, squad index, and our index within that squad
		self.tacticsHandlerSquadInfo = {};
		for k, v in pairs(squadInfo) do
			self.tacticsHandlerSquadInfo[k] = v;
			--print("got squad info:" .. k .. v)
		end
		
		self.saveLoadHandler = require("Activities/Utility/SaveLoadHandler");
		self.saveLoadHandler:Initialize(self);
		
	elseif message == "TacticsHandler_UpdateSquadIndex" then
		self.tacticsHandlerSquadInfo.squadIndex = squadInfo;
		--print("Actor updated squad index");
	end
	
end

function OnSave(self)

	--print("tried to save...?")
	self:SetEncodedStringValue("tacticsActorInvalidatorSquadInfo", self.saveLoadHandler:SerializeTable(self.tacticsHandlerSquadInfo))		

end

function Destroy(self)

--	if self.PresetName then
--		print("On Destroy, we invalidated this actor of team " .. self.Team);
---		print(self.PresetName)
--	end
	local activity = ActivityMan:GetActivity();
	if activity then
		activity:SendMessage("TacticsHandler_InvalidateActor", self.tacticsHandlerSquadInfo);
	end
end