/* lua_toxdns.c
 *
 * Lua binding for ToxDNS.
 *
 *  Copyright (C) 2014 Peersuasive Technologies All Rights Reserved.
 *
 *  This file is part of LuaTox.
 *
 *  LuaTox is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  LuaTox is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with LuaTox.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "tox/tox.h"
#include "lua_toxdns.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************
 *                        *
 * lua module functions   *
 *                        *
 **************************/

static ToxDNS *toToxDNS(lua_State* L, int index) {
    ToxDNS *toxDNS = (ToxDNS*)lua_touserdata(L, index);
    if(toxDNS==NULL)
        luaL_typerror(L, index, TOX_DNS_STR);
    return toxDNS;
}

static ToxDNS *checkToxDNS(lua_State* L, int index) {
    luaL_checktype(L, index, LUA_TUSERDATA);
    ToxDNS *toxDNS = (ToxDNS*)luaL_checkudata(L, index, TOX_DNS_STR);
    if(toxDNS==NULL)
        luaL_typerror(L, index, TOX_DNS_STR);
    return toxDNS;
}

static ToxDNS *pushToxDNS(lua_State* L, uint8_t *key) {
    ToxDNS *toxDNS = (ToxDNS*)lua_newuserdata(L, sizeof(ToxDNS));
    toxDNS->key = key;
    toxDNS->dns = tox_dns3_new(key);
    if(toxDNS->dns==NULL) {
        free(key);
        lua_pushstring(L, "Can't create new tox DNS!");
        lua_error(L);
    }
    luaL_getmetatable(L, TOX_DNS_STR);
    lua_setmetatable(L, -2);
    return toxDNS;
}


// dns3_test.c
uint8_t *hex_string_to_bin(char *hex_string) {
    // byte is represented by exactly 2 hex digits, so lenth of binary string
    // is half of that of the hex one. only hex string with even length
    // valid. the more proper implementation would be to check if strlen(hex_string)
    // is odd and return error code if it is. we assume strlen is even. if it's not
    // then the last byte just won't be written in 'ret'.
    size_t i, len = strlen(hex_string) / 2;
    uint8_t *ret = malloc(len);
    char *pos = hex_string;

    for (i = 0; i < len; ++i, pos += 2)
        sscanf(pos, "%2hhx", &ret[i]);

    return ret;
}

/***********************
 *                     *
 * Tox wrapped methods *
 *                     *
 ***********************/

int lua_tox_generate_dns3_string(lua_State* L) {
    ToxDNS *toxDNS = checkToxDNS(L,1);
    size_t len;
    const char *host = luaL_checkstring(L,2);
    uint8_t *name = (uint8_t*)luaL_checklstring(L,3,&len);
    uint8_t res[1024] = {0};
    uint32_t req;
    int status = tox_generate_dns3_string(toxDNS->dns, res + 1 , sizeof(res) - 1, &req, name, len);

    res[0] = '_';
    memcpy(res + status + 1, "._tox.", sizeof("._tox."));
    memcpy((char *)(res + status + 1 + sizeof("._tox.") - 1), host, strlen(host));

    int rlen = status + sizeof("._tox.") + strlen(host);

    lua_settop(L,0);

    if(status<0) {
        lua_pushnil(L);
        lua_pushnumber(L, status);
        return 2;
    }
    else {
        lua_newtable(L);
        lua_pushnumber(L, 1);
        lua_pushnumber(L, req); // request id
        lua_settable(L,-3);
        lua_pushnumber(L, 2);
        lua_pushlstring(L, res, rlen); // request string
        lua_settable(L,-3);
        return 1;
    }
}

