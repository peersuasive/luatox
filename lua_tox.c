/* lua_tox.c
 *
 * Lua binding for Tox.
 *
 *  Copyright (C) 2014 Peersuasive Technologies All Rights Reserved.
 *
 *  This file is part of Luce.
 *
 *  Luce is free software: you can redistribute it and/or modify
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

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "lua_tox.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************
 *                        *
 * lua module functions   *
 *                        *
 **************************/

static Tox *toTox(lua_State* L, int index) {
    Tox *tox = (Tox*)lua_touserdata(L, index);
    if(tox==NULL)
        luaL_typerror(L, index, TOX_STR);
    return tox;
}

static Tox *checkTox(lua_State* L, int index) {
    luaL_checktype(L, index, LUA_TUSERDATA);
    LTox *ltox = (LTox*)luaL_checkudata(L, index, TOX_STR);
    Tox *tox = ltox->tox;
    if(tox==NULL)
        luaL_typerror(L, index, TOX_STR);
    return tox;
}
static LTox *checkLTox(lua_State* L, int index) {
    luaL_checktype(L, index, LUA_TUSERDATA);
    LTox *ltox = (LTox*)luaL_checkudata(L, index, TOX_STR);
    if(ltox->tox==NULL)
        luaL_typerror(L, index, TOX_STR);
    return ltox;
}

static LTox *pushTox(lua_State* L, uint8_t ipv6enabled) {
    LTox *ltox = (LTox*)lua_newuserdata(L, sizeof(LTox));
    ltox->tox = tox_new(ipv6enabled);
    if(ltox->tox==NULL) {
        lua_pushstring(L, "Can't create new tox!");
        lua_error(L);
    }
    luaL_getmetatable(L, TOX_STR);
    lua_setmetatable(L, -2);
    return ltox;
}

void bin_to_string(uint8_t *address, char *id, int len) {
    size_t i;
    for (i = 0; i < len; ++i) {
        char xx[3];
        snprintf(xx, sizeof(xx), "%02X", address[i] & 0xff);
        strcat(id, xx);
    }
}
void pub_bin_to_string(uint8_t *address, char *id) {
    bin_to_string(address, id, TOX_CLIENT_ID_SIZE);
}
void friend_bin_to_string(uint8_t *address, char *id) {
    bin_to_string(address, id, TOX_FRIEND_ADDRESS_SIZE);
}

int string_to_bin(const char *id, uint8_t *address, int len) {
    size_t i;
    char xx[3];
    uint32_t x;

    for (i = 0; i < len; ++i) {
        xx[0] = id[2 * i];
        xx[1] = id[2 * i + 1];
        xx[2] = '\0';

        if (sscanf(xx, "%02x", &x) != 1) {
            // error
            return -1;
        }

        address[i] = x;
    }
    return 0;
}
int friend_string_to_bin(const char *id, uint8_t *address) {
    return string_to_bin(id, address, TOX_FRIEND_ADDRESS_SIZE);
}
int pub_string_to_bin(const char *id, uint8_t *address) {
    return string_to_bin(id, address, TOX_CLIENT_ID_SIZE);
}

/**************************
 *                        *
 * callbacks              *
 *                        *
 **************************/

void on_friend_request(Tox *tox, uint8_t *public_key, uint8_t *data, uint16_t length, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.friend_request) {
        lua_pushlstring(Ls, public_key, TOX_CLIENT_ID_SIZE);
        lua_pushlstring(Ls, data, length);
        call_cb(Ls, ltox, "friend_request", 0, 2);
    }
}
int lua_tox_callback_friend_request(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "friend_request", 2);
    lua_settop(L,0);

    ltox->callbacks.friend_request = 1;
    tox_callback_friend_request(ltox->tox, on_friend_request, ltox);
    return 0;
}

void on_friend_message(Tox *tox, int32_t friendnumber, uint8_t *message, uint16_t length, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.friend_message) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushlstring(Ls, message, length);
        call_cb(Ls, ltox, "friend_message", 0, 2);
    }
}
int lua_tox_callback_friend_message(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "friend_message", 2);
    lua_settop(L,0);

    ltox->callbacks.friend_message = 1;
    tox_callback_friend_message(ltox->tox, on_friend_message, ltox);
    return 0;
}

void on_friend_action(Tox *tox, int32_t friendnumber, uint8_t *action, uint16_t length, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.friend_action) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushlstring(Ls, action, length);
        call_cb(Ls, ltox, "friend_action", 0, 2);
    }
}
int lua_tox_callback_friend_action(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "friend_action", 2);
    lua_settop(L,0);

    ltox->callbacks.friend_action = 1;
    tox_callback_friend_action(ltox->tox, on_friend_action, ltox);
    return 0;
}

void on_name_change(Tox *tox, int32_t friendnumber, uint8_t *string, uint16_t length, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.name_change) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushlstring(Ls, string, length);
        call_cb(Ls, ltox, "name_change",0,2);
    }
}
int lua_tox_callback_name_change(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "name_change", 2);
    lua_settop(L,0);

    ltox->callbacks.name_change = 1;
    tox_callback_name_change(ltox->tox, on_name_change, ltox);
    return 0;
}

