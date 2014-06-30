/* lua_toxdns.h
 *
 * Lua binding for Tox DNS.
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

#ifndef LUA_TOXDNS_H
#define LUA_TOXDNS_H

#include <stdint.h>

#include "lua_common.h"

#include "tox/toxdns.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TOX_DNS_STR "ToxDNS"
typedef struct _ToxDNS {
    void *dns;
    uint8_t *key;
} ToxDNS;

int lua_tox_generate_dns3_string(lua_State*);
int lua_tox_decrypt_dns3_TXT(lua_State*);

#ifdef __cplusplus
}
#endif

#endif // LUA_TOXDNS_H
