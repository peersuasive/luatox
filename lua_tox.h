/* lua_tox.h
 *
 * Lua binding for Tox.
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

#ifndef LUA_TOX_H
#define LUA_TOX_H

#define MAX_STR_SIZE 256

#include <stdint.h>

#include "lua_common.h"

typedef struct _callbacks_t {
    int friend_request;
    int friend_message;
    int friend_action;
    int name_change;
    int status_message;
    int user_status;
    int typing_change;
    int read_receipt;
    int connection_status;
    int group_invite;
    int group_message;
    int group_action;
    int group_namelist_change;
    int file_send_request;
    int file_control;
    int file_data;
} callbacks_t;

/*
enum callback_n {
    friend_request,
    friend_message,
    friend_action,
    name_change,
    status_message,
    user_status,
    typing_change,
    read_receipt,
    connection_status,
    group_invite,
    group_message,
    group_action,
    group_namelist_change,
    file_send_request,
    file_control,
    file_data
};

static const char *callback_nanes[] = {
    "friend_request",
    "friend_message",
    "friend_action",
    "name_change",
    "status_message",
    "user_status",
    "typing_change",
    "read_receipt",
    "connection_status",
    "group_invite",
    "group_message",
    "group_action",
    "group_namelist_change",
    "file_send_request",
    "file_control",
    "file_data"
};
*/

#include "tox/tox.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TOX_STR "Tox"
typedef struct _LTox {
    Tox *tox;
    callbacks_t callbacks;
} LTox;

int lua_tox_get_address(lua_State*);
int lua_tox_add_friend(lua_State*);
int lua_tox_add_friend_norequest(lua_State*);
int lua_tox_get_friend_number(lua_State*);
int lua_tox_get_client_id(lua_State*);
int lua_tox_del_friend(lua_State*);
int lua_tox_get_friend_connection_status(lua_State*);
int lua_tox_friend_exists(lua_State*);
int lua_tox_send_message(lua_State*);
int lua_tox_send_action(lua_State*);
int lua_tox_set_name(lua_State*);
int lua_tox_get_self_name(lua_State*);
int lua_tox_get_name(lua_State*);
int lua_tox_get_name_size(lua_State*);
int lua_tox_get_self_name_size(lua_State*);
int lua_tox_set_status_message(lua_State*);
int lua_tox_set_user_status(lua_State*);
int lua_tox_get_status_message_size(lua_State*);
int lua_tox_get_self_status_message_size(lua_State*);
int lua_tox_get_status_message(lua_State*);
int lua_tox_get_self_status_message(lua_State*);
int lua_tox_get_user_status(lua_State*);
int lua_tox_get_self_user_status(lua_State*);

int lua_tox_get_last_online(lua_State*);
int lua_tox_set_user_is_typing(lua_State*);
int lua_tox_get_is_typing(lua_State*);
int lua_tox_count_friendlist(lua_State*);
int lua_tox_get_num_online_friends(lua_State*);
int lua_tox_get_friendlist(lua_State*);

int lua_tox_callback_friend_request(lua_State*);
int lua_tox_callback_friend_message(lua_State*);
int lua_tox_callback_friend_action(lua_State*);
int lua_tox_callback_name_change(lua_State*);
int lua_tox_callback_status_message(lua_State*);
int lua_tox_callback_user_status(lua_State*);
int lua_tox_callback_typing_change(lua_State*);
int lua_tox_callback_read_receipt(lua_State*);
int lua_tox_callback_connection_status(lua_State*);

int lua_tox_get_nospam(lua_State*);
int lua_tox_set_nospam(lua_State*);

int lua_tox_callback_group_invite(lua_State*);
int lua_tox_callback_group_message(lua_State*);
int lua_tox_callback_group_action(lua_State*);
int lua_tox_callback_group_namelist_change(lua_State*);

int lua_tox_add_groupchat(lua_State*);
int lua_tox_del_groupchat(lua_State*);
int lua_tox_group_peername(lua_State*);
int lua_tox_invite_friend(lua_State*);
int lua_tox_join_groupchat(lua_State*);
int lua_tox_group_message_send(lua_State*);
int lua_tox_group_action_send(lua_State*);
int lua_tox_group_number_peers(lua_State*);
int lua_tox_group_get_names(lua_State*);
int lua_tox_count_chatlist(lua_State*);
int lua_tox_get_chatlist(lua_State*);

int lua_tox_callback_file_send_request(lua_State*);
int lua_tox_callback_file_control(lua_State*);
int lua_tox_callback_file_data(lua_State*);

int lua_tox_new_file_sender(lua_State*);
int lua_tox_file_send_control(lua_State*);
int lua_tox_file_send_data(lua_State*);
int lua_tox_file_data_size(lua_State*);
int lua_tox_file_data_remaining(lua_State*);

int lua_tox_bootstrap_from_address(lua_State*);
int lua_tox_isconnected(lua_State*);
int lua_tox_new(lua_State*);
int lua_tox_kill(lua_State*);
int lua_tox_do_interval(lua_State*);
int lua_tox_do(lua_State*);
int lua_tox_size(lua_State*);
int lua_tox_save(lua_State*);
int lua_tox_load(lua_State*);

#ifdef __cplusplus
}
#endif

#endif // LUA_TOX_H
