
// automatically generated by luatablegen
#include "../lua-5.3.4/src/lua.h"
#include "../lua-5.3.4/src/lauxlib.h"
#include "../lua-5.3.4/src/lualib.h"
#include <inttypes.h>
#include <stdbool.h>
#include "./W_Function_Body_tablegen.h"

#include "../wasm.h"

static W_Function_Body* convert_W_Function_Body (lua_State* __ls, int index) {
	W_Function_Body* dummy = (W_Function_Body*)lua_touserdata(__ls, index);
	if (dummy == NULL) printf("W_Function_Body:bad user data type.\n");
	return dummy;
}

static W_Function_Body* check_W_Function_Body(lua_State* __ls, int index) {
	W_Function_Body* dummy;
	luaL_checktype(__ls, index, LUA_TUSERDATA);
	dummy = (W_Function_Body*)luaL_checkudata(__ls, index, "W_Function_Body");
	if (dummy == NULL) printf("W_Function_Body:bad user data type.\n");
	return dummy;
}

W_Function_Body* push_W_Function_Body(lua_State* __ls) {
	lua_checkstack(__ls, 1);
	W_Function_Body* dummy = lua_newuserdata(__ls, sizeof(W_Function_Body));
	luaL_getmetatable(__ls, "W_Function_Body");
	lua_setmetatable(__ls, -2);
	return dummy;
}

int W_Function_Body_push_args(lua_State* __ls, W_Function_Body* _st) {
	lua_checkstack(__ls, 4);
	lua_pushinteger(__ls, _st->body_size);
	lua_pushinteger(__ls, _st->local_count);
	lua_pushlightuserdata(__ls, _st->locals);
	lua_pushstring(__ls, _st->code);
	return 0;
}

int new_W_Function_Body(lua_State* __ls) {
	lua_checkstack(__ls, 4);
	varuint32 body_size = luaL_optinteger(__ls,-4,0);
	varuint32 local_count = luaL_optinteger(__ls,-3,0);
	W_Local_Entry** locals = lua_touserdata(__ls,-2);
	char* code = lua_tostring(__ls,-1);
	W_Function_Body* dummy = push_W_Function_Body(__ls);
	dummy->body_size = body_size;
	dummy->local_count = local_count;
	dummy->locals = locals;
	dummy->code = code;
	return 1;
}

static int getter_W_Function_Body_body_size(lua_State* __ls) {
	W_Function_Body* dummy = check_W_Function_Body(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->body_size);
	return 1;
}
static int getter_W_Function_Body_local_count(lua_State* __ls) {
	W_Function_Body* dummy = check_W_Function_Body(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->local_count);
	return 1;
}
static int getter_W_Function_Body_locals(lua_State* __ls) {
	W_Function_Body* dummy = check_W_Function_Body(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushlightuserdata(__ls, dummy->locals);
	return 1;
}
static int getter_W_Function_Body_code(lua_State* __ls) {
	W_Function_Body* dummy = check_W_Function_Body(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushstring(__ls, dummy->code);
	return 1;
}

static int setter_W_Function_Body_body_size(lua_State* __ls) {
	W_Function_Body* dummy = check_W_Function_Body(__ls, 1);
	dummy->body_size = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_W_Function_Body_local_count(lua_State* __ls) {
	W_Function_Body* dummy = check_W_Function_Body(__ls, 1);
	dummy->local_count = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_W_Function_Body_locals(lua_State* __ls) {
	W_Function_Body* dummy = check_W_Function_Body(__ls, 1);
	dummy->locals = luaL_checkudata(__ls, 2, "W_Function_Body");
	lua_settop(__ls, 1);
	return 1;
}
static int setter_W_Function_Body_code(lua_State* __ls) {
	W_Function_Body* dummy = check_W_Function_Body(__ls, 1);
	dummy->code = luaL_checkstring(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}

static const luaL_Reg W_Function_Body_methods[] = {
	{"new", new_W_Function_Body},
	{"set_body_size", setter_W_Function_Body_body_size},
	{"set_local_count", setter_W_Function_Body_local_count},
	{"set_locals", setter_W_Function_Body_locals},
	{"set_code", setter_W_Function_Body_code},
	{"body_size", getter_W_Function_Body_body_size},
	{"local_count", getter_W_Function_Body_local_count},
	{"locals", getter_W_Function_Body_locals},
	{"code", getter_W_Function_Body_code},
	{0,0}
};

static const luaL_Reg W_Function_Body_meta[] = {
	{0, 0}
};

int W_Function_Body_register(lua_State* __ls) {
	luaL_openlib(__ls, "W_Function_Body", W_Function_Body_methods, 0);
	luaL_newmetatable(__ls, "W_Function_Body");
	luaL_openlib(__ls, 0, W_Function_Body_meta, 0);
	lua_pushliteral(__ls, "__index");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pushliteral(__ls, "__metatable");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pop(__ls, 1);
return 1;
}


