RefineryAssaultDynamicMusicControllerFunctions = {};

function RefineryAssaultDynamicMusicControllerFunctions.checkForPreferencePossibility(self, prefersTable, possibilityTable)

	local resultsTable = {};
	local foundAny = false;
	-- possibly inefficient?
	for k, v in pairs(possibilityTable) do
		for pKey, pValue in pairs(prefersTable) do
			if v == pValue then
				foundAny = true;
				table.insert(resultsTable, v);
			end
		end
	end
	if foundAny then
		--print("foundpreferredtoo")
		return resultsTable;
	else
		return false;
	end
	
end

function RefineryAssaultDynamicMusicControllerFunctions.selectPossibleLoops(self, loopTable)

	-- basically just avoids repeats and any loops the currentloop says to Never play

	local resultIndex
	local loopSelectedTable = {};
	for k, v in pairs(loopTable) do
		local valid = true;
		if v ~= self.currentIndex then
			if self.currentTune.Components[self.currentIndex].Never then
				for k, neverV in pairs(self.currentTune.Components[self.currentIndex].Never) do
					if v == neverV then
						valid = false;
						break;
					end
				end
			end
			if valid == true then
				table.insert(loopSelectedTable, v);
			end
		end
	end
	resultIndex = loopSelectedTable[math.random(1, #loopSelectedTable)];
	
	-- if there's any compatibility between our final curated selection and loop preferences,
	-- 80% chance to pick one (not always so things are at least a little random)
	
	if math.random(0, 100) < 80 and self.currentTune.Components[self.currentIndex].Prefers ~= nil then
		local prefersTable = self.currentTune.Components[self.currentIndex].Prefers
		local resultsTable = RefineryAssaultDynamicMusicControllerFunctions.checkForPreferencePossibility(self, prefersTable, loopSelectedTable)
		if resultsTable then
			resultIndex = prefersTable[math.random(1, #prefersTable)];
		end
	end
	
	return resultIndex

end

function RefineryAssaultDynamicMusicController:StartScript()
	self.activity = ActivityMan:GetActivity();
	
	AudioMan:ClearMusicQueue();
	AudioMan:StopMusic();
	
	self.actorTable = {};
	
	for actor in MovableMan.AddedActors do
		table.insert(self.actorTable, actor);
	end
	
	-- our dynamic music is just normal sound, so get the current ratio between sound and music volume
	-- to set the container volumes
	
	self.dynamicVolume = (AudioMan.MusicVolume / AudioMan.SoundsVolume);
	
	self.MUSIC_STATE = "Main";
	
	self.componentTimer = Timer();
	self.restTimer = Timer();
	
	self.loopNumber = 0;
	self.totalLoopNumber = 0;
	self.tuneMaxLoops = 9999;
	
	-- unfortunately the real intensities we have to set are as follows
	-- 1: ambient
	-- 3: light
	-- 6: heavy
	-- 8: extreme
	-- this is due to ease of selecting transitions and comedowns later in code
	-- i should probably change these to an enum of some sort, but i dont know how
	self.desiredIntensity = 1;
	self.Intensity = 1;
	

	self.Tunes = {};	
	
	-- note that ambients can really be upgraded at any time not just according to postExit
	
	self.Tunes.browncoatTrack = {};
	
	local indexAutomator = 1;
	
	self.Tunes.browncoatTrack.Components = {};
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Intro 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 2687;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 35000;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Ambient Transition 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 5324;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 47833;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Transition";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Ambient Main 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 2687;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 44901;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Ambient Main 2", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 2687;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 44901;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Ambient Main 3", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 2687;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 44901;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Light Transition 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 2687;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 44901;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Transition";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Light Main 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 5337;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 26536;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Light Main 2", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 2687;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 26743;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Heavy Transition 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 2687;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 44901;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Transition";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Heavy Main 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 5337;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 47541;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Heavy Main 2", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 2687;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 23784;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Boss Transition 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 5524;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 26615;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Transition";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Boss Main 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 5077;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 24853;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Boss Main 2", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 4993;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 24807;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";

	indexAutomator = indexAutomator + 1;
	
	self.Tunes.browncoatTrack.Components[indexAutomator] = {};
	self.Tunes.browncoatTrack.Components[indexAutomator].Container = CreateSoundContainer("BCOSTTest Outro 1", "Browncoats.rte");
	self.Tunes.browncoatTrack.Components[indexAutomator].preEntry = 189;
	self.Tunes.browncoatTrack.Components[indexAutomator].postExit = 49242;
	self.Tunes.browncoatTrack.Components[indexAutomator].Type = "Main";
	
	-- intro : 0
	-- ambient transition: 1
	-- ambient: 2
	-- light transition: 3
	-- light main: 4
	-- heavy transition: 5
	-- heavy main: 6
	-- boss transition: 7
	-- boss: 8
	-- outro: 9
	
	self.Tunes.browncoatTrack.typeTables = {};
	
	self.Tunes.browncoatTrack.typeTables[0] = {};
	self.Tunes.browncoatTrack.typeTables[0].Loops = {1};	
	
	self.Tunes.browncoatTrack.typeTables[1] = {};
	self.Tunes.browncoatTrack.typeTables[1].Loops = {2};
	
	self.Tunes.browncoatTrack.typeTables[2] = {};
	self.Tunes.browncoatTrack.typeTables[2].Loops = {2, 3, 4, 5};
	
	self.Tunes.browncoatTrack.typeTables[3] = {};
	self.Tunes.browncoatTrack.typeTables[3].Loops = {6};
	
	self.Tunes.browncoatTrack.typeTables[4] = {};
	self.Tunes.browncoatTrack.typeTables[4].Loops = {6, 7, 8};
	
	self.Tunes.browncoatTrack.typeTables[5] = {};
	self.Tunes.browncoatTrack.typeTables[5].Loops = {9};
	
	self.Tunes.browncoatTrack.typeTables[6] = {};
	self.Tunes.browncoatTrack.typeTables[6].Loops = {9, 10, 11};
	
	self.Tunes.browncoatTrack.typeTables[7] = {};
	self.Tunes.browncoatTrack.typeTables[7].Loops = {12};
	
	self.Tunes.browncoatTrack.typeTables[8] = {};
	self.Tunes.browncoatTrack.typeTables[8].Loops = {13, 14};
	
	self.Tunes.browncoatTrack.typeTables[9] = {};
	self.Tunes.browncoatTrack.typeTables[9].Loops = {15};

	
	self.currentIndex = 1;
	local tuneTable = {};
	for k, v in pairs(self.Tunes) do
		table.insert(tuneTable, v);
	end
	self.currentTuneIndex = math.random(1, #tuneTable);
	self.currentTune = tuneTable[self.currentTuneIndex];
	if self.currentTune.Components then
		self.dynamicMusic = true;
		self.currentIndex = math.random(1, #self.currentTune.typeTables[0].Loops)
		self.currentTune.Components[self.currentIndex].Container.Volume = self.dynamicVolume;
		self.currentTune.Components[self.currentIndex].Container:Play();
		--print(self.currentTune.Components[self.currentIndex].Container)
		self.MUSIC_STATE = "Intro";
		self.desiredIntensity = 2;
		self.Intensity = 0;
	end
	
end

function RefineryAssaultDynamicMusicController:UpdateScript()

	-- DEBUG
	if UInputMan:KeyPressed(Key.KP_1) then
		self.desiredIntensity = 2;
		print("debug set desired: " .. self.desiredIntensity);
	elseif UInputMan:KeyPressed(Key.KP_2) then
		self.desiredIntensity = 4;
		print("debug set desired: " .. self.desiredIntensity);
	elseif UInputMan:KeyPressed(Key.KP_3) then
		self.desiredIntensity = 6;
		print("debug set desired: " .. self.desiredIntensity);
	elseif UInputMan:KeyPressed(Key.KP_4) then
		self.desiredIntensity = 8;
		print("debug set desired: " .. self.desiredIntensity);
	elseif UInputMan:KeyPressed(Key.KP_5) then
		self.desiredIntensity = 9;
		print("debug set desired: " .. self.desiredIntensity);
	
	end
	
	
	if self.dynamicMusic == true then
	
		--print("stillrunning")
		
		AudioMan:ClearMusicQueue();
		AudioMan:StopMusic();
		
		-- "play stuff right the hell now" logic first, the else is the normal loop logic
	
		if (self.MUSIC_STAETE == "Main" and self.Intensity < 3 and self.desiredIntensity ~= 1) then
		
			-- if we're in ambience and anything, at all, is going on, immediately upgrade
			
			local loopTable
			local index
			
			-- set intensity early here, we wanna use transitions of the resulting intensity
			self.Intensity = self.desiredIntensity

			loopTable = self.currentTune.typeTables[self.Intensity - 1].Loops;
			
			if #loopTable ~= 0 then
				index = RefineryAssaultDynamicMusicControllerFunctions.selectPossibleLoops(self, loopTable);
				self.MUSIC_STATE = "Transition";
			else
				-- if we lack a transition go right into the Main instead
				self.MUSIC_STATE = "Main";
				loopTable = self.currentTune.typeTables[self.Intensity].Loops;
				index = RefineryAssaultDynamicMusicControllerFunctions.selectPossibleLoops(self, loopTable);
			end
			
			-- finally set our real intensity for real
			
			self.nextDecided = false;
		
			self.loopNumber = self.loopNumber + 1;
			
			self.totalLoopNumber = self.totalLoopNumber + 1;
			
			local oldIndex = self.currentIndex + 0;
			self.oldSoundContainer = self.currentTune.Components[oldIndex].Container
			
			self.currentIndex = index
			
			-- -- fade the ambience/comedown out by the preEntry of what we're about to play
			-- -- maybe a bit messy? just stop it when the new thing plays proper?
			-- note: it was a bit messy and i just did the second thing
			-- self.currentTune.Components[oldIndex].Container:FadeOut(self.currentTune.Components[self.currentIndex].preEntry);
			
			self.dynamicVolume = (AudioMan.MusicVolume / AudioMan.SoundsVolume);
			
			self.currentTune.Components[self.currentIndex].Container.Volume = self.dynamicVolume;
			self.currentTune.Components[self.currentIndex].Container:Play();
			
			--print(self.currentTune.Components[self.currentIndex].Container)
			
			self.componentTimer:Reset();
			
		else
			if self.componentTimer:IsPastRealMS(self.currentTune.Components[self.currentIndex].postExit/3) then
				-- a third thru current loop, decide what to play next
				if self.nextDecided ~= true then
					self.nextDecided = true;	
					
					print("intensity:")
					print(self.Intensity)
					print("desired:")
					print(self.desiredIntensity)
					
					local index
					local loopTable
					
					if self.MUSIC_STATE == "Transition" then

						loopTable = self.currentTune.typeTables[self.Intensity].Loops;
						index = RefineryAssaultDynamicMusicControllerFunctions.selectPossibleLoops(self, loopTable);
						
						self.MUSIC_STATE = "Main";

					else
						
						loopTable = self.currentTune.typeTables[self.Intensity].Loops;
						
						index = RefineryAssaultDynamicMusicControllerFunctions.selectPossibleLoops(self, loopTable);
					
						if (self.desiredIntensity ~= self.Intensity) then					
							
							-- set intensity early here, we wanna use transitions of the resulting intensity
							self.Intensity = self.desiredIntensity
						
							if self.Intensity > 1 then
								-- minus one: transition
								loopTable = self.currentTune.typeTables[self.Intensity - 1].Loops;
							end
							
							if #loopTable ~= 0 then
								index = RefineryAssaultDynamicMusicControllerFunctions.selectPossibleLoops(self, loopTable);
								self.MUSIC_STATE = "Transition";
							else
								-- if we lack a transition go right into the Main instead
								self.MUSIC_STATE = "Main";
								loopTable = self.currentTune.typeTables[self.Intensity].Loops;
								index = RefineryAssaultDynamicMusicControllerFunctions.selectPossibleLoops(self, loopTable);
							end

						end			

					end						
					
					self.indexToPlay = index;
					
				end
						
				local actingpreEntry = self.currentTune.Components[self.indexToPlay].preEntry
			
				if self.componentTimer:IsPastRealMS(self.currentTune.Components[self.currentIndex].postExit - actingpreEntry) then
				
					self.nextDecided = false;
				
					self.loopNumber = self.loopNumber + 1;
					
					self.totalLoopNumber = self.totalLoopNumber + 1;
					--local oldIndex = self.currentIndex + 0
					--self.oldSoundContainer = self.currentTune.Components[oldIndex].Container;
					
					self.currentIndex = self.indexToPlay;
					
					self.dynamicVolume = (AudioMan.MusicVolume / AudioMan.SoundsVolume);
					
					self.currentTune.Components[self.currentIndex].Container.Volume = self.dynamicVolume;
					self.currentTune.Components[self.currentIndex].Container:Play();
					
					print(self.currentTune.Components[self.currentIndex].Container)
					
					self.componentTimer:Reset();
					
				end					

			elseif self.componentTimer:IsPastRealMS(self.currentTune.Components[self.currentIndex].preEntry) then
				if self.oldSoundContainer then
					self.oldSoundContainer:FadeOut(50);
					self.oldSoundContainer = nil;
				end
			end
			
		end
		
	end

end

function RefineryAssaultDynamicMusicController:EndScript()

	self.gameActivity = ToGameActivity(ActivityMan:GetActivity())

	AudioMan:StopMusic();
	AudioMan:ClearMusicQueue();

end

function RefineryAssaultDynamicMusicController:PauseScript()

	if not self.paused then
		self.paused = true;
		self.pauseTime = self.componentTimer.ElapsedRealTimeMS
	else
		self.paused = false;
		self.componentTimer.ElapsedRealTimeMS = self.componentTimer.ElapsedRealTimeMS + (self.pauseTime - self.componentTimer.ElapsedRealTimeMS)
	end
	
	--print("paused?")
end

function RefineryAssaultDynamicMusicController:CraftEnteredOrbit()
end
