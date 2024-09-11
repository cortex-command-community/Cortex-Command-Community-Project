function OnMessage(self, message)
	if message == "ActivateRefineryAuthorizationConsole" then
		self.Activated = true;
	end
end
	

function Create(self)
	-- hoo boy... the things we do for draw order
	local us = MovableMan:RemoveActor(self);
	if us then -- fixed issue when reloading scripts
		MovableMan:AddParticle(us);
	end
	
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.closeActorTable = {};
	self.actorUpdateTimer = Timer();
	self.actorUpdateDelay = 50;
	
	self.detectRange = 200;
	
	self.orderPieSlice = CreatePieSlice("Refinery Authorization Console Action", "Browncoats.rte");
end

function ThreadedUpdate(self)
	if self.Activated then
	
		if self.actorUpdateTimer:IsPastSimMS(self.actorUpdateDelay) then
			for actor in MovableMan:GetMOsInRadius(self.Pos, self.detectRange) do
				if (not self.closeActorTable[actor.UniqueID]) and (IsAHuman(actor) or IsACrab(actor)) and actor:IsInGroup("Brains") then
					actor = ToActor(actor);
					self.closeActorTable[actor.UniqueID] = actor.UniqueID;
				end
			end
			
			self:RequestSyncedUpdate();
		end
	end
end

function SyncedUpdate(self)
	if self.actorUpdateTimer:IsPastSimMS(self.actorUpdateDelay) then
		self.actorUpdateTimer:Reset();
		
		for k, v in pairs(self.closeActorTable) do
			local actor = MovableMan:FindObjectByUniqueID(v);
			if actor and MovableMan:ValidMO(actor) then
				actor = ToActor(actor);
				local dist = SceneMan:ShortestDistance(self.Pos, actor.Pos, true);
				if dist:MagnitudeIsGreaterThan(self.detectRange) then
					actor.PieMenu:RemovePieSlicesByPresetName(self.orderPieSlice.PresetName);
					actor:RemoveNumberValue("RefineryAuthorizationConsole_Authorize");
					self.closeActorTable[k] = nil;
				else
					actor.PieMenu:AddPieSliceIfPresetNameIsUnique(self.orderPieSlice, self);
					if actor:NumberValueExists("RefineryAuthorizationConsole_Authorize") then
						actor:RemoveNumberValue("RefineryAuthorizationConsole_Authorize");
						
						self.Activity:SendMessage("RefineryAssault_S7BrainAuthorized");
						for k, v in pairs(self.closeActorTable) do
							local actor = MovableMan:FindObjectByUniqueID(v);
							actor = ToActor(actor);
							if actor and MovableMan:ValidMO(actor) then
								actor.PieMenu:RemovePieSlicesByPresetName(self.orderPieSlice.PresetName);
								actor:RemoveNumberValue("RefineryAuthorizationConsole_Authorize");
							end
						end
						
						self:DisableScript();
						return;
							
					end
				end
			else
				self.closeActorTable[k] = nil;
			end
		end
	end
end