void on_status_message(Tox *tox, int32_t friendnumber, uint8_t *string, uint16_t length, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.status_message) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushlstring(Ls, string, length);
        call_cb(Ls, ltox, "status_message", 0, 2);
    }
}
int lua_tox_callback_status_message(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "status_message", 2);
    lua_settop(L,0);

    ltox->callbacks.status_message = 1;
    tox_callback_status_message(ltox->tox, on_status_message, ltox);
    return 0;
}

void on_user_status(Tox *tox, int32_t friendnumber, uint8_t status, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.user_status) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushnumber(Ls, status);
        call_cb(Ls, ltox, "user_status", 0, 2);
    }
}
int lua_tox_callback_user_status(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "user_status", 2);
    lua_settop(L,0);

    ltox->callbacks.user_status = 1;
    tox_callback_user_status(ltox->tox, on_user_status, ltox);
    return 0;
}

void on_typing_change(Tox *tox, int32_t friendnumber, uint8_t is_typing, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.typing_change) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushboolean(Ls, (is_typing==1));
        call_cb(Ls, ltox, "typing_change", 0, 2);
    }
}
int lua_tox_callback_typing_change(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "typing_change", 2);
    lua_settop(L,0);

    ltox->callbacks.typing_change = 1;
    tox_callback_typing_change(ltox->tox, on_typing_change, ltox);
    return 0;
}

void on_read_receipt(Tox *tox, int32_t friendnumber, uint32_t receipt, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.read_receipt) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushnumber(Ls, receipt);
        call_cb(Ls, ltox, "read_receipt", 0, 2);
    }
}
int lua_tox_callback_read_receipt(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "read_receipt", 2);
    lua_settop(L,0);

    ltox->callbacks.read_receipt = 1;
    tox_callback_read_receipt(ltox->tox, on_read_receipt, ltox);
    return 0;
}

void on_connection_status(Tox *tox, int32_t friendnumber, uint8_t status, void *_ltox) {
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.connection_status) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushnumber(Ls, status);
        call_cb(Ls, ltox, "connection_status", 0, 2);
    }
}
int lua_tox_callback_connection_status(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "connection_status", 2);
    lua_settop(L,0);

    ltox->callbacks.connection_status = 1;
    tox_callback_connection_status(ltox->tox, on_connection_status, ltox);
    return 0;
}

void on_group_invite(Tox *tox, int32_t friendnumber, uint8_t *group_pub_key, void *_ltox)
{
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.group_invite) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushlstring(Ls, group_pub_key, TOX_CLIENT_ID_SIZE);
        call_cb(Ls, ltox, "group_invite", 0, 2);
    }
}
int lua_tox_callback_group_invite(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "group_invite", 2);
    lua_settop(L,0);

    ltox->callbacks.group_invite = 1;
    tox_callback_group_invite(ltox->tox, on_group_invite, ltox);
    return 0;
}

void on_group_message(Tox *tox, int groupnumber, int peernumber, uint8_t *message, uint16_t length, void *_ltox)
{
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.group_message) {
        lua_pushnumber(Ls, groupnumber);
        lua_pushnumber(Ls, peernumber);
        lua_pushlstring(Ls, message, length);
        call_cb(Ls, ltox, "group_message", 0, 3);
    }
}
int lua_tox_callback_group_message(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "group_message", 2);
    lua_settop(L,0);

    ltox->callbacks.group_message = 1;
    tox_callback_group_message(ltox->tox, on_group_message, ltox);
    return 0;
}

void on_group_action(Tox *tox, int groupnumber, int peernumber, uint8_t *action, uint16_t length, void *_ltox) 
{
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.group_action) {
        lua_pushnumber(Ls, groupnumber);
        lua_pushnumber(Ls, peernumber);
        lua_pushlstring(Ls, action, length);
        call_cb(Ls, ltox, "group_action", 0, 3);
    }
}
int lua_tox_callback_group_action(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "group_action", 2);
    lua_settop(L,0);

    ltox->callbacks.group_action = 1;
    tox_callback_group_action(ltox->tox, on_group_action, ltox);
    return 0;
}

void on_group_namelist_change(Tox *tox, int groupnumber, int peernumber, uint8_t change, void *_ltox) 
{
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.group_namelist_change) {
        lua_pushnumber(Ls, groupnumber);
        lua_pushnumber(Ls, peernumber);
        lua_pushnumber(Ls, change);
        call_cb(Ls, ltox, "group_namelist_change", 0, 2);
    }
}
int lua_tox_callback_group_namelist_change(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "group_namelist_change", 2);
    lua_settop(L,0);

    ltox->callbacks.group_namelist_change = 1;
    tox_callback_group_namelist_change(ltox->tox, on_group_namelist_change, ltox);
    return 0;
}

