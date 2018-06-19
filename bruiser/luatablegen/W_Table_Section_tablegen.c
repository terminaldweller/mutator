
// automatically generated by luatablegen
#include "../lua-5.3.4/src/lua.h"
#include "../lua-5.3.4/src/lauxlib.h"
#include "../lua-5.3.4/src/lualib.h"
#include <inttypes.h>
#include <stdbool.h>
#include "./W_Table_Section_tablegen.h"

#include "../wasm.h"

static W_Table_Section* convert_W_Table_Section (lua_State* __ls, int index) {
	W_Table_Section* dummy = (W_Table_Section*)lua_touserdata(__ls, index);
	if (dummy == NULL) printf("W_Table_Section:bad user data type.\n");
	return dummy;
}

static W_Table_Section* check_W_Table_Section(lua_State* __ls, int index) {
	W_Table_Section* dummy;
	luaL_checktype(__ls, index, LUA_TUSERDATA);
	dummy = (W_Table_Section*)luaL_checkudata(__ls, index, "W_Table_Section");
	if (dummy == NULL) printf("W_Table_Section:bad user data type.\n");
	return dummy;
}

W_Table_Section* push_W_Table_Section(lua_State* __ls) {
	lua_checkstack(__ls, 1);
	W_Table_Section* dummy = lua_newuserdata(__ls, sizeof(W_Table_Section));
	luaL_getmetatable(__ls, "W_Table_Section");
	lua_setmetatable(__ls, -2);
	return dummy;
}

int W_Table_Section_push_args(lua_State* __ls, W_Table_Section* _st) {
	lua_checkstack(__ls, 2);
	lua_pushinteger(__ls, _st->count);
	lua_pushlightuserdata(__ls, _st->entries);
	return 0;
}

int new_W_Table_Section(lua_State* __ls) {
	lua_checkstack(__ls, 2);
	varuint32 count = luaL_optinteger(__ls,-2,0);
	table_type_t** entries = lua_touserdata(__ls,-1);
	W_Table_Section* dummy = push_W_Table_Section(__ls);
	dummy->count = count;
	dummy->entries = entries;
	return 1;
}

static int getter_W_Table_Section_count(lua_State* __ls) {
	W_Table_Section* dummy = check_W_Table_Section(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->count);
	return 1;
}
static int getter_W_Table_Section_entries(lua_State* __ls) {
	W_Table_Section* dummy = check_W_Table_Section(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushlightuserdata(__ls, dummy->entries);
	return 1;
}

static int setter_W_Table_Section_count(lua_State* __ls) {
	W_Table_Section* dummy = check_W_Table_Section(__ls, 1);
	dummy->count = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_W_Table_Section_entries(lua_State* __ls) {
	W_Table_Section* dummy = check_W_Table_Section(__ls, 1);
	dummy->entries = luaL_checkudata(__ls, 2, "W_Table_Section");
	lua_settop(__ls, 1);
	return 1;
}

static const luaL_Reg W_Table_Section_methods[] = {
	{"new", new_W_Table_Section},
	{"set_count", setter_W_Table_Section_count},
	{"set_entries", setter_W_Table_Section_entries},
	{"count", getter_W_Table_Section_count},
	{"entries", getter_W_Table_Section_entries},
	{0,0}
};

static const luaL_Reg W_Table_Section_meta[] = {
	{0, 0}
};

int W_Table_Section_register(lua_State* __ls) {
	luaL_openlib(__ls, "W_Table_Section", W_Table_Section_methods, 0);
	luaL_newmetatable(__ls, "W_Table_Section");
	luaL_openlib(__ls, 0, W_Table_Section_meta, 0);
	lua_pushliteral(__ls, "__index");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pushliteral(__ls, "__metatable");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pop(__ls, 1);
return 1;
}


