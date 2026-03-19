#pragma once

// This restores features removed from the C++17 standard (auto_ptr and some other stuff). This is needed for LuaBind to work because it relies on it heavily.
#define _HAS_AUTO_PTR_ETC 1
#define _LIBCPP_ENABLE_CXX17_REMOVED_AUTO_PTR

// without this, BS threadpool can crash on launch in some builds
#define _DISABLE_CONSTEXPR_MUTEX_CONSTRUCTOR

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
// POSIX / Emscripten: stricmp is not standard, use strcasecmp
#ifdef __EMSCRIPTEN__
#include <strings.h>
#ifndef stricmp
#define stricmp strcasecmp
#endif
#ifndef strnicmp
#define strnicmp strncasecmp
#endif
#endif

#ifndef __EMSCRIPTEN__
#include <execution>
#else
// Emscripten doesn't support std::execution parallel policies.
// Redirect parallel for_each/transform/sort etc. to sequential execution.
#include <algorithm>
namespace std {
    namespace execution {
        struct _EmExecPolicy {};
        struct sequenced_policy            : _EmExecPolicy {};
        struct parallel_policy             : _EmExecPolicy {};
        struct parallel_unsequenced_policy : _EmExecPolicy {};
        struct unsequenced_policy          : _EmExecPolicy {};
        inline constexpr sequenced_policy              seq{};
        inline constexpr parallel_policy               par{};
        inline constexpr parallel_unsequenced_policy   par_unseq{};
        inline constexpr unsequenced_policy            unseq{};
    }
    // Drop the execution policy argument and run sequentially
    template<class FwdIt, class Fn>
    void for_each(execution::_EmExecPolicy, FwdIt first, FwdIt last, Fn fn) {
        for (; first != last; ++first) fn(*first);
    }
    template<class FwdIt1, class FwdIt2, class Fn>
    FwdIt2 transform(execution::_EmExecPolicy, FwdIt1 first, FwdIt1 last, FwdIt2 dst, Fn fn) {
        return std::transform(first, last, dst, fn);
    }
    template<class FwdIt>
    void sort(execution::_EmExecPolicy, FwdIt first, FwdIt last) {
        std::sort(first, last);
    }
    template<class FwdIt, class Cmp>
    void sort(execution::_EmExecPolicy, FwdIt first, FwdIt last, Cmp cmp) {
        std::sort(first, last, cmp);
    }
}
#endif
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
