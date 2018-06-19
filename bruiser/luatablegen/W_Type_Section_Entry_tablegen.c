
// automatically generated by luatablegen
#include "../lua-5.3.4/src/lua.h"
#include "../lua-5.3.4/src/lauxlib.h"
#include "../lua-5.3.4/src/lualib.h"
#include <inttypes.h>
#include <stdbool.h>
#include "./W_Type_Section_Entry_tablegen.h"

#include "../wasm.h"

static W_Type_Section_Entry* convert_W_Type_Section_Entry (lua_State* __ls, int index) {
	W_Type_Section_Entry* dummy = (W_Type_Section_Entry*)lua_touserdata(__ls, index);
	if (dummy == NULL) printf("W_Type_Section_Entry:bad user data type.\n");
	return dummy;
}

static W_Type_Section_Entry* check_W_Type_Section_Entry(lua_State* __ls, int index) {
	W_Type_Section_Entry* dummy;
	luaL_checktype(__ls, index, LUA_TUSERDATA);
	dummy = (W_Type_Section_Entry*)luaL_checkudata(__ls, index, "W_Type_Section_Entry");
	if (dummy == NULL) printf("W_Type_Section_Entry:bad user data type.\n");
	return dummy;
}

W_Type_Section_Entry* push_W_Type_Section_Entry(lua_State* __ls) {
	lua_checkstack(__ls, 1);
	W_Type_Section_Entry* dummy = lua_newuserdata(__ls, sizeof(W_Type_Section_Entry));
	luaL_getmetatable(__ls, "W_Type_Section_Entry");
	lua_setmetatable(__ls, -2);
	return dummy;
}

int W_Type_Section_Entry_push_args(lua_State* __ls, W_Type_Section_Entry* _st) {
	lua_checkstack(__ls, 5);
	lua_pushinteger(__ls, _st->form);
	lua_pushinteger(__ls, _st->param_count);
	lua_pushlightuserdata(__ls, _st->param_types);
	lua_pushinteger(__ls, _st->return_count);
	lua_pushlightuserdata(__ls, _st->return_types);
	return 0;
}

int new_W_Type_Section_Entry(lua_State* __ls) {
	lua_checkstack(__ls, 5);
	varint7 form = luaL_optinteger(__ls,-5,0);
	varuint32 param_count = luaL_optinteger(__ls,-4,0);
	varint7* param_types = lua_touserdata(__ls,-3);
	varuint1 return_count = luaL_optinteger(__ls,-2,0);
	varint7* return_types = lua_touserdata(__ls,-1);
	W_Type_Section_Entry* dummy = push_W_Type_Section_Entry(__ls);
	dummy->form = form;
	dummy->param_count = param_count;
	dummy->param_types = param_types;
	dummy->return_count = return_count;
	dummy->return_types = return_types;
	return 1;
}

static int getter_W_Type_Section_Entry_form(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->form);
	return 1;
}
static int getter_W_Type_Section_Entry_param_count(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->param_count);
	return 1;
}
static int getter_W_Type_Section_Entry_param_types(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushlightuserdata(__ls, dummy->param_types);
	return 1;
}
static int getter_W_Type_Section_Entry_return_count(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->return_count);
	return 1;
}
static int getter_W_Type_Section_Entry_return_types(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushlightuserdata(__ls, dummy->return_types);
	return 1;
}

static int setter_W_Type_Section_Entry_form(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	dummy->form = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_W_Type_Section_Entry_param_count(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	dummy->param_count = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_W_Type_Section_Entry_param_types(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	dummy->param_types = luaL_checkudata(__ls, 2, "W_Type_Section_Entry");
	lua_settop(__ls, 1);
	return 1;
}
static int setter_W_Type_Section_Entry_return_count(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	dummy->return_count = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_W_Type_Section_Entry_return_types(lua_State* __ls) {
	W_Type_Section_Entry* dummy = check_W_Type_Section_Entry(__ls, 1);
	dummy->return_types = luaL_checkudata(__ls, 2, "W_Type_Section_Entry");
	lua_settop(__ls, 1);
	return 1;
}

static const luaL_Reg W_Type_Section_Entry_methods[] = {
	{"new", new_W_Type_Section_Entry},
	{"set_form", setter_W_Type_Section_Entry_form},
	{"set_param_count", setter_W_Type_Section_Entry_param_count},
	{"set_param_types", setter_W_Type_Section_Entry_param_types},
	{"set_return_count", setter_W_Type_Section_Entry_return_count},
	{"set_return_types", setter_W_Type_Section_Entry_return_types},
	{"form", getter_W_Type_Section_Entry_form},
	{"param_count", getter_W_Type_Section_Entry_param_count},
	{"param_types", getter_W_Type_Section_Entry_param_types},
	{"return_count", getter_W_Type_Section_Entry_return_count},
	{"return_types", getter_W_Type_Section_Entry_return_types},
	{0,0}
};

static const luaL_Reg W_Type_Section_Entry_meta[] = {
	{0, 0}
};

int W_Type_Section_Entry_register(lua_State* __ls) {
	luaL_openlib(__ls, "W_Type_Section_Entry", W_Type_Section_Entry_methods, 0);
	luaL_newmetatable(__ls, "W_Type_Section_Entry");
	luaL_openlib(__ls, 0, W_Type_Section_Entry_meta, 0);
	lua_pushliteral(__ls, "__index");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pushliteral(__ls, "__metatable");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pop(__ls, 1);
return 1;
}


