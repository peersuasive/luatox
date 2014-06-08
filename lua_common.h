/* lua_common.h
 *
 * Lua common binding for Tox.
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

#ifndef LUA_TOX_COMMON_H
#define LUA_TOX_COMMON_H

#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#if LUA_VERSION_NUM > 501
int luaL_typerror (lua_State *L, int narg, const char *tname);
#endif

void reg(lua_State*, const void*);
void unreg(lua_State *L, const void* key);
void unregCallbacks(lua_State *L, const void* key);
int set(lua_State*, const void*, const char* name, int index);
int getCallback(lua_State*, const void*, const char* name);
int store(lua_State*, const void* key, const char* name, void *data);
int retrieve(lua_State*, const void* key, const char* name);
void unref(lua_State* L, const void* key, const char* name);
int call_cb(lua_State*, const void*, const char *name, int nb_ret, int nb_args);

lua_State *Ls;

#endif // LUA_TOX_COMMON_H
