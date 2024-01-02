SecretCodeEntry = {};
dofile("Base.rte/Scripts/Shared/SecretCodeEntry/SecretCodes.pem")
SecretCodeEntry.allowedControlStates = { Controller.PRESS_PRIMARY, Controller.PRESS_SECONDARY, Controller.PRESS_RIGHT, Controller.PRESS_LEFT, Controller.PRESS_UP, Controller.PRESS_DOWN };
SecretCodeEntry.defaultFirstEntrySoundContainer = CreateSoundContainer("Funds Changed", "Base.rte");
SecretCodeEntry.defaultFirstEntrySoundContainer.Volume = 4;
SecretCodeEntry.defaultCorrectEntrySoundContainer = CreateSoundContainer("Funds Changed", "Base.rte");
SecretCodeEntry.defaultCorrectEntrySoundContainer.Volume = 2;
SecretCodeEntry.defaultIncorrectEntrySoundContainer = CreateSoundContainer("Error", "Base.rte");
SecretCodeEntry.defaultCodeCompletionSoundContainer = CreateSoundContainer("Funds Changed", "Base.rte");
SecretCodeEntry.defaultCodeCompletionSoundContainer.Volume = 4;
SecretCodeEntry.data = {};

function SecretCodeEntry.Setup(callbackFunction, callbackSelfObject, codeSequenceOrCodeType, firstEntrySoundContainer, correctEntrySoundContainer, incorrectEntrySoundContainer, codeCompletionSoundContainer)
	local activity = ActivityMan:GetActivity();
	if activity == nil then
		print("Secret Code Entry Error: You need to start an Activity before setting up code entry!");
		return;
	end
	
	if callbackFunction == nil then
		print("Secret Code Entry Error: You need to specify the callback function to run upon successful code entry!");
		return;
	end
	
	if codeSequenceOrCodeType == nil then
		print("Secret Code Entry Error: You need to specify a code sequence or code type!");
		return;
	end
	
	local sequenceLength = 0;
	if type(codeSequenceOrCodeType) == "number" then
		local numberOfCodes = SecretCodeEntry.GetNumberOfCodes();
		if codeSequenceOrCodeType < 1 or codeSequenceOrCodeType > numberOfCodes then
			print("Secret Code Entry Error: Code type must be between 1 and " .. tostring(numberOfCodes));
			return;
		end
		sequenceLength = SecretCodeEntry.GetCodeSequenceLength(codeSequenceOrCodeType);
	elseif type(codeSequenceOrCodeType) == "table" then
		for _, codeSequenceControlState in pairs(codeSequenceOrCodeType) do
			local isAllowedControlState = false;
			for _, allowedControlState in pairs(SecretCodeEntry.allowedControlStates) do
				if codeSequenceControlState == allowedControlState then
					isAllowedControlState = true;
					break;
				end
			end
			
			if not isAllowedControlState then
				print("Secret Code Entry Error: Only the following Controller control states are supported: " .. table.concat(SecretCodeEntry.allowedControlStates, ", "));
				return;
			end
			
			sequenceLength = #codeSequenceOrCodeType;
		end
	end
	
	local secretCodeEntryData = {}
	secretCodeEntryData.callbackFunction = callbackFunction;
	secretCodeEntryData.callbackSelfObject = callbackSelfObject;
	secretCodeEntryData.codeSequenceOrCodeType = codeSequenceOrCodeType;
	secretCodeEntryData.sequenceLength = sequenceLength;
	secretCodeEntryData.firstEntrySoundContainer = firstEntrySoundContainer or SecretCodeEntry.defaultFirstEntrySoundContainer;
	secretCodeEntryData.correctEntrySoundContainer = correctEntrySoundContainer or SecretCodeEntry.defaultCorrectEntrySoundContainer;
	secretCodeEntryData.incorrectEntrySoundContainer = incorrectEntrySoundContainer or SecretCodeEntry.defaultIncorrectEntrySoundContainer;
	secretCodeEntryData.codeCompletionSoundContainer = codeCompletionSoundContainer or SecretCodeEntry.defaultCodeCompletionSoundContainer;
	
	secretCodeEntryData.inputs = {};
	for player = Activity.PLAYER_1, Activity.MAXPLAYERCOUNT - 1 do
		if activity:PlayerActive(player) and activity:PlayerHuman(player) then
			secretCodeEntryData.inputs[player] = { controller = ActivityMan:GetActivity():GetPlayerController(player), currentStep = 0 };
		end
	end
	
	SecretCodeEntry.data[#SecretCodeEntry.data + 1] = secretCodeEntryData;
	
	return #SecretCodeEntry.data;
end

function SecretCodeEntry.IsValid(secretCodeEntryDataIndex)
	return secretCodeEntryDataIndex ~= nil and secretCodeEntryDataIndex > 0 and secretCodeEntryDataIndex <= #SecretCodeEntry.data;
end

function SecretCodeEntry.Update(secretCodeEntryDataIndex)
	if secretCodeEntryDataIndex == nil or secretCodeEntryDataIndex < 0 or secretCodeEntryDataIndex > #SecretCodeEntry.data then
		print("Secret Code Entry Error: The secret code entry index must be between 1 and " .. tostring(#SecretCodeEntry.data) .. ". This index was returned by SecretCodeEntry.Setup.");
		return;
	end
	
	local secretCodeEntryData = SecretCodeEntry.data[secretCodeEntryDataIndex];
	local playersWhoCompletedCode = {};
	
	if UInputMan:AnyPress() then
		for player, inputData in pairs(secretCodeEntryData.inputs) do
			local expectedNextControlState;
			if type(secretCodeEntryData.codeSequenceOrCodeType) == "number" then
				expectedNextControlState = SecretCodeEntry.GetExpectedNextControlState(secretCodeEntryData.codeSequenceOrCodeType, inputData.currentStep);
			else
				expectedNextControlState = secretCodeEntryData.codeSequence[inputData.currentStep + 1];
			end
			
			local correctInputPressed = inputData.controller:IsState(expectedNextControlState);
			local incorrectInputPressed = false;
			if inputData.currentStep > 0 then
				for _, controlState in ipairs(SecretCodeEntry.allowedControlStates) do
					if inputData.controller:IsState(controlState) and controlState ~= expectedNextControlState then
						incorrectInputPressed = true;
						break;
					end
				end
			end
			
			local soundToPlay;
			if incorrectInputPressed then
				soundToPlay = secretCodeEntryData.incorrectEntrySoundContainer;
				inputData.currentStep = 0;
			elseif correctInputPressed then
				soundToPlay = inputData.currentStep == 0 and secretCodeEntryData.firstEntrySoundContainer or (inputData.currentStep + 1 == secretCodeEntryData.sequenceLength and secretCodeEntryData.codeCompletionSoundContainer or secretCodeEntryData.correctEntrySoundContainer);
				inputData.currentStep = inputData.currentStep + 1;
			end
			if soundToPlay then
				soundToPlay:Play(CameraMan:GetScrollTarget(inputData.controller.Player), inputData.controller.Player);
			end
			
			if inputData.currentStep == secretCodeEntryData.sequenceLength then
				secretCodeEntryData.inputs[player] = nil;
				playersWhoCompletedCode[#playersWhoCompletedCode + 1] = player;
			end
		end
	end
	
	if #playersWhoCompletedCode > 0 then
		secretCodeEntryData.callbackFunction(secretCodeEntryData.callbackSelfObject, playersWhoCompletedCode);
	end
end