function Create(self)
	local activity = ActivityMan:GetActivity();
	
	print("Refinery Ambience Controller initialized")
	
	self.playerIndoornesses = {};
	self.playerExtContainers = {};
	self.playerIntContainers = {};
	
	self.playerIntOneShotContainers = {};
	
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if activity:PlayerActive(player) and activity:PlayerHuman(player) then
			self.playerIndoornesses[player] = 0;
			self.playerExtContainers[player] = CreateSoundContainer("Yskely Refinery Ambience Ext", "Browncoats.rte");
			self.playerIntContainers[player] = CreateSoundContainer("Yskely Refinery Ambience Int", "Browncoats.rte");
			
			self.playerIntOneShotContainers[player] = CreateSoundContainer("Yskely Refinery Ambience Int OneShot", "Browncoats.rte");
			
			self.playerExtContainers[player].Volume = 1;
			self.playerExtContainers[player]:Play(player);
			
			self.playerIntContainers[player].Volume = 0;
			self.playerIntContainers[player]:Play(player);
			
			self.playerIntOneShotContainers[player].Volume = 0;
		end
	end
	
	self.ambienceTimer = Timer();
	self.ambienceDelay = 2500;
	
	self.oneShotTimer = Timer();
	self.oneShotDelay = math.random(20000, 60000);
end

function ThreadedUpdate(self)
	self.ToSettle = false;
	self.ToDelete = false;

	for player, indoorness in pairs(self.playerIndoornesses) do
		local cursorPos = CameraMan:GetScrollTarget(player)
		
		if SceneMan.Scene:WithinArea("IndoorArea", cursorPos) then		
			self.playerIndoornesses[player] = math.min(1, indoorness + TimerMan.DeltaTimeSecs * 0.5);			
		else			
			self.playerIndoornesses[player] = math.max(0, indoorness - TimerMan.DeltaTimeSecs * 0.5);			
		end	
		
		-- it seems that allowing sounds to go to Volume 0 will disable them, and create gaps in
		-- our audio that we don't want.
		self.playerExtContainers[player].Volume = math.max(0.01, 1 - self.playerIndoornesses[player]);
		self.playerIntContainers[player].Volume = math.max(0.01, self.playerIndoornesses[player]);
		self.playerIntOneShotContainers[player].Volume = math.max(0.01, self.playerIndoornesses[player]);
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
	
	if self.oneShotTimer:IsPastRealMS(self.oneShotDelay) then
		self.oneShotTimer:Reset();
		self.oneShotDelay = math.random(20000, 60000);
		
		for player, container in pairs(self.playerIntOneShotContainers) do
			container:Play(player);
			container.CustomPanValue = math.random(-100, 100)/100;
		end
	end
end

function Destroy(self)
	print("Refinery Ambience Controller was destroyed! This should never happen.");
end