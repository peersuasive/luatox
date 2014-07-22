/* lua_toxav.h
 *
 * Lua binding for ToxAV.
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

#ifndef LUA_TOXAV_H
#define LUA_TOXAV_H

#include "tox/toxav.h"
#include "lua_common.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define TOX_STR "Tox"
#define TOXAV_STR "ToxAv"

typedef struct _callbacks_t {
    int on_invite;
    int on_start;
    int on_cancel;
    int on_reject;
    int on_end;
    int on_ringing;
    int on_starting;
    int on_ending;
    //int on_error;
    int on_request_timeout;
    int on_peer_timeout;
    int on_audio_recv;
    int on_video_recv;
    int on_media_change;
} callbacks_t;

typedef struct _LTox {
    Tox *tox;
    callbacks_t callbacks;
} LTox;

struct _Call;
typedef struct _LToxAv {
    ToxAv *av;
    int max_calls;
    ToxAvCodecSettings settings;
    struct _Call *calls;
    int nb_calls;
    callbacks_t callbacks;
} LToxAv;

typedef struct _HandleCall {
    LToxAv *lav;
    uint32_t call_index;
    int16_t *frame;
    int16_t *dec_frame;
    int frame_size;
    int from_string;
    uint8_t payload[RTP_PAYLOAD_SIZE];
    // TODO: add video
} HandleCall;

// FIXME: use it to maintain a list of calls
// or clean the code
typedef struct _Call {
    int call_index;
    HandleCall *handle;
} Call;


int setSettings(lua_State*);
int getSettings(lua_State*);
int setVideoBitrate(lua_State*);
int getVideoBitrate(lua_State*);
int setVideoWidth(lua_State*);
int getVideoWidth(lua_State*);
int setVideoHeight(lua_State*);
int getVideoHeight(lua_State*);
int setAudioBitrate(lua_State*);
int getAudioBitrate(lua_State*);
int setAudioFrame_duration(lua_State*);
int getAudioFrame_duration(lua_State*);
int setAudioSample_rate(lua_State*);
int getAudioSample_rate(lua_State*);
int setAudioChannels(lua_State*);
int getAudioChannels(lua_State*);
int setJbufCapacity(lua_State*);
int getJbufCapacity(lua_State*);

int lua_toxav_new(lua_State*);

int lua_toxav_call(lua_State*);
int lua_toxav_hangup(lua_State*);
int lua_toxav_answer(lua_State*);
int lua_toxav_reject(lua_State*);
int lua_toxav_cancel(lua_State*);
int lua_toxav_stop_call(lua_State*);
int lua_toxav_prepare_transmission(lua_State*);
int lua_toxav_kill_transmission(lua_State*);
int lua_toxav_recv_video (lua_State*);
int lua_toxav_recv_audio(lua_State*);
int lua_toxav_send_video (lua_State*);
int lua_toxav_send_audio (lua_State*);
int lua_toxav_prepare_video_frame (lua_State*);
int lua_toxav_prepare_audio_frame (lua_State*);
int lua_toxav_get_peer_transmission_type (lua_State*);
int lua_toxav_get_peer_id (lua_State*);
int lua_toxav_capability_supported (lua_State*);
int lua_toxav_set_audio_queue_limit (lua_State*);
int lua_toxav_set_video_queue_limit (lua_State*);

int lua_toxav_get_call_state(lua_State*);
int lua_toxav_change_type(lua_State*);

int lua_toxav_get_tox(lua_State*);

#endif /* LUA_TOXAV_H */
