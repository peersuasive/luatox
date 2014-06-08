#include <stdlib.h>
#include "lua_common.h"

#if LUA_VERSION_NUM > 501
int luaL_typerror (lua_State *L, int narg, const char *tname) {
  const char *msg = lua_pushfstring(L, "%s expected, got %s",
                                    tname, luaL_typename(L, narg));
  return luaL_argerror(L, narg, msg);
}
#endif



void reg(lua_State *L, const void* key) {
    if(key==NULL) {
        lua_pushstring(L, "Trying to register with a NULL key!");
        lua_error(L);
    }
    lua_pushlightuserdata(L, (void*)key); // check if table exists
    lua_gettable(L, LUA_REGISTRYINDEX);   //
    if ( lua_isnil(L, -1) ) {
        lua_pop(L,1);
        lua_pushlightuserdata(L, (void*)key); // check if table exists
        lua_newtable(L);
        lua_settable(L, LUA_REGISTRYINDEX);
    }
    else lua_pop(L,1);  // already there
}

void unreg(lua_State *L, const void* key) {
    if(key==NULL) {
        lua_pushstring(L, "Trying to unregister with a NULL key!");
        lua_error(L);
    }
    lua_pushlightuserdata(L, (void*)key);
    lua_pushnil(L);
    lua_settable(L, LUA_REGISTRYINDEX);
}


void unregCallbacks(lua_State *L, const void* key) {
    if(key==NULL) {
        lua_pushstring(L, "Trying to unregister with a NULL key!");
        lua_error(L);
    }
    lua_pushlightuserdata(L, (void*)key);
    lua_newtable(L);
    lua_settable(L, LUA_REGISTRYINDEX);
}

int set(lua_State* L, const void* key, const char* name, int index) {
    int res = 0;
    if ( ! lua_isfunction(L, index) ) {
        lua_pushfstring(L, "luatox ERROR: callback: %s is not a valid function: %s.", 
                                                        name, lua_typename(L, lua_type(L, index)));
        lua_error(L);
    }
    int n = (index<0) ? (lua_gettop(L)-(index+1)) : index;
    //int n = -(index+1);
    lua_pushlightuserdata(L, (void*)key);
    lua_gettable(L, LUA_REGISTRYINDEX);
    if ( lua_istable(L, -1) ) {
        int i = lua_gettop(L);
        lua_pushstring(L, name);
        lua_pushvalue(L, n);
        lua_settable(L, i);
        lua_pop(L,1);
        lua_remove(L, n);
        res = 1;
    } else {
        printf("** set: couldn't get registry!!!\n");
        lua_pushfstring(L, "Can't get registry!");
        lua_error(L);
        lua_pop(L,1); // nil
        //std::cout << "Can't get registry for " << key << std::endl;
        //throwError("LUCE ERROR: can't get registry to store callbacks");
    }
    return res;
}

int store(lua_State* L, const void* key, const char* name, void *data) {
    int res = 0;
    lua_pushlightuserdata(L, (void*)key);
    lua_gettable(L, LUA_REGISTRYINDEX);
    if ( lua_istable(L, -1) ) {
        lua_pushlightuserdata(L, data);
        lua_setfield(L, -2, name);
        lua_pop(L,1); // pop registry
        res = 1;
    }
    else {
        printf("** set: couldn't get the associated registry.\n");
        lua_pushfstring(L, "Can't get associated registry.");
        lua_error(L);
        //lua_pop(L,1); // nil
    }
    return res; 
}

// push lighuserdata on top
int retrieve(lua_State* L, const void* key, const char* name) {
    int res = 0;
    lua_pushlightuserdata(L, (void*)key);
    lua_gettable(L, LUA_REGISTRYINDEX);
    int i = lua_gettop(L);
    if ( lua_istable(L, -1) ) {
        lua_getfield(L, -1, name);
        res = 1;
    }
    lua_remove(L, i);
    return res;
}

void unref(lua_State* L, const void* key, const char* name) {
    lua_pushlightuserdata(L, (void*)key);
    lua_gettable(L, LUA_REGISTRYINDEX);
    int i = lua_gettop(L);
    if ( lua_istable(L, -1) ) {
        lua_getfield(L, -1, name);
        void *p = (void*)lua_touserdata(L,-1);
        lua_pop(L,1);
        lua_pushnil(L);
        lua_setfield(L, -1, name);
        free(p);
    }
    lua_remove(L, i);
}


int getCallback(lua_State *L, const void* key, const char* name) {
    int res = 0;
    lua_pushlightuserdata(L, (void*)key);
    lua_gettable(L, LUA_REGISTRYINDEX);
    int i = lua_gettop(L);
    if ( lua_istable(L, -1) ) {
        lua_getfield(L, -1, name);
        if ( ! lua_isfunction(L, -1 ) ) {
            printf("** callbacks: NOT a function: %s!\n", lua_typename(L, lua_type(L,-1)));
            lua_pop(L,1); // nil
        }
        else
            res = 1;
    }
    lua_remove(L, i);
    // leaves callback on top
    return res;
}


int call_cb(lua_State *L, const void* key, const char *name, int nb_ret, int nb_args) {
    if ( ! getCallback(L, key, name) ) {
        printf("WARNING: no callback found with name: %s\n", name);
        return 0; // not found
    }
    // move function on top
    lua_insert(L, -(nb_args+1));

    int status = 0;
    if ( ! (status = ! lua_pcall(L, nb_args, nb_ret, 0)) ) {
        const char *msg = lua_tostring(L,-1);
        lua_pushfstring(L, "Failed to execute callback '%s': %s", name, msg);
        lua_error(L);
    }
    return status;
}
