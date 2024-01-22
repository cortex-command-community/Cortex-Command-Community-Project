#pragma once

// This restores features removed from the C++17 standard (auto_ptr and some other stuff). This is needed for LuaBind to work because it relies on it heavily.
#define _HAS_AUTO_PTR_ETC 1
#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

// Inclusion of relevant C++ Standard Library headers.
#include <cstdlib>
#include <cstdarg>
#include <cstddef>
#include <chrono>
#include <functional>
#include <thread>
#include <mutex>
#include <cctype>
#include <string>
#include <cstring>
#include <vector>
#include <stack>
#include <list>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <deque>
#include <regex>
#include <iostream>
#include <sstream>
#include <fstream>
#include <istream>
#include <algorithm>
#include <cmath>
#include <ctime>
#include <cerrno>
#include <cassert>
#include <limits>
#include <random>
#include <array>
#include <filesystem>
#include <atomic>
#include <execution>
#include <source_location>
#include <regex>
#include <future>

// Include our chunkiest files.
// "chunkiest" is a term from MyNameIsTrez's repository
// that is calculated to be the number of times a header is included,
// times the number of headers it itself (indirectly) includes.
// On MyNameIsTrez's computer, it brought total recompilation time down from 2m53s to 2m37s.
// https://github.com/MyNameIsTrez/includes_tabulator
#include "GUI.h"
#include "MOSprite.h"
#include "MovableObject.h"
#include "SceneMan.h"
#include "MOSRotating.h"
#include "Actor.h"
#include "Activity.h"
#include "SoundContainer.h"
#include "ActivityMan.h"
#include "AudioMan.h"
#include "GUISound.h"
#include "SpatialPartitionGrid.h"
#include "MovableMan.h"
#include "Vector.h"
#include "Attachable.h"
#include "Atom.h"
#include "GameActivity.h"
#include "Controller.h"
#include "AHuman.h"
#include "Scene.h"
#include "Entity.h"
#include "LimbPath.h"
#include "Leg.h"

#include "Hash.h"
