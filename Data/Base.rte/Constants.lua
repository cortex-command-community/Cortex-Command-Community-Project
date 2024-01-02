--Misc global constants. Add whatever you think makes sense across multiple scripts. All should be under the rte table.
rte = {};

rte.NoMOID = 255;
rte.SpawnIntervalScale = 1.0;
rte.StartingFundsScale = 1.0;
rte.DiggersRate = 0.4;
rte.MetabaseArea = "MetabaseServiceArea";
rte.PxTravelledPerFrame = GetPPM() * TimerMan.DeltaTimeSecs;

--Materials
rte.airID = 0;
rte.goldID = 2;
rte.grassID = 128;
rte.doorID = 181;