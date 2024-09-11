function DebugFunctionsScript:StartScript()
	
	self.Activity = ToGameActivity(ActivityMan:GetActivity());
	
	self.MOUtility = require("Scripts/Utility/MOUtility");	
	
end

function DebugFunctionsScript:UpdateScript()

	self.MOUtility:Update();

	-- Space + F to enter edit mode at any point
	if UInputMan:KeyPressed(Key.F) and UInputMan:KeyHeld(Key.SPACE) then
		self.Activity.ActivityState = Activity.EDITING;

		-- Remove and refund the player's brains (otherwise edit mode does not work)
		for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
			if self.Activity:PlayerActive(player) and self.Activity:PlayerHuman(player) then
				local brain = self.Activity:GetPlayerBrain(player);
				if MovableMan:IsActor(brain) then
					self.Activity:ChangeTeamFunds(brain:GetTotalValue(0, 1), self.Activity:GetTeamOfPlayer(player));
					MovableMan:RemoveActor(brain);
				end
			end
		end
	end
	
	-- Hold Alt to do any of these
	if UInputMan.FlagAltState then

		-- Keypad 9 for HUD Toggle
		if UInputMan:KeyPressed(Key.KP_9) then
			self.HUDToggle = not self.HUDToggle and true or false;
			if self.HUDToggle then
				FrameMan:SetHudDisabled(true, 0);
			else
				FrameMan:SetHudDisabled(false, 0);
			end
		end
		
		if self.HUDToggle then
			for actor in MovableMan.Actors do actor.HUDVisible = false end
		else
			for actor in MovableMan.Actors do actor.HUDVisible = true end
		end
		
		-- Keypad plus to give player teams 1000 funds
		if UInputMan:KeyPressed(Key.KP_PLUS) then
			for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
				if self.Activity:PlayerActive(player) and self.Activity:PlayerHuman(player) then
					self.Activity:ChangeTeamFunds(1000, self.Activity:GetTeamOfPlayer(player));
				end
			end
		end
		
		-- Keypad enter to skip mission stage for compatible activities
		if UInputMan:KeyPressed(Key.KP_ENTER) then
			self.Activity:SendMessage("SkipCurrentStage");
		end

		-- Press delete to destroy all actors except brains and the current selected one
		if UInputMan:KeyPressed(Key.DELETE) then
			for actor in MovableMan.Actors do 
				if not actor:IsPlayerControlled() and not actor:IsInGroup("Brains") and not actor.MissionCritical then 
					actor.ToDelete = true 
				end 
			end
		end
		
		-- L to reveal all unseen areas
		if UInputMan:KeyPressed(Key.L) then
			SceneMan:RevealUnseenBox(0, 0, SceneMan.Scene.Width, SceneMan.Scene.Height, 0);
			SceneMan:RevealUnseenBox(0, 0, SceneMan.Scene.Width, SceneMan.Scene.Height, 1);
			SceneMan:RevealUnseenBox(0, 0, SceneMan.Scene.Width, SceneMan.Scene.Height, 2);
			SceneMan:RevealUnseenBox(0, 0, SceneMan.Scene.Width, SceneMan.Scene.Height, 3);
		end

		-- I to toggle all brains unhittable and with frozen HP. Not fully invincible because that'd be code-intensive.
		-- Does not play well with scripts that make all actors brains for obvious reasons.
		if UInputMan:KeyPressed(Key.I) then
			local unhittable;
			for actor in MovableMan.Actors do
				if actor:IsInGroup("Brains") then
					unhittable = self.MOUtility:SetMOUnhittable(actor);
					self.MOUtility:SetFreezeActorHP(actor);
				end
			end
			print("INFO: DebugFunctions: All brains invincible is: " .. tostring(unhittable));
		end
	
		-- Team switch controlled actor: Keypad 1 is team -1, incremental
		local teamNum;
		local toSwitch = false;
		if UInputMan:KeyPressed(Key.KP_1) then
			teamNum = -1
			toSwitch = true;
		elseif UInputMan:KeyPressed(Key.KP_2) then
			teamNum = 0
			toSwitch = true;
		elseif UInputMan:KeyPressed(Key.KP_3) then
			teamNum = 1
			toSwitch = true;
		elseif UInputMan:KeyPressed(Key.KP_4) then
			teamNum = 2
			toSwitch = true;
		elseif UInputMan:KeyPressed(Key.KP_5) then
			teamNum = 3
			toSwitch = true;
		end
		if toSwitch then
			toSwitch = false;
			for actor in MovableMan.Actors do
				if actor:IsPlayerControlled() then
					MovableMan:ChangeActorTeam(actor, teamNum)
					if not IsACraft(actor) then
						actor.AIMode = Actor.AIMODE_SENTRY;
					end
					if IsACRocket(actor) or IsACDropShip(ToActor(actor)) then
						for item in actor.Inventory do
							if (IsAHuman(item) or IsACrab(item)) then
								MovableMan:ChangeActorTeam(ToActor(item), teamNum)
							end
						end
					end
				end
			end
		end
	end
	
end