void on_file_send_request(Tox *tox, int32_t friendnumber, uint8_t filenumber, uint64_t filesize,
                                 uint8_t *filename, uint16_t filename_length, void *_ltox)
{
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.file_send_request) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushnumber(Ls, filenumber);
        lua_pushnumber(Ls, filesize);
        lua_pushlstring(Ls, filename, filename_length);
        call_cb(Ls, ltox, "file_send_request", 0, 4);
    }
}
int lua_tox_callback_file_send_request(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "file_send_request", 2);
    lua_settop(L,0);

    ltox->callbacks.file_send_request = 1;
    tox_callback_file_send_request(ltox->tox, on_file_send_request, ltox);
    return 0;
}

void on_file_control (Tox *tox, int32_t friendnumber, uint8_t send_receive, uint8_t filenumber,
                              uint8_t control_type, uint8_t *data, uint16_t length, void *_ltox)
{
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.file_control) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushnumber(Ls, send_receive); // reveiving == 1, sending == 0
        lua_pushnumber(Ls, filenumber);
        lua_pushnumber(Ls, control_type);
        lua_pushlstring(Ls, data, length);
        call_cb(Ls, ltox, "file_control", 0, 5);
    }
}
int lua_tox_callback_file_control(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "file_control", 2);
    lua_settop(L,0);

    ltox->callbacks.file_control = 1;
    tox_callback_file_control(ltox->tox, on_file_control, ltox);
    return 0;
}

void on_file_data(Tox *tox, int32_t friendnumber, uint8_t filenumber, uint8_t *data, uint16_t length, void *_ltox)
{
    LTox *ltox = (LTox*)_ltox;
    if(ltox->callbacks.file_data) {
        lua_pushnumber(Ls, friendnumber);
        lua_pushnumber(Ls, filenumber);
        lua_pushlstring(Ls, data, length);
        call_cb(Ls, ltox, "file_data", 0, 3);
    }
}
int lua_tox_callback_file_data(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    set(L, ltox, "file_data", 2);
    lua_settop(L,0);

    ltox->callbacks.file_data = 1;
    tox_callback_file_data(ltox->tox, on_file_data, ltox);
    return 0;
}

/***********************
 *                     *
 * Tox wrapped methods *
 *                     *
 ***********************/

int lua_tox_get_address(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);

    uint8_t address[TOX_FRIEND_ADDRESS_SIZE];
    tox_get_address(tox, address);

    lua_pushlstring(L, address, TOX_FRIEND_ADDRESS_SIZE);

    /*
    // convert to string
    char id[TOX_FRIEND_ADDRESS_SIZE * 2 + 1] = {0};
    friend_bin_to_string(address, id);
    lua_pushstring(L, id);
    return 2;
    */
    return 1;
}

int lua_tox_add_friend(lua_State* L) {
    Tox *tox = checkTox(L,1);
    uint8_t *address = (uint8_t*)luaL_checkstring(L, 2);
    size_t len;
    uint8_t *data = (uint8_t*)lua_tolstring(L,3, &len);
    lua_settop(L,0);

    int32_t status = tox_add_friend(tox, address, data, len);
    if(status<0) {
        lua_pushnil(L);
        lua_pushnumber(L, status);
        return 2;
    }
    else {
        lua_pushnumber(L, status);
        return 1;
    }
}

int lua_tox_add_friend_norequest(lua_State* L) {
    Tox *tox = checkTox(L,1);
    uint8_t *client_id = (uint8_t*)luaL_checkstring(L,2);
    lua_settop(L,0);
    int32_t status = tox_add_friend_norequest(tox, client_id);
    lua_pushnumber(L,status);
    if(status<0) {
        lua_pushnil(L);
        lua_pushstring(L,"Failed to add friend");
    }
    else {
        lua_pushnumber(L, status);
        lua_pushnil(L);
    }

    return 2;
}   

int lua_tox_get_friend_number(lua_State* L) {
    Tox *tox = checkTox(L,1);
    uint8_t *client_id = (uint8_t*)luaL_checkstring(L,2);
    lua_settop(L,0);

    int32_t num = tox_get_friend_number(tox, client_id);
    if(num<0) {
        lua_pushnil(L);
        lua_pushstring(L, "Friend not found");
    }
    else {
        lua_pushnumber(L, num);
        lua_pushnil(L);
    }
    return 2;
}

int lua_tox_get_client_id(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = (int32_t)luaL_checknumber(L,2);
    lua_settop(L,0);

    uint8_t client_id[TOX_CLIENT_ID_SIZE];
    if( tox_get_client_id(tox, friendnumber, client_id) < 0 ) {
        lua_pushnil(L);
        lua_pushstring(L, "Can't get client id");
    }
    else {
        lua_pushlstring(L, client_id, TOX_CLIENT_ID_SIZE);
        lua_pushnil(L);
    }
    return 2;
}

