
// automatically generated by luatablegen
#include "../lua-5.3.4/src/lua.h"
#include "../lua-5.3.4/src/lauxlib.h"
#include "../lua-5.3.4/src/lualib.h"
#include <inttypes.h>
#include <stdbool.h>
#include "./init_expr_t_tablegen.h"

#include "../wasm.h"

static init_expr_t* convert_init_expr_t (lua_State* __ls, int index) {
	init_expr_t* dummy = (init_expr_t*)lua_touserdata(__ls, index);
	if (dummy == NULL) printf("init_expr_t:bad user data type.\n");
	return dummy;
}

static init_expr_t* check_init_expr_t(lua_State* __ls, int index) {
	init_expr_t* dummy;
	luaL_checktype(__ls, index, LUA_TUSERDATA);
	dummy = (init_expr_t*)luaL_checkudata(__ls, index, "init_expr_t");
	if (dummy == NULL) printf("init_expr_t:bad user data type.\n");
	return dummy;
}

init_expr_t* push_init_expr_t(lua_State* __ls) {
	lua_checkstack(__ls, 1);
	init_expr_t* dummy = lua_newuserdata(__ls, sizeof(init_expr_t));
	luaL_getmetatable(__ls, "init_expr_t");
	lua_setmetatable(__ls, -2);
	return dummy;
}

int init_expr_t_push_args(lua_State* __ls, init_expr_t* _st) {
	lua_checkstack(__ls, 2);
	lua_pushinteger(__ls, _st->size);
	lua_pushstring(__ls, _st->code);
	return 0;
}

int new_init_expr_t(lua_State* __ls) {
	lua_checkstack(__ls, 2);
	varuint32 size = luaL_optinteger(__ls,-2,0);
	char* code = lua_tostring(__ls,-1);
	init_expr_t* dummy = push_init_expr_t(__ls);
	dummy->size = size;
	dummy->code = code;
	return 1;
}

static int getter_init_expr_t_size(lua_State* __ls) {
	init_expr_t* dummy = check_init_expr_t(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushinteger(__ls, dummy->size);
	return 1;
}
static int getter_init_expr_t_code(lua_State* __ls) {
	init_expr_t* dummy = check_init_expr_t(__ls, 1);
	lua_pop(__ls, -1);
	lua_pushstring(__ls, dummy->code);
	return 1;
}

static int setter_init_expr_t_size(lua_State* __ls) {
	init_expr_t* dummy = check_init_expr_t(__ls, 1);
	dummy->size = luaL_checkinteger(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}
static int setter_init_expr_t_code(lua_State* __ls) {
	init_expr_t* dummy = check_init_expr_t(__ls, 1);
	dummy->code = luaL_checkstring(__ls, 2);
	lua_settop(__ls, 1);
	return 1;
}

static const luaL_Reg init_expr_t_methods[] = {
	{"new", new_init_expr_t},
	{"set_size", setter_init_expr_t_size},
	{"set_code", setter_init_expr_t_code},
	{"size", getter_init_expr_t_size},
	{"code", getter_init_expr_t_code},
	{0,0}
};

static const luaL_Reg init_expr_t_meta[] = {
	{0, 0}
};

int init_expr_t_register(lua_State* __ls) {
	luaL_openlib(__ls, "init_expr_t", init_expr_t_methods, 0);
	luaL_newmetatable(__ls, "init_expr_t");
	luaL_openlib(__ls, 0, init_expr_t_meta, 0);
	lua_pushliteral(__ls, "__index");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pushliteral(__ls, "__metatable");
	lua_pushvalue(__ls, -3);
	lua_rawset(__ls, -3);
	lua_pop(__ls, 1);
return 1;
}


