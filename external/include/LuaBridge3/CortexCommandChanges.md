# LuaBridge3 in Cortex Command

This is [LuaBridge3](https://github.com/kunitoki/LuaBridge3) at commit `64b3ec3d265578c742abd5d3939b133353952718` (version 3.1 development, October 2026), with only the `Source/LuaBridge` folder kept.

It replaced luabind 0.7.1. Most of what makes the bindings behave the way scripts written for luabind expect lives in `Source/Lua/LuaBridgeDefinitions.h` and `Source/Lua/LuaBridgeRegistry.cpp` rather than in here, and LuaBridge itself is changed as little as possible, so it can be updated without much trouble.

## Changes made to LuaBridge

Every change is marked with a `Cortex Command` comment, and `CortexCommandChanges.patch` has all of them in one place. Any other change made to LuaBridge needs adding to it too.

- `detail/Config.h`: added the `LUABRIDGE_LUABIND_COMPATIBLE_NUMBERS` option, off by default. The game turns it on in `LuaBridgeDefinitions.h`.
- `detail/Stack.h`: with `LUABRIDGE_LUABIND_COMPATIBLE_NUMBERS` on, the integer `Stack` specializations truncate any Lua number towards zero like luabind did, instead of rejecting numbers that have a fractional part or don't fit, and the floating point ones don't range check. Scripts pass things like `self.Team + 0.5` and `math.floor` results that ended up as floats all over the place.
- `detail/CFunctions.h`: calling a method on `nil` (most often by using `.` instead of `:`) raises a Lua error, like it did with luabind, instead of calling the member function through a null pointer and crashing the game.
- `detail/CFunctions.h`: `invoke_member_function`, `invoke_const_member_function` and `invoke_proxy_functor` accept the function they call in a light userdata as well as in a full userdata. The game keeps what its bindings call outside the Lua heap and hands Lua light userdata pointing to it, since the garbage collector visits every full userdata on every cycle, and there would be thousands of them.
- `detail/Userdata.h`: added `AncestryInfo`, a list of every class an object's class can be cast to with the pointer offsets to them, which `Userdata::get` uses when an object's metatable has one. LuaBridge otherwise goes up the class hierarchy one parent list and cast table at a time, which made using an object as one of its base classes, like calling an `Entity` method on an `AHuman`, several times slower than with luabind. The game puts these in the metatables of every class it registers. Like LuaBridge's own keys, the key it's stored under is a fixed value rather than an address, which the simulation's determinism depends on (see `RawGetP` in `Source/Lua/LuaBridgeDefinitions.h`).

## Updating

Copy over `Source/LuaBridge` from the new version and commit it on its own, then apply the changes again by running `git apply --3way external/include/LuaBridge3/CortexCommandChanges.patch` from the root of the repository. Afterwards, make the patch again with `git diff` against that commit, keeping the header at its top, and update the upstream commit given here and in the patch.
