local imagePath = "Missions.rte/Objects/Tutorial/";

-- The screens and room signs are drawn onto the bunker's back wall, so they show in front of the terrain background layer but behind the terrain foreground and actors.
local wallDrawDepth = DrawDepth.TerrainBackground - 1;

local screenHalfSize = Vector(23, 20);
local signHalfSize = Vector(24, 24);

local screenOffImage = imagePath .. "ScreenStatic000.png";
local screenStaticLittleImage = imagePath .. "ScreenStatic001.png";
local screenStaticLargeImage = imagePath .. "ScreenStatic002.png";

local function TutorialStep(text, duration, imageName, frameCount, frameDuration)
	local step = { text = text, duration = duration, frameDuration = frameDuration or 250, frames = {} };
	for frame = 0, frameCount - 1 do
		step.frames[frame + 1] = imagePath .. imageName .. string.format("%03d", frame) .. ".png";
	end
	return step;
end

function TutorialMission:StartActivity(isNewGame)
	self.tutorialPlayer = Activity.PLAYER_1;

	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		if self:TeamActive(team) and team ~= self.CPUTeam then
			self:SetTeamFunds(self:GetStartingGold(), team);
		end
	end
	self:SetLZArea(Activity.TEAM_1, SceneMan.Scene:GetArea("LZ Team 1"));

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			-- Expand all modules to show the goods
			self:GetBuyGUI(player):SetModuleExpanded(0, true);
			-- No need for the LZ to follow the brain, the scene-set one is enough
			self:SetBrainLZWidth(player, 0);

			if self:GetPlayerBrain(player) then
				self.tutorialPlayer = player;
			else
				ConsoleMan:PrintString("ERROR: Can't find brain for tutorial game mode!");
			end
		end
	end

	self.CPUBrain = MovableMan:GetUnassignedBrain(self.CPUTeam) or MovableMan:GetFirstBrainActor(self.CPUTeam);
	self.CPUActorCount = self:CountCPUActors();

	-- Keep everyone in place, the practice bodies included, and disable all enemy AIs so they don't attack prematurely
	for team = Activity.TEAM_1, Activity.MAXTEAMCOUNT - 1 do
		self:SetTeamAIMode(team, Actor.AIMODE_SENTRY);
	end
	self:DisableAIs(true, self.CPUTeam);
	self.fightTrigger = SceneMan.Scene:GetArea("AIAttack");
	self.fightStarted = false;

	self.roomSigns = {
		{ pos = Vector(744, 695), name = "TutEntryA" },
		{ pos = Vector(888, 695), name = "TutEntryB" },
		{ pos = Vector(888, 599), name = "TutEntryC" },
		{ pos = Vector(888, 431), name = "TutEntryD" }
	};
	self:SetupAreas();

	self.areaTimer = Timer();
	self.stepTimer = Timer();
	self.currentArea = 1;
	self.previousArea = nil;
	self.currentStep = 1;
	self.currentRoom = self.areas[self.currentArea].room;

	MusicMan:PlayDynamicSong("Generic Battle Music");
end

function TutorialMission:PauseActivity(pause)
	-- Re-setup the areas with any updated control mappings that the player might have made in the menu
	if not pause then
		self:SetupAreas();
	end
end

function TutorialMission:EndActivity()
	-- UpdateActivity stops running once the game is over, so clear the markers it would otherwise keep re-adding
	self:ClearObjectivePoints();

	local playerWon = false;
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) then
			local screen = self:ScreenOfPlayer(player);
			FrameMan:ClearScreenText(screen);
			if self:GetTeamOfPlayer(player) == self.WinnerTeam then
				playerWon = true;
				FrameMan:SetScreenText("You destroyed the dummy CPU!\nPress [SPACE] or [START] to continue", screen, 0, -1, false);
			else
				FrameMan:SetScreenText("Your brain has been destroyed!", screen, 0, -1, false);
			end
		end
	end

	-- Temp fix so music doesn't start playing if ending the Activity when changing resolution through the in-game settings.
	if not self:IsPaused() then
		if playerWon then
			MusicMan:PlayDynamicSong("Generic Victory Music", "Default", true);
		else
			MusicMan:PlayDynamicSong("Generic Defeat Music", "Default", true);
		end
		MusicMan:PlayDynamicSong("Generic Ambient Music");
	end
end

