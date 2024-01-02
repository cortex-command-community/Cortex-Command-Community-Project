function MetaScript:StartScript()
	if SettingsMan.PrintDebugInfo then
		print (self.PresetName.." Create");
	end

	-- Init script
	if MetaMan.PlayerCount > 0 then
		-- Store activity
		self.Activity = ToGameActivity(ActivityMan:GetActivity());

		-- Determine which teams are CPU-controlled
		self.TeamIsCPU = {};

		for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
			self.TeamIsCPU[team] = true;
		end
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self.Activity:PlayerActive(player) then
				if self.Activity:PlayerHuman(player) then
					self.TeamIsCPU[self.Activity:GetTeamOfPlayer(player)] = false;
				end
			end
		end
	else
		if SettingsMan.PrintDebugInfo then
			print (self.PresetName .. ": Not a Metafight, terminating!");
		end
		-- No need to execute the script as we're not in metagame
		self:Deactivate();
	end
end

function MetaScript:UpdateScript()
	-- Only give orders if Activity is already running, not in deployment or building phases, or when activity ended
	-- Hold a pause before giving orders because brain might be unavailable till it lands
	if self.Activity and self.Activity.ActivityState == Activity.RUNNING then
		for actor in MovableMan.Actors do
			if self.TeamIsCPU[actor.Team] and not actor:IsInGroup("Brains") then
				if IsAHuman(actor) or IsACrab(actor) then
					if not actor:StringValueExists("ScriptControlled") or actor:GetStringValue("ScriptControlled") == self.PresetName then
						if actor.AIMode ~= Actor.AIMODE_BRAINHUNT then
							actor:FlashWhite(10000);
							actor.AIMode = Actor.AIMODE_BRAINHUNT;
						end
						-- Mark actor as the one, which receives orders from external script, and MetaFight.lua won't interfere.
						if not actor:StringValueExists("ScriptControlled") then
							actor:SetStringValue("ScriptControlled", self.PresetName);
						end
					end
				end
			end
		end
	end
end

function MetaScript:EndScript()
	if SettingsMan.PrintDebugInfo then
		print (self.PresetName.." Destroy");
	end
end

function MetaScript:PauseScript()
	if SettingsMan.PrintDebugInfo then
		print (self.PresetName.." Pause");
	end
end

function MetaScript:CraftEnteredOrbit(orbitedCraft)
	if SettingsMan.PrintDebugInfo then
		print (self.PresetName.." Orbited");
		print (orbitedCraft);
	end
end
