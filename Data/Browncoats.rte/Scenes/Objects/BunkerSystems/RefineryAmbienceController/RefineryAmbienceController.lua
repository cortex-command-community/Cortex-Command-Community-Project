function Create(self)

	local activity = ActivityMan:GetActivity();
	
	self.playerIndoornesses = {};
	self.playerExtContainers = {};
	self.playerIntContainers = {};
	
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if activity:PlayerActive(player) and activity:PlayerHuman(player) then
			self.playerIndoornesses[player] = 0;
			self.playerExtContainers[player] = CreateSoundContainer("Yskely Refinery Ambience Ext", "Browncoats.rte");
			self.playerIntContainers[player] = CreateSoundContainer("Yskely Refinery Ambience Int", "Browncoats.rte");
			
			self.playerExtContainers[player].Volume = 1;
			self.playerExtContainers[player]:Play(player);
			
			self.playerIntContainers[player].Volume = 0;
			self.playerIntContainers[player]:Play(player);
			
		end
	end
	
	self.ambienceTimer = Timer();
	self.ambienceDelay = 2500;

end

function ThreadedUpdate(self)

	for player, indoorness in pairs(self.playerIndoornesses) do
	
		local cursorPos = CameraMan:GetScrollTarget(player)
		
		if SceneMan.Scene:WithinArea("Indoor Area", cursorPos) then		
			self.playerIndoornesses[player] = math.min(1, indoorness + TimerMan.DeltaTimeSecs * 0.5);			
		else			
			self.playerIndoornesses[player] = math.max(0, indoorness - TimerMan.DeltaTimeSecs * 0.5);			
		end	
		
		-- it seems that allowing sounds to go to Volume 0 will disable them, and create gaps in
		-- our audio that we don't want.
		self.playerExtContainers[player].Volume = math.max(0.01, 1 - self.playerIndoornesses[player]);
		self.playerIntContainers[player].Volume = math.max(0.01, self.playerIndoornesses[player]);
		
	end
	
	if self.ambienceTimer:IsPastRealMS(self.ambienceDelay) then
		self.ambienceTimer:Reset();
		
		for player, container in pairs(self.playerExtContainers) do
			container:Play(player);
		end
		
		for player, container in pairs(self.playerIntContainers) do
			container:Play(player);
		end
		
	end

end