function TutorialMission:UpdateActivity()
	-- Clear all objective markers, they get re-added each frame
	self:ClearObjectivePoints();

	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if self:PlayerActive(player) and self:PlayerHuman(player) and self.ActivityState ~= Activity.OVER then
			local team = self:GetTeamOfPlayer(player);
			local brain = self:GetPlayerBrain(player);
			if not brain or not MovableMan:IsActor(brain) then
				self:SetPlayerBrain(nil, player);
				FrameMan:SetScreenText("Your brain has been destroyed!", self:ScreenOfPlayer(player), 333, -1, false);

				-- Now see if all brains of this player's team are dead, and if so, end the game
				if not MovableMan:GetFirstBrainActor(team) then
					self.WinnerTeam = self:OtherTeam(team);
					ActivityMan:EndActivity();
				end
				self:ResetMessageTimer(player);
			elseif self.fightStarted then
				self:AddObjectivePoint("Protect!", brain.Pos + Vector(0, 10), team, GameActivity.ARROWUP);
				if MovableMan:IsActor(self.CPUBrain) then
					self:AddObjectivePoint("Destroy!", self.CPUBrain.Pos + Vector(0, 12), team, GameActivity.ARROWUP);
				end
			end
		end
	end

	self:UpdateTutorialAreas();
	self:DrawTutorialAreas();

	local controlledActor = self:GetControlledActor(self.tutorialPlayer);
	if not self.fightStarted and ((controlledActor and self.fightTrigger:IsInside(controlledActor.Pos)) or self:CountCPUActors() < self.CPUActorCount) then
		local screen = self:ScreenOfPlayer(self.tutorialPlayer);
		self:ResetMessageTimer(self.tutorialPlayer);
		FrameMan:ClearScreenText(screen);
		FrameMan:SetScreenText("DEFEND YOUR BRAIN AGAINST THE INCOMING FORCES!", screen, 500, 8000, true);
		self:SetTeamAIMode(self.CPUTeam, Actor.AIMODE_BRAINHUNT);
		self:DisableAIs(false, self.CPUTeam);
		self.fightStarted = true;
	end

	if self.ActivityState ~= Activity.OVER and not MovableMan:IsActor(self.CPUBrain) then
		self.WinnerTeam = Activity.TEAM_1;
		ActivityMan:EndActivity();
	end
end

function TutorialMission:SetTeamAIMode(team, aiMode)
	for _, actors in ipairs({ MovableMan.Actors, MovableMan.AddedActors }) do
		for actor in actors do
			if actor.Team == team and (IsAHuman(actor) or IsACrab(actor)) and not actor:IsPlayerControlled() then
				actor.AIMode = aiMode;
			end
		end
	end
end

function TutorialMission:CountCPUActors()
	local count = 0;
	for actor in MovableMan.Actors do
		if actor.Team == self.CPUTeam then
			count = count + 1;
		end
	end
	for actor in MovableMan.AddedActors do
		if actor.Team == self.CPUTeam then
			count = count + 1;
		end
	end
	return count;
end

function TutorialMission:UpdateTutorialAreas()
	-- Switch to a new area when the player's actor enters its trigger box
	local controlledActor = self:GetControlledActor(self.tutorialPlayer);
	if controlledActor then
		for areaIndex, area in ipairs(self.areas) do
			if areaIndex ~= self.currentArea and area.triggerBox:IsWithinBox(controlledActor.Pos) then
				self.previousArea = self.currentArea;
				self.currentArea = areaIndex;
				self.currentStep = 1;
				self.areaTimer:Reset();
				self.stepTimer:Reset();
			end
		end
	end

	-- Cycle through the steps of the current area, looping around to the first
	local steps = self.areas[self.currentArea].steps;
	if self.stepTimer:IsPastRealMS(steps[self.currentStep].duration) then
		self.currentStep = self.currentStep % #steps + 1;
		self.stepTimer:Reset();
	end

	self.currentRoom = self.areas[self.currentArea].room or self.currentRoom;
end

