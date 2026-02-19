#include "lua_scripting.hpp"
#include "injector_control.hpp"
#include "can_driver.hpp"
#include "FreeRTOS.h"
#include "task.h"
#include <cstring>

extern InjectorControl injectorControl;
extern CanDriver canDriver;

LuaScripting::LuaScripting() : L(nullptr) {}

LuaScripting::~LuaScripting() {
    if (L) {
        lua_close(L);
    }
}

bool LuaScripting::init() {
    L = luaL_newstate();
    if (!L) {
        return false;
    }

    // Load only the libraries we need — no io, os, package, debug, or coroutine.
    luaL_requiref(L, "_G", luaopen_base, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "math", luaopen_math, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "string", luaopen_string, 1);
    lua_pop(L, 1);
    luaL_requiref(L, "table", luaopen_table, 1);
    lua_pop(L, 1);

    registerFunctions();

    return true;
}

void LuaScripting::registerFunctions() {
    // Register injector control functions
    lua_register(L, "fire_injector", luaFireInjector);
    lua_register(L, "stop_injector", luaStopInjector);
    lua_register(L, "set_injector_timing", luaSetInjectorTiming);
    lua_register(L, "fire_all_injectors", luaFireAllInjectors);
    lua_register(L, "stop_all_injectors", luaStopAllInjectors);

    // Register CAN functions
    lua_register(L, "can_send", luaCanSend);
    lua_register(L, "can_receive", luaCanReceive);

    // Register utility functions
    lua_register(L, "delay", luaDelay);
    lua_register(L, "get_time", luaGetTime);
}

bool LuaScripting::executeScript(const char* script) {
    if (!L) {
        return false;
    }

    if (luaL_dostring(L, script) != LUA_OK) {
        (void)lua_tostring(L, -1);
        lua_pop(L, 1);
        return false;
    }

    return true;
}

bool LuaScripting::callFunction(const char* funcName, int numArgs, int numResults) {
    if (!L) {
        return false;
    }

    lua_getglobal(L, funcName);
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return false;
    }

    if (lua_pcall(L, numArgs, numResults, 0) != LUA_OK) {
        (void)lua_tostring(L, -1);
        lua_pop(L, 1);
        return false;
    }

    return true;
}

// Lua function implementations
int LuaScripting::luaFireInjector(lua_State* L) {
    int injectorId = static_cast<int>(luaL_checkinteger(L, 1));
    if (injectorId < 0 || injectorId >= 8) {
        return luaL_error(L, "fire_injector: id must be 0..7");
    }
    injectorControl.fireInjector(static_cast<uint8_t>(injectorId));
    return 0;
}

int LuaScripting::luaStopInjector(lua_State* L) {
    int injectorId = static_cast<int>(luaL_checkinteger(L, 1));
    if (injectorId < 0 || injectorId >= 8) {
        return luaL_error(L, "stop_injector: id must be 0..7");
    }
    injectorControl.stopInjector(static_cast<uint8_t>(injectorId));
    return 0;
}

int LuaScripting::luaSetInjectorTiming(lua_State* L) {
    int injectorId = static_cast<int>(luaL_checkinteger(L, 1));
    int onTime = static_cast<int>(luaL_checkinteger(L, 2));
    int offTime = static_cast<int>(luaL_checkinteger(L, 3));
    if (injectorId < 0 || injectorId >= 8) {
        return luaL_error(L, "set_injector_timing: id must be 0..7");
    }
    if (onTime <= 0 || offTime <= 0) {
        return luaL_error(L, "set_injector_timing: on/off times must be > 0");
    }
    injectorControl.setInjectorTiming(static_cast<uint8_t>(injectorId),
                                       static_cast<uint32_t>(onTime),
                                       static_cast<uint32_t>(offTime));
    return 0;
}

int LuaScripting::luaFireAllInjectors(lua_State* L) {
    injectorControl.fireAllInjectors();
    return 0;
}

int LuaScripting::luaStopAllInjectors(lua_State* L) {
    injectorControl.stopAllInjectors();
    return 0;
}

int LuaScripting::luaCanSend(lua_State* L) {
    int id = static_cast<int>(luaL_checkinteger(L, 1));
    if (id < 0 || id > 0x7FF) {
        return luaL_error(L, "can_send: id must be 0..0x7FF");
    }
    const char* data = luaL_checkstring(L, 2);
    size_t dataLen = strlen(data);

    CanMessage msg;
    msg.id = id;
    msg.length = dataLen > 8 ? 8 : dataLen;
    memcpy(msg.data, data, msg.length);

    bool success = canDriver.sendMessage(msg);
    lua_pushboolean(L, success);
    return 1;
}

int LuaScripting::luaCanReceive(lua_State* L) {
    CanMessage msg;
    if (canDriver.receiveMessage(msg)) {
        lua_pushinteger(L, msg.id);
        lua_pushlstring(L, (const char*)msg.data, msg.length);
        return 2;
    } else {
        return 0;
    }
}

int LuaScripting::luaDelay(lua_State* L) {
    int ms = luaL_checkinteger(L, 1);
    vTaskDelay(pdMS_TO_TICKS(ms));
    return 0;
}

int LuaScripting::luaGetTime(lua_State* L) {
    uint32_t time = xTaskGetTickCount();
    lua_pushinteger(L, time);
    return 1;
}
