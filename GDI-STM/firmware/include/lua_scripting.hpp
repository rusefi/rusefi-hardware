#ifndef LUA_SCRIPTING_HPP
#define LUA_SCRIPTING_HPP

#include <climits>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

class LuaScripting {
public:
    LuaScripting();
    ~LuaScripting();

    bool init();
    bool executeScript(const char* script);
    bool callFunction(const char* funcName, int numArgs = 0, int numResults = 0);

private:
    lua_State* L;

    void registerFunctions();

    // Lua function implementations
    static int luaFireInjector(lua_State* L);
    static int luaStopInjector(lua_State* L);
    static int luaSetInjectorTiming(lua_State* L);
    static int luaFireAllInjectors(lua_State* L);
    static int luaStopAllInjectors(lua_State* L);
    static int luaCanSend(lua_State* L);
    static int luaCanReceive(lua_State* L);
    static int luaDelay(lua_State* L);
    static int luaGetTime(lua_State* L);
};

#endif // LUA_SCRIPTING_HPP