function TutorialMission:DrawTutorialAreas()
	local primitives = {};
	local areaTime = self.areaTimer.ElapsedRealTimeMS;

	-- The current area's screen turns on through static, the previous area's turns off through static, and all others stay off
	for areaIndex, area in ipairs(self.areas) do
		local image = screenOffImage;
		if areaIndex == self.currentArea then
			if areaTime > 200 then
				local step = area.steps[self.currentStep];
				image = step.frames[math.floor(self.stepTimer.ElapsedRealTimeMS / step.frameDuration) % #step.frames + 1];
			else
				image = areaTime > 100 and screenStaticLargeImage or screenStaticLittleImage;
			end
		elseif areaIndex == self.previousArea and areaTime <= 200 then
			image = areaTime > 100 and screenStaticLittleImage or screenStaticLargeImage;
		end
		table.insert(primitives, BitmapPrimitive(-1, area.screenPos + screenHalfSize, image, 0, false, false, wallDrawDepth));
	end

	-- Light up the signs of the rooms reached so far, and blink the next room's sign
	for roomIndex, sign in ipairs(self.roomSigns) do
		local lit = roomIndex <= self.currentRoom or (roomIndex == self.currentRoom + 1 and self.areaTimer:AlternateReal(200));
		local image = imagePath .. sign.name .. (lit and "001.png" or "000.png");
		table.insert(primitives, BitmapPrimitive(-1, sign.pos + signHalfSize, image, 0, false, false, wallDrawDepth));
	end

	PrimitiveMan:DrawPrimitives(DrawBlendMode.NoBlend, 0, primitives);

	-- Reveal the current step's text one character at a time, then blink dots after it
	local area = self.areas[self.currentArea];
	local text = area.steps[self.currentStep].text;
	local stepDuration = area.steps[self.currentStep].duration;
	local revealPeriod = math.min((#text + 3) * 30, stepDuration * 0.85);
	local revealed = math.min(self.stepTimer.ElapsedRealTimeMS / revealPeriod, 1);
	local revealText = text:sub(1, math.floor((#text + 3) * revealed));
	if revealed == 1 then
		local timePhase = areaTime % 1200;
		revealText = revealText .. (timePhase > 900 and "..." or (timePhase > 600 and ".. " or (timePhase > 300 and ".  " or "   ")));
	end
	PrimitiveMan:DrawTextPrimitive(area.screenPos + area.textOffset, revealText, true, 1);
end

function TutorialMission:SetupAreas()
	local controlScheme = UInputMan:GetControlScheme(self.tutorialPlayer);
	local function MappingName(inputElement)
		return controlScheme:GetMappingName(inputElement);
	end

	local prevName = MappingName(InputElements.INPUT_PREV);
	local nextName = MappingName(InputElements.INPUT_NEXT);
	local jumpName = MappingName(InputElements.INPUT_JUMP);
	local proneName = MappingName(InputElements.INPUT_PRONE);

	local pieName = MappingName(InputElements.INPUT_PIEMENU_ANALOG);
	if pieName == "" then
		pieName = MappingName(InputElements.INPUT_PIEMENU_DIGITAL);
	end
	local fireName = MappingName(InputElements.INPUT_FIRE);

	-- If no preset, adjust the pie menu and fire names when using the defaults on a gamepad, otherwise they'll show up as unhelpful axis names
	if controlScheme.Device >= InputDevice.DEVICE_GAMEPAD_1 and controlScheme.Preset == InputScheme.NoPreset then
		pieName = "Pie Menu Trigger";
		fireName = "Fire Trigger";
	end

	local textBelowScreen = Vector(screenHalfSize.X, screenHalfSize.Y * 2 + 4);
	local textAboveScreen = Vector(screenHalfSize.X, -16);

	-- The room field is the room sign this area lights up to; areas without one keep the current room
	self.areas = {
		{
			-- Brain chamber
			triggerBox = Box(Vector(631, 608), 137, 155),
			screenPos = Vector(673, 688),
			textOffset = textBelowScreen,
			room = 1,
			steps = {
				TutorialStep("Welcome to Cortex Command", 4000, "CCLogo", 2),
				TutorialStep("Above is your disembodied brain", 4000, "ArrowUp", 2),
				TutorialStep("With it, you can remotely control other bodies", 4000, "BodyZap", 2),
				TutorialStep("Switch to one now by using [" .. prevName .. "] or [" .. nextName .. "]", 4000, "BodyZap", 2),
				TutorialStep("If you haven't set up your controls yet, hit [Esc] and do so", 8000, "Joystick", 2)
			}
		},
		{
			-- Body storage
			triggerBox = Box(Vector(911, 662), 397, 98),
			screenPos = Vector(961, 688),
			textOffset = textBelowScreen,
			room = 2,
			steps = {
				TutorialStep("Here are some dummy bodies for practice", 4000, "BodyHop", 2),
				TutorialStep("Quickly switch control between them left and right with [" .. prevName .. "] and [" .. nextName .. "]", 6000, "BodyZap", 2),
				TutorialStep("Or hold down either [" .. prevName .. "] or [" .. nextName .. "] to get a selection cursor", 6000, "BodyZap", 2),
				TutorialStep("Switch to the leftmost body and walk it out of the room with [" .. MappingName(InputElements.INPUT_L_LEFT) .. "]", 8000, "ArrowLeft", 2)
			}
		},
		{
			-- Shaft
			triggerBox = Box(Vector(772, 385), 135, 380),
			screenPos = Vector(817, 688),
			textOffset = textBelowScreen,
			steps = {
				TutorialStep("Use [" .. jumpName .. "] to activate jetpack", 4000, "ArrowUp", 2),
				TutorialStep("Fire jetpack in bursts for better control", 4000, "BodyJetpack", 2),
				TutorialStep("Adjust the jet direction by aiming or looking", 4000, "BodyJetpack", 2),
				TutorialStep("Jump height is affected by the body's total weight", 4000, "BodyJetpack", 2),
				TutorialStep("So the more you carry, the less you can jump", 8000, "BodyJetpack", 2)
			}
		},
		{
			-- Obstacle course
			triggerBox = Box(Vector(915, 492), 395, 167),
			screenPos = Vector(961, 592),
			textOffset = textBelowScreen,
			room = 3,
			steps = {
				TutorialStep("Obstacle course", 2000, "BodyHop", 2),
				TutorialStep("Climb obstacles by holding [" .. MappingName(InputElements.INPUT_L_RIGHT) .. "]", 4000, "ArrowRight", 2),
				TutorialStep("Even climb ladders by simply moving toward them", 8000, "BodyClimb", 2),
				TutorialStep("You can also go prone and crawl with [" .. proneName .. "]", 6000, "BodyCrawl", 2),
				TutorialStep("In tight spaces, you may need to angle your head down to get through", 8000, "BodyCrawl", 2)
			}
		},
		{
			-- Firing range
			triggerBox = Box(Vector(913, 394), 389, 97),
			screenPos = Vector(961, 424),
			textOffset = textBelowScreen,
			room = 4,
			steps = {
				TutorialStep("Firing range", 2000, "FireTarget", 1),
				TutorialStep("Pick up the weapon by first standing over it", 4000, "ArrowRight", 2),
				TutorialStep("And then hold down [" .. pieName .. "]", 4000, "MenuPickUp", 1, 500),
				TutorialStep("Point up to 'Pick Up'", 4000, "MenuPickUp", 2, 500),
				TutorialStep("Release [" .. pieName .. "] to complete the command", 4000, "MenuPickUp", 1),
				TutorialStep("If you aim continuously toward your target", 4000, "BodyAim", 3),
				TutorialStep("It improves your accuracy and view distance", 4000, "BodyAim", 3),
				TutorialStep("Use [" .. fireName .. "] to Fire!", 4000, "BodyFire", 2),
				TutorialStep("Reload manually with [" .. pieName .. "] + up", 8000, "BodyFire", 2)
			}
		},
		{
			-- Rooftop
			triggerBox = Box(Vector(732, 176), 356, 210),
			screenPos = Vector(961, 316),
			textOffset = textAboveScreen,
			steps = {
				TutorialStep("Pick up the digging tool", 4000, "MenuPickUp", 2, 500),
				TutorialStep("Use it on the dirt here", 8000, "DigPile", 2, 750),
				TutorialStep("If you dig up gold, it is added to your team's funds", 4000, "Funds", 2, 250),
				TutorialStep("Funds can be spent in the Buy Menu", 4000, "Funds", 1, 333),
				TutorialStep("Which is opened through the Command Menu", 4000, "MenuBuyMenu", 1, 500),
				TutorialStep("Hold [" .. pieName .. "] and point up and left to 'Buy Menu'", 6000, "MenuBuyMenu", 2, 500),
				TutorialStep("The Buy Menu works like a shopping cart", 6000, "BuyMenuCargo", 1, 500),
				TutorialStep("Add to the Cargo list the items you want delivered", 6000, "BuyMenuCargo", 2, 500),
				TutorialStep("Then use the BUY button, or click outside the menu", 4000, "BuyMenuBuy", 2, 500),
				TutorialStep("Finally select a flat area where you want the goods delivered", 8000, "BuyMenuBuy", 1, 500),
				TutorialStep("Next, you can go explore to the west to try flying and climbing in the wild", 6000, "ArrowLeft", 2),
				TutorialStep("Or, go to the east to learn about squads!", 8000, "ArrowRight", 2)
			}
		},
		{
			-- Roof east
			triggerBox = Box(Vector(1100, 176), 200, 210),
			screenPos = Vector(1201, 316),
			textOffset = textAboveScreen,
			steps = {
				TutorialStep("Hold [" .. pieName .. "] and point down and right to 'Form Squad'", 4000, "MenuTeam", 2, 500),
				TutorialStep("Adjust selection circle to select nearby bodies", 4000, "TeamSelect", 4, 500),
				TutorialStep("All selected units will follow you, and engage on their own", 4000, "TeamFollow", 2, 500),
				TutorialStep("Units with similar weapons will fire in unison with the leader", 4000, "TeamFollow", 2, 500),
				TutorialStep("Hold [" .. pieName .. "] and point down and right again to disband squad", 4000, "MenuTeam", 2, 500),
				TutorialStep("Next, you can head east for a TRIAL BATTLE!", 8000, "ArrowRight", 2)
			}
		}
	};
end