int lua_tox_del_friend(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    int r = tox_del_friend(tox, friendnumber);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_get_friend_connection_status(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    
    int r = tox_get_friend_connection_status(tox, friendnumber);
    lua_pushnumber(L,r);
    return 1;
}

int lua_tox_friend_exists(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    
    int r = tox_get_friend_connection_status(tox, friendnumber);
    lua_pushboolean(L, (r==1));

    return 1;
}

int lua_tox_send_message(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    size_t len;
    uint8_t *message = (uint8_t*)luaL_checklstring(L,3,&len);
    lua_settop(L,0);

    int r = tox_send_message(tox, friendnumber, message, len);
    if(r==0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,r);
    return 1;
}

int lua_tox_send_message_withid(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    uint32_t theid = luaL_checknumber(L,3);
    size_t len;
    uint8_t *message = (uint8_t*)luaL_checklstring(L,4,&len);
    lua_settop(L,0);

    int r = tox_send_message_withid(tox, friendnumber, theid, message, len);
    if(r==0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,r);

    return 1;
}

int lua_tox_send_action(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    size_t len;
    uint8_t *action = (uint8_t*)luaL_checklstring(L,3, &len);
    lua_settop(L,0);

    int r = tox_send_action(tox, friendnumber, action, len);
    if(r==0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,r);

    return 1;
}

int lua_tox_send_action_withid(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    uint32_t theid = luaL_checknumber(L,3);
    size_t len;
    uint8_t *action = (uint8_t*)luaL_checklstring(L,4,&len);
    lua_settop(L,0);

    int r = tox_send_action_withid(tox, friendnumber, theid, action, len);
    if(r==0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,r);

    return 1;
}

int lua_tox_set_name(lua_State* L) {
    Tox *tox = checkTox(L,1);
    size_t len;
    uint8_t *name = (uint8_t*)lua_tolstring(L,2, &len);
    lua_settop(L,0);

    int r = tox_set_name(tox, name, len);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_get_self_name(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint8_t name[TOX_MAX_NAME_LENGTH];
    uint16_t len = tox_get_self_name(tox, name);
    lua_pushlstring(L, name, len);
    return 1;
}

int lua_tox_get_name(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    uint8_t name[TOX_MAX_NAME_LENGTH];
    int len = tox_get_name(tox, friendnumber, name);
    if(len<0)
        lua_pushnil(L);
    else
        lua_pushlstring(L, name, len);
    return 1;
}

int lua_tox_get_name_size(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    int len = tox_get_name_size(tox, friendnumber);
    if(len<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,len);

    return 1;
}

int lua_tox_get_self_name_size(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    int len = tox_get_self_name_size(tox);
    if(len<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,len);

    return 1;
}

int lua_tox_set_status_message(lua_State* L) {
    Tox *tox = checkTox(L,1);
    size_t len;
    uint8_t *status = (uint8_t*)lua_tolstring(L,2, &len);
    lua_settop(L,0);

    int r = tox_set_status_message(tox, status, len);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_set_user_status(lua_State* L) {
    Tox *tox = checkTox(L,1);
    uint8_t status = (uint8_t)luaL_checknumber(L,2);
    lua_settop(L,0);
    int r = tox_set_user_status(tox, status);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_get_status_message_size(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    int len = tox_get_status_message_size(tox, friendnumber);
    if(len<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,len);
    return 1;
}

int lua_tox_get_self_status_message_size(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    int len = tox_get_self_status_message_size(tox);
    if(len<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,len);
    return 1;
}

// should I better return an empty string instead of nil?
int lua_tox_get_status_message(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    uint32_t maxlen = tox_get_status_message_size(tox, friendnumber);
    int r = -1;
    if(maxlen > 0) {
        uint8_t buf[maxlen];
        r = tox_get_status_message(tox, friendnumber, buf, maxlen);
        if(r>0)
            lua_pushlstring(L, buf, r);
        else
            lua_pushnil(L);
    }
    else
        lua_pushnil(L);
    
    return 1;
}

int lua_tox_get_self_status_message(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint32_t maxlen = tox_get_self_status_message_size(tox);
    int r = -1;
    if(maxlen > 0) {
        uint8_t buf[maxlen];
        r = tox_get_self_status_message(tox, buf, maxlen);
        if(r>0)
            lua_pushlstring(L, buf, r);
        else
            lua_pushnil(L);
    }
    else
        lua_pushnil(L);
    return 1;
}

int lua_tox_get_user_status(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    uint8_t status = tox_get_user_status(tox, friendnumber);
    lua_pushnumber(L,status);
    return 1;
}

int lua_tox_get_self_user_status(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint8_t status = tox_get_self_user_status(tox);
    lua_pushnumber(L,status);

    return 1;
}

int lua_tox_get_last_online(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    uint64_t time = tox_get_last_online(tox, friendnumber);
    if(time<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,time);
    return 1;
}

int lua_tox_set_user_is_typing(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    uint8_t is_typing = lua_toboolean(L,3);
    lua_settop(L,0);
    int r = tox_set_user_is_typing(tox, friendnumber, is_typing);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_get_is_typing(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    int r = tox_get_is_typing(tox,friendnumber);
    lua_pushboolean(L, (r==1));
    return 1;
}

int lua_tox_set_sends_receipts(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    int yesno = luaL_checknumber(L,3);
    lua_settop(L,0);
    tox_set_sends_receipts(tox, friendnumber, yesno);
    return 0;
}

int lua_tox_count_friendlist(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint32_t friends = tox_count_friendlist(tox);
    lua_pushnumber(L, friends);
    return 1;
}

int lua_tox_get_num_online_friends(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint32_t friends = tox_get_num_online_friends(tox);
    lua_pushnumber(L, friends);
    return 1;
}

int lua_tox_get_friendlist(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint32_t size = tox_count_friendlist(tox);
    int32_t out[size];
    uint32_t n = tox_get_friendlist(tox, out, size);

    lua_newtable(L);
    for(int i=0;i<n;++i) {
        lua_pushnumber(L,out[i]);
        lua_pushnumber(L,i+1);
        lua_settable(L,-3);
    }
    return 1;
}

int lua_tox_get_nospam(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint32_t nospam = tox_get_nospam(tox);
    // assume 1 for set
    lua_pushboolean(L, (nospam==1));
    return 1;
}

int lua_tox_set_nospam(lua_State* L) {
    Tox *tox = checkTox(L,1);
    uint32_t nospam = luaL_checknumber(L,2);
    lua_settop(L,0);
    tox_set_nospam(tox, nospam);
    return 0;
}

int lua_tox_add_groupchat(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    int r = tox_add_groupchat(tox);
    if(r<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,r);
    return 1;
}

int lua_tox_del_groupchat(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int groupnumber = luaL_checknumber(L,2);
    lua_settop(L,0);

    int r = tox_del_groupchat(tox, groupnumber);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_group_peername(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int groupnumber = luaL_checknumber(L, 2);
    int peernumber = luaL_checknumber(L, 3);
    lua_settop(L,0);
    uint8_t name[TOX_MAX_NAME_LENGTH];
    int len = tox_group_peername(tox, groupnumber, peernumber, name);
    if(len<0)
        lua_pushnil(L);
    else
        lua_pushlstring(L, name, len);
    return 1;
}

int lua_tox_invite_friend(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    int groupnumber = luaL_checknumber(L,3);
    lua_settop(L,0);
    int id = tox_invite_friend(tox, friendnumber, groupnumber);
    if(id<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L, id);
    return 1;
}

int lua_tox_join_groupchat(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L,2);
    uint8_t *friend_group_public_key = (uint8_t*)luaL_checkstring(L,3);
    lua_settop(L,0);

    int id = tox_join_groupchat(tox, friendnumber, friend_group_public_key);
    if(id<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,id);
    return 1;
}

int lua_tox_group_message_send(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t groupnumber = luaL_checknumber(L,2);
    size_t len;
    uint8_t *message = (uint8_t*)luaL_checklstring(L,3, &len);
    lua_settop(L,0);
    int r = tox_group_message_send(tox, groupnumber, message, len);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_group_action_send(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int groupnumber = luaL_checknumber(L,2);
    size_t len;
    uint8_t *action = (uint8_t*)luaL_checklstring(L,3, &len);
    lua_settop(L,0);
    int r = tox_group_action_send(tox, groupnumber, action, len);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_group_number_peers(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int groupnumber = luaL_checknumber(L,2);
    lua_settop(L,0);
    int n = tox_group_number_peers(tox, groupnumber);
    if(n<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,n);
    return 1;
}

int lua_tox_group_get_names(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int groupnumber = luaL_checknumber(L,2);
    lua_settop(L,0);

    int nb = tox_group_number_peers(tox, groupnumber);
    uint8_t names[nb][TOX_MAX_NAME_LENGTH];
    uint16_t lengths[nb];

    int nb_peers = tox_group_get_names(tox, groupnumber, names, lengths, nb);

    if(nb_peers<0)
        lua_pushnil(L);
    else {
        lua_newtable(L);
        for(int i=0;i<nb_peers;++i) {
            lua_pushlstring(L, names[i], lengths[i]);
            lua_pushnumber(L, i+1);
            lua_settable(L,-3);
        }
    }
    return 1;
}

int lua_tox_count_chatlist(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint32_t n = tox_count_chatlist(tox);
    lua_pushnumber(L,n);
    return 1;
}

int lua_tox_get_chatlist(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint32_t n = tox_count_chatlist(tox);
    int out[n];
    uint32_t r = tox_get_chatlist(tox, out, n);
    
    lua_newtable(L);
    for(int i=0;i<r;++i) {
        lua_pushnumber(L, out[i]);
        lua_pushnumber(L, i+1);
        lua_settable(L, -3);
    }
    return 1;
}

int lua_tox_new_file_sender(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L, 2);
    uint64_t filesize = (uint64_t)luaL_checknumber(L, 3);
    size_t len;
    uint8_t *filename = (uint8_t*)luaL_checklstring(L, 4, &len);
    lua_settop(L,0);

    int filenumber = tox_new_file_sender(tox, friendnumber, filesize, filename, len);
    if(filenumber<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,filenumber);
    return 1;
}

int lua_tox_file_send_control(lua_State* L) {
    Tox *tox = checkTox(L, 1);
    int32_t friendnumber = luaL_checknumber(L, 2);
    uint8_t send_receive = luaL_checknumber(L, 3);
    uint8_t filenumber = luaL_checknumber(L, 4);
    uint8_t message_id = luaL_checknumber(L, 5);
    size_t len;
    uint8_t *data = (uint8_t*)lua_tolstring(L, 6, &len);
    lua_settop(L,0);

    int r = tox_file_send_control(tox, friendnumber, send_receive, filenumber, message_id, data, len);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_file_send_data(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L, 2);
    uint8_t filenumber = luaL_checknumber(L, 3);
    uint8_t length = -1;
    size_t len;
    uint8_t *data = (uint8_t*)luaL_checklstring(L, 4, &len);
    if(!lua_isnoneornil(L,5))
        len = luaL_checknumber(L,5);
    lua_settop(L,0);

    int r = tox_file_send_data(tox, friendnumber, filenumber, data, len);
    lua_pushboolean(L, (r==0));
    return 1;
}

int lua_tox_file_data_size(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L, 2);
    lua_settop(L,0);
    int r = tox_file_data_size(tox, friendnumber);
    if(r<0)
        lua_pushnil(L);
    else
        lua_pushnumber(L,r);
    return 1;
}

int lua_tox_file_data_remaining(lua_State* L) {
    Tox *tox = checkTox(L,1);
    int32_t friendnumber = luaL_checknumber(L, 2);
    uint8_t filenumber = luaL_checknumber(L, 3);
    uint8_t send_receive = luaL_checknumber(L, 4);
    lua_settop(L,0);
    uint64_t r = tox_file_data_remaining(tox, friendnumber, filenumber, send_receive);
    lua_pushnumber(L, r);
    return 1;
}


int lua_tox_bootstrap_from_address(lua_State* L) {
    Tox *tox = checkTox(L,1);
    const char *address = luaL_checkstring(L,2);
    uint8_t ipv6enabled = luaL_checknumber(L,3);
    uint16_t port = luaL_checknumber(L,4);
    uint8_t *public_key = (uint8_t*)luaL_checkstring(L, 5);
    lua_settop(L,0);
    int r = tox_bootstrap_from_address(tox, address, ipv6enabled, port, public_key);
    lua_pushboolean(L, (r==1));
    return 1;
}

int lua_tox_isconnected(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    int r = tox_isconnected(tox);
    lua_pushboolean(L, (r==1));
    return 1;
}

int lua_tox_do_interval(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    lua_pushnumber(L, tox_do_interval(tox));
    return 1;
}

int lua_tox_do(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    tox_do(tox);
    return 0;
}

int lua_tox_size(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    lua_pushnumber(L, tox_size(tox));
    return 1;
}

int lua_tox_save(lua_State* L) {
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    uint32_t size = tox_size(tox);
    uint8_t data[size];
    tox_save(tox, data);
    lua_pushlstring(L, data, size);
    return 1;
}

int lua_tox_load(lua_State* L) {
    Tox *tox = checkTox(L,1);
    size_t len;
    uint8_t *data = (uint8_t*)luaL_checklstring(L, 2, &len);
    lua_settop(L,0);
    int r = tox_load(tox, data, len);
    lua_pushboolean(L, (r==0));
    return 1;
}

//int lua_tox_new(lua_State* L);

//int lua_tox_kill(lua_State* L);

/************************************
 *                                  *
 * lua module loader and destructor *
 *                                  *
 ************************************/

int lua_tox_gc(lua_State* L) {
    // not sure how it works yet
    // maybe because of an error with using lua userdata ?
    // segfault anyway
    Tox *tox = checkTox(L,1);
    lua_settop(L,0);
    if(tox!=NULL)
        tox_kill( tox );
    return 0;
}

static int lua_tox_tostring(lua_State* L) {
    char buf[32];
    sprintf(buf, "%p", checkTox(L,1));
    lua_pushfstring(L, "Tox (%s)", buf);
    return 1;
}

int lua_tox_new(lua_State* L) {
    uint8_t ipv6enabled;
    if(lua_isnumber(L,1)) {
        ipv6enabled = (uint8_t)lua_tonumber(L,1);
    }
    else
        ipv6enabled = TOX_ENABLE_IPV6_DEFAULT;
    lua_settop(L,0);

    LTox *ltox = pushTox(L, ipv6enabled);
    ltox->callbacks.friend_request = 0;
    ltox->callbacks.friend_message = 0;
    ltox->callbacks.friend_action = 0;
    ltox->callbacks.name_change = 0;
    ltox->callbacks.status_message = 0;
    ltox->callbacks.user_status = 0;
    ltox->callbacks.typing_change = 0;
    ltox->callbacks.read_receipt = 0;
    ltox->callbacks.connection_status = 0;
    ltox->callbacks.group_invite = 0;
    ltox->callbacks.group_message = 0;
    ltox->callbacks.group_action = 0;
    ltox->callbacks.group_namelist_change = 0;
    ltox->callbacks.file_send_request = 0;
    ltox->callbacks.file_control = 0;
    ltox->callbacks.file_data = 0;

    reg(L, ltox);
    return 1;
}

// TODO: improve: set methods only when new is called
static const luaL_Reg tox_methods[] = {
    {"new", lua_tox_new},

    {"getAddress", lua_tox_get_address},
    {"addFriend", lua_tox_add_friend},
    {"addFriendNorequest", lua_tox_add_friend_norequest},
    {"getFriendNumber", lua_tox_get_friend_number},
    {"getClientId", lua_tox_get_client_id},
    {"delFriend", lua_tox_del_friend},
    {"getFriendConnectionStatus", lua_tox_get_friend_connection_status},
    {"friendExists", lua_tox_friend_exists},
    {"sendMessage", lua_tox_send_message},
    {"sendMessageWithid", lua_tox_send_message_withid},
    {"sendAction", lua_tox_send_action},
    {"sendActionWithid", lua_tox_send_action_withid},
    {"setName", lua_tox_set_name},
    {"getSelfName", lua_tox_get_self_name},
    {"getName", lua_tox_get_name},
    {"getNameSize", lua_tox_get_name_size},
    {"getSelfNameSize", lua_tox_get_self_name_size},
    {"setStatusMessage", lua_tox_set_status_message},
    {"setUserStatus", lua_tox_set_user_status},
    {"getStatusMessageSize", lua_tox_get_status_message_size},
    {"getSelfStatusMessageSize", lua_tox_get_self_status_message_size},
    {"getStatusMessage", lua_tox_get_status_message},
    {"getSelfStatusMessage", lua_tox_get_self_status_message},
    {"getUserStatus", lua_tox_get_user_status},
    {"getSelfUserStatus", lua_tox_get_self_user_status},
    {"getLastOnline", lua_tox_get_last_online},
    {"setUserIsTyping", lua_tox_set_user_is_typing},
    {"getIsTyping", lua_tox_get_is_typing},
    {"setSendsReceipts", lua_tox_set_sends_receipts},
    {"countFriendlist", lua_tox_count_friendlist},
    {"getNumOnlineFriends", lua_tox_get_num_online_friends},
    {"getFriendlist", lua_tox_get_friendlist},
    {"callbackFriendRequest", lua_tox_callback_friend_request},
    {"callbackFriendMessage", lua_tox_callback_friend_message},
    {"callbackFriendAction", lua_tox_callback_friend_action},
    {"callbackNameChange", lua_tox_callback_name_change},
    {"callbackStatusMessage", lua_tox_callback_status_message},
    {"callbackUserStatus", lua_tox_callback_user_status},
    {"callbackTypingChange", lua_tox_callback_typing_change},
    {"callbackReadReceipt", lua_tox_callback_read_receipt},
    {"callbackConnectionStatus", lua_tox_callback_connection_status},
    {"getNospam", lua_tox_get_nospam},
    {"setNospam", lua_tox_set_nospam},
    {"callbackGroupInvite", lua_tox_callback_group_invite},
    {"callbackGroupMessage", lua_tox_callback_group_message},
    {"callbackGroupAction", lua_tox_callback_group_action},
    {"callbackGroupNamelistChange", lua_tox_callback_group_namelist_change},
    {"addGroupchat", lua_tox_add_groupchat},
    {"delGroupchat", lua_tox_del_groupchat},
    {"groupPeername", lua_tox_group_peername},
    {"inviteFriend", lua_tox_invite_friend},
    {"joinGroupchat", lua_tox_join_groupchat},
    {"groupMessageSend", lua_tox_group_message_send},
    {"groupActionSend", lua_tox_group_action_send},
    {"groupNumberPeers", lua_tox_group_number_peers},
    {"groupGetNames", lua_tox_group_get_names},
    {"countChatlist", lua_tox_count_chatlist},
    {"getChatlist", lua_tox_get_chatlist},
    {"callbackFileSendRequest", lua_tox_callback_file_send_request},
    {"callbackFileControl", lua_tox_callback_file_control},
    {"callbackFileData", lua_tox_callback_file_data},
    {"newFileSender", lua_tox_new_file_sender},
    {"fileSendControl", lua_tox_file_send_control},
    {"fileSendData", lua_tox_file_send_data},
    {"fileDataSize", lua_tox_file_data_size},
    {"fileDataRemaining", lua_tox_file_data_remaining},
    {"bootstrapFromAddress", lua_tox_bootstrap_from_address},
    {"isConnected", lua_tox_isconnected},

    {"toxDoInterval", lua_tox_do_interval},
    {"toxDo", lua_tox_do},
    {"size", lua_tox_size},
    {"save", lua_tox_save},
    {"load", lua_tox_load},

    {NULL,NULL}
};

static const luaL_Reg tox_meta[] = {
    {"__gc", lua_tox_gc},
    {"__tostring", lua_tox_tostring},
    {NULL,NULL}
};

#define TOX_CLASS "ToxClass"
static const luaL_Reg class_meta[] = {
    {"__call", lua_tox_new},
    {NULL,NULL}
};

int lua_tox_register(lua_State* L) {
    Ls = L;
    lua_newtable(L);
    // lua 5.2's luaL_setfuncs light emulation
    for(int f = 0; tox_methods[f].name != NULL; ++f) {
        lua_pushstring(L, tox_methods[f].name);
        lua_pushcclosure(L, tox_methods[f].func, 0);
        lua_settable(L, -3);
    }
    // SEND, RECEIVE
    lua_pushnumber(L, 0);
    lua_setfield(L, -2, "SEND");
    lua_pushnumber(L, 1);
    lua_setfield(L, -2, "RECEIVE");

    // status
    lua_pushstring(L, "status");
    lua_newtable(L);
    lua_pushstring(L,"NONE");
    lua_pushnumber(L,TOX_USERSTATUS_NONE);
    lua_settable(L,-3);
    lua_pushstring(L,"AWAY");
    lua_pushnumber(L,TOX_USERSTATUS_AWAY);
    lua_settable(L,-3);
    lua_pushstring(L,"BUSY");
    lua_pushnumber(L,TOX_USERSTATUS_BUSY);
    lua_settable(L,-3);
    lua_pushstring(L,"INVALID");
    lua_pushnumber(L,TOX_USERSTATUS_INVALID);
    lua_settable(L,-3);
    lua_settable(L,-3);
 
    // chat status
    lua_pushstring(L, "chat");
    lua_newtable(L);
    lua_pushstring(L,"ADD");
    lua_pushnumber(L,TOX_CHAT_CHANGE_PEER_ADD);
    lua_settable(L,-3);
    lua_pushstring(L,"DEL");
    lua_pushnumber(L,TOX_CHAT_CHANGE_PEER_DEL);
    lua_settable(L,-3);
    lua_pushstring(L,"NAME");
    lua_pushnumber(L,TOX_CHAT_CHANGE_PEER_NAME);
    lua_settable(L,-3);
    lua_settable(L,-3);

    // file control
    lua_pushstring(L, "control");
    lua_newtable(L);
    lua_pushstring(L,"ACCEPT");
    lua_pushnumber(L,TOX_FILECONTROL_ACCEPT);
    lua_settable(L,-3);
    lua_pushstring(L,"PAUSE");
    lua_pushnumber(L,TOX_FILECONTROL_PAUSE);
    lua_settable(L,-3);
    lua_pushstring(L,"KILL");
    lua_pushnumber(L,TOX_FILECONTROL_KILL);
    lua_settable(L,-3);
    lua_pushstring(L,"FINISHED");
    lua_pushnumber(L,TOX_FILECONTROL_FINISHED);
    lua_settable(L,-3);
    lua_pushstring(L,"RESUME_BROKEN");
    lua_pushnumber(L,TOX_FILECONTROL_RESUME_BROKEN);
    lua_settable(L,-3);
    lua_settable(L,-3);
 

    // errors
    lua_pushstring(L, "errors");
    lua_newtable(L);
    lua_pushstring(L, "TOOLONG");
    lua_pushnumber(L, TOX_FAERR_TOOLONG);
    lua_settable(L, -3);
    lua_pushstring(L, "NOMESSAGE");
    lua_pushnumber(L, TOX_FAERR_NOMESSAGE);
    lua_settable(L, -3);
    lua_pushstring(L, "OWNKEY");
    lua_pushnumber(L, TOX_FAERR_OWNKEY);
    lua_settable(L, -3);
    lua_pushstring(L, "ALREADYSENT");
    lua_pushnumber(L, TOX_FAERR_ALREADYSENT);
    lua_settable(L, -3);
    lua_pushstring(L, "UNKNOWN");
    lua_pushnumber(L, TOX_FAERR_UNKNOWN);
    lua_settable(L, -3);
    lua_pushstring(L, "BADCHECKSUM");
    lua_pushnumber(L, TOX_FAERR_BADCHECKSUM);
    lua_settable(L, -3);
    lua_pushstring(L, "SETNEWNOSPAM");
    lua_pushnumber(L, TOX_FAERR_SETNEWNOSPAM);
    lua_settable(L, -3);
    lua_pushstring(L, "NOMEM");
    lua_pushnumber(L, TOX_FAERR_NOMEM);
    lua_settable(L, -3);
    lua_settable(L, -3);


    luaL_newmetatable(L, TOX_STR);
    for(int f = 0; tox_meta[f].name != NULL; ++f) {
        lua_pushstring(L, tox_meta[f].name);
        lua_pushcclosure(L, tox_meta[f].func, 0);
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
    luaL_newmetatable(L, TOX_CLASS);
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

int luaopen_tox(lua_State* L) {
    return lua_tox_register(L);
}

#ifdef __cplusplus
}
#endif
