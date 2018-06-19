
// automatically generated by luatablegen
#include "../lua-5.3.4/src/lua.h"
#include "../lua-5.3.4/src/lauxlib.h"
#include "../lua-5.3.4/src/lualib.h"
#include <inttypes.h>
#include <stdbool.h>
#include "./resizable_limit_t_tablegen.h"

#include "../wasm.h"

static resizable_limit_t* convert_resizable_limit_t (lua_State* __ls, int index) {
	resizable_limit_t* dummy = (resizable_limit_t*)lua_touserdata(__ls, index);
	if (dummy == NULL) printf("resizable_limit_t:bad user data type.\n");
	return dummy;
}

static resizable_limit_t* check_resizable_limit_t(lua_State* __ls, int index) {
	resizable_limit_t* dummy;
	luaL_checktype(__ls, index, LUA_TUSERDATA);
	dummy = (resizable_limit_t*)luaL_checkudata(__ls, index, "resizable_limit_t");
	if (dummy == NULL) printf("resizable_limit_t:bad user data type.\n");
	return dummy;
}

resizable_limit_t* push_resizable_limit_t(lua_State* __ls) {
	lua_checkstack(__ls, 1);
	resizable_limit_t* dummy = lua_newuserdata(__ls, sizeof(resizable_limit_t));
	luaL_getmetatable(__ls, "resizable_limit_t");
	lua_setmetatable(__ls, -2);
	return dummy;
}

int resizable_limit_t_push_args(lua_State* __ls, resizable_limit_t* _st) {
	lua_checkstack(__ls, 3);
	lua_pushinteger(__ls, _st->flags);
	lua_pushinteger(__ls, _st->initial);
	lua_pushinteger(__ls, _st->maximum);
	return 0;
}

int new_resizable_limit_t(lua_State* __ls) {
	lua_checkstack(__ls, 3);
	varuint1 flags = luaL_optinteger(__ls,-3,0);
	varuint32 initial = luaL_optinteger(__ls,-2,0);
	varuint32 maximum = luaL_optinteger(__ls,-1,0);
	resizable_limit_t* dummy = push_resizable_limit_t(__ls);
	dummy->flags = flags;
	dummy->initial = initial;
	dummy->maximum = maximum;
	return 1;
}

static int getter_resizable_limit_t_flags(lua_State* __ls) {
	resizable_limit_t* dummy = check_resizable_limit_t(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->flags);
	return 1;
}
static int getter_resizable_limit_t_initial(lua_State* __ls) {
	resizable_limit_t* dummy = check_resizable_limit_t(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->initial);
	return 1;
}
static int getter_resizable_limit_t_maximum(lua_State* __ls) {
	resizable_limit_t* dummy = check_resizable_limit_t(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->maximum);
	return 1;
}

static int setter_resizable_limit_t_flags(lua_State* __ls) {
	resizable_limit_t* dummy = check_resizable_limit_t(__ls, 1);
	dummy->flags = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_resizable_limit_t_initial(lua_State* __ls) {
	resizable_limit_t* dummy = check_resizable_limit_t(__ls, 1);
	dummy->initial = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_resizable_limit_t_maximum(lua_State* __ls) {
	resizable_limit_t* dummy = check_resizable_limit_t(__ls, 1);
	dummy->maximum = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}

static const luaL_Reg resizable_limit_t_methods[] = {
	{"new", new_resizable_limit_t},
	{"set_flags", setter_resizable_limit_t_flags},
	{"set_initial", setter_resizable_limit_t_initial},
	{"set_maximum", setter_resizable_limit_t_maximum},
	{"flags", getter_resizable_limit_t_flags},
	{"initial", getter_resizable_limit_t_initial},
	{"maximum", getter_resizable_limit_t_maximum},
	{0,0}
};

static const luaL_Reg resizable_limit_t_meta[] = {
	{0, 0}
};

int resizable_limit_t_register(lua_State* __ls) {
	luaL_openlib(__ls, "resizable_limit_t", resizable_limit_t_methods, 0);
	luaL_newmetatable(__ls, "resizable_limit_t");
	luaL_openlib(__ls, 0, resizable_limit_t_meta, 0);
	lua_pushliteral(__ls, "__index");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pushliteral(__ls, "__metatable");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pop(__ls, 1);
return 1;
}