int lua_tox_decrypt_dns3_TXT(lua_State* L) {
    ToxDNS *toxDNS = checkToxDNS(L,1);

    uint32_t id = luaL_checknumber(L,2);

    size_t len;
    uint8_t *request = (uint8_t*)luaL_checklstring(L, 3, &len);

    uint8_t tox_id[TOX_FRIEND_ADDRESS_SIZE];
    
    int status = tox_decrypt_dns3_TXT(toxDNS->dns, tox_id, request, len, id);

    lua_settop(L,0);

    char res[TOX_FRIEND_ADDRESS_SIZE * 2 + 1] = {0};
    size_t i;
    for (i = 0; i < TOX_FRIEND_ADDRESS_SIZE; ++i) {
        char xx[3];
        snprintf(xx, sizeof(xx), "%02hhX", tox_id[i]);
        strcat(res, xx);
    }

    if(status<0) {
        lua_pushnil(L);
        lua_pushnumber(L, status);
        return 2;
    }
    else {
        lua_pushstring(L, res);
        lua_pushlstring(L, tox_id, TOX_FRIEND_ADDRESS_SIZE);
        return 2;
    }
}

/************************************
 *                                  *
 * lua module loader and destructor *
 *                                  *
 ************************************/

int lua_toxdns_gc(lua_State* L) {
    ToxDNS *toxDNS = checkToxDNS(L,1);
    lua_settop(L,0);
    if(toxDNS!=NULL) {
        if( toxDNS->key != NULL )
            free(toxDNS->key);
        tox_dns3_kill( toxDNS->dns );
    }
    return 0;
}

static int lua_toxdns_tostring(lua_State* L) {
    char buf[32];
    sprintf(buf, "%p", checkToxDNS(L,1));
    lua_pushfstring(L, "ToxDNS (%s)", buf);
    return 1;
}

int lua_toxdns_new(lua_State* L) {
    char *pub_key = (char*)luaL_checkstring(L,1);
    uint8_t *key = hex_string_to_bin( pub_key );
    lua_settop(L,0);

    ToxDNS *toxDNS = pushToxDNS(L, key);
    reg(L, toxDNS);
    return 1;
}
int lua_toxdns_new_self(lua_State* L) {
    lua_remove(L,1);
    return lua_toxdns_new(L);
}

// TODO: improve: set methods only when new is called
static const luaL_Reg toxdns_methods[] = {
    {"new", lua_toxdns_new},

    {"generate", lua_tox_generate_dns3_string},
    {"decrypt", lua_tox_decrypt_dns3_TXT},

    {NULL,NULL}
};

static const luaL_Reg toxdns_meta[] = {
    {"__gc", lua_toxdns_gc},
    {"__tostring", lua_toxdns_tostring},
    {NULL,NULL}
};

#define TOX_DNS_CLASS "ToxDNSClass"
static const luaL_Reg class_meta[] = {
    {"__call", lua_toxdns_new_self},
    {NULL,NULL}
};

int lua_toxdns_register(lua_State* L) {
    Ls = L;
    lua_newtable(L);
    // lua 5.2's luaL_setfuncs light emulation
    for(int f = 0; toxdns_methods[f].name != NULL; ++f) {
        lua_pushstring(L, toxdns_methods[f].name);
        lua_pushcclosure(L, toxdns_methods[f].func, 0);
        lua_settable(L, -3);
    }

    lua_pushnumber(L, TOX_FRIEND_ADDRESS_SIZE);
    lua_setfield(L, -2, "FRIEND_ADDRESS_SIZE");

    luaL_newmetatable(L, TOX_DNS_STR);
    for(int f = 0; toxdns_meta[f].name != NULL; ++f) {
        lua_pushstring(L, toxdns_meta[f].name);
        lua_pushcclosure(L, toxdns_meta[f].func, 0);
        lua_settable(L, -3);
    }

    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    lua_pop(L,1); // drop metatable

    // class metatable
    luaL_newmetatable(L, TOX_DNS_CLASS);
    for(int f = 0; class_meta[f].name != NULL; ++f) {
        lua_pushstring(L, class_meta[f].name);
        lua_pushcclosure(L, class_meta[f].func, 0);
        lua_settable(L, -3);
    }

    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    lua_setmetatable(L, -2);

    return 1;
}

int luaopen_toxdns(lua_State* L) {
    return lua_toxdns_register(L);
}

#ifdef __cplusplus
}
#endif
