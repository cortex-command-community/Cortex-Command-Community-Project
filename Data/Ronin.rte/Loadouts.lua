--The following is a list of loadout-specific weapon sets that are drawn at random when deploying a Ronin from Loadouts
--Primary, Secondary and Tertiary weapons are always of class HDFirearm
RoninLoadouts = {["Rifleman"] = {}, ["Machinegunner"] = {}, ["CQB"] = {}, ["Grenadier"] = {}, ["Sniper"] = {}, ["Engineer"] = {}, ["Medic"] = {}};

RoninLoadouts["Rifleman"]["Primary"] = {"AK-47", "M16A2"};
RoninLoadouts["Rifleman"]["Secondary"] = {".357 Magnum", "Desert Eagle"};
RoninLoadouts["Rifleman"]["Tertiary"] = {"Luger P08", "Beretta 93R", "Shovel", "Grapple Gun", "Medikit"};
RoninLoadouts["Rifleman"]["Throwable"] = {"M67 Grenade Bandolier", "M24 Potato Masher Bandolier", "Molotov Cocktail Bandolier"};

RoninLoadouts["Machinegunner"]["Headgear"] = {"Soldier Helmet", "Motorcycle Helmet", "Dummy Mask", "Browncoat Mask"};
RoninLoadouts["Machinegunner"]["Primary"] = {"M60", "Stoner 63"};
RoninLoadouts["Machinegunner"]["Secondary"] = {"Luger P08", "Beretta 93R"};
RoninLoadouts["Machinegunner"]["Tertiary"] = {"Luger P08", "Beretta 93R", "Shovel", "Grapple Gun"};

RoninLoadouts["CQB"]["Headgear"] = {"Dummy Mask", "Browncoat Mask"};
RoninLoadouts["CQB"]["Primary"] = {"Model 590", "SPAS 12"};
RoninLoadouts["CQB"]["Secondary"] = {".357 Magnum", "Desert Eagle", "UZI", "MP5K", "Sawed-Off Shotgun", "Chainsaw"};
RoninLoadouts["CQB"]["Tertiary"] = {".357 Magnum", "Desert Eagle", "UZI", "MP5K", "Riot Shield", "Medikit"};
RoninLoadouts["CQB"]["Throwable"] = {"Molotov Cocktail Bandolier", "Scrambler Bandolier"};

RoninLoadouts["Grenadier"]["Headgear"] = {"Soldier Helmet"};
RoninLoadouts["Grenadier"]["Primary"] = {"RPG-7", "M79", "RPC M17"};
RoninLoadouts["Grenadier"]["Secondary"] = {"UZI", "MP5K"};
RoninLoadouts["Grenadier"]["Throwable"] = {"M67 Grenade", "M24 Potato Masher Bandolier"};

RoninLoadouts["Sniper"]["Headgear"] = {"Sniper Hat"};
RoninLoadouts["Sniper"]["Primary"] = {"Kar98k", "M1 Garand"};
RoninLoadouts["Sniper"]["Secondary"] = {"UZI", "MP5K"};
RoninLoadouts["Sniper"]["Tertiary"] = {"Medikit", "Light Digger"};
RoninLoadouts["Sniper"]["Throwable"] = {"Empty Bottle Bandolier"};

RoninLoadouts["Engineer"]["Headgear"] = {"Soldier Helmet"};
RoninLoadouts["Engineer"]["Primary"] = {"UZI", "MP5K"};
RoninLoadouts["Engineer"]["Secondary"] = {"Medium Digger"};
RoninLoadouts["Engineer"]["Throwable"] = {"Remote Explosive", "Remote Explosive", "Scrambler Bandolier"};

RoninLoadouts["Medic"]["Headgear"] = {"Soldier Helmet"};
RoninLoadouts["Medic"]["Primary"] = {"UZI", "MP5K"};
RoninLoadouts["Medic"]["Secondary"] = {"Medical Dart Gun"};
RoninLoadouts["Medic"]["Tertiary"] = {"Medikit"};