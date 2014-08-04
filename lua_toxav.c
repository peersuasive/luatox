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

#include <stdlib.h>
#include <string.h>


#include "lua_toxav.h"

#ifdef __cplusplus
extern "C" {
#endif

/**************************
 *                        *
 * lua module functions   *
 *                        *
 **************************/

// TODO: retourner le call_index en userdata
// et associer au moins la méthode getCallId

static ToxAv *toToxAv(lua_State* L, int index) {
    ToxAv *av = (ToxAv*)lua_touserdata(L, index);
    if(av==NULL)
        luaL_typerror(L, index, TOXAV_STR);
    return av;
}

static ToxAv *checkToxAv(lua_State* L, int index) {
    luaL_checktype(L, index, LUA_TUSERDATA);
    LToxAv *lav = (LToxAv*)luaL_checkudata(L, index, TOXAV_STR);
    ToxAv *av = lav->av;
    if(av==NULL)
        luaL_typerror(L, index, TOXAV_STR);
    return av;
}
static LToxAv *checkLToxAv(lua_State* L, int index) {
    luaL_checktype(L, index, LUA_TUSERDATA);
    LToxAv *lav = (LToxAv*)luaL_checkudata(L, index, TOXAV_STR);
    if(lav->av==NULL)
        luaL_typerror(L, index, TOXAV_STR);
    return lav;
}
static LToxAv *toLToxAv(lua_State* L, int index) {
    LToxAv *lav = (LToxAv*)luaL_checkudata(L, index, TOXAV_STR);
    return lav;
}

static LTox *checkLTox(lua_State* L, int index) {
    luaL_checktype(L, index, LUA_TUSERDATA);
    LTox *ltox = (LTox*)luaL_checkudata(L, index, TOX_STR);
    if(ltox->tox==NULL)
        luaL_typerror(L, index, TOX_STR);
    return ltox;
}

static LToxAv *pushToxAv(lua_State* L, Tox *tox, int32_t max_calls) {
    LToxAv *lav = (LToxAv*)lua_newuserdata(L, sizeof(LToxAv));
    lav->av = toxav_new(tox, max_calls);
    lav->settings = av_DefaultSettings;
    if(lav->av==NULL) {
        lua_pushstring(L, "Can't create new tox av!");
        lua_error(L);
    }
    luaL_getmetatable(L, TOXAV_STR);
    lua_setmetatable(L, -2);
    return lav;
}

int throw_error(lua_State *L, int32_t err) {
    lua_pushnil(L);

    switch(err) {
        case ErrorNone:
            lua_pushliteral(L, "Size was 0.");
            break;

        case ErrorAlreadyInCall:
            lua_pushliteral(L, "Already has an active call.");
            break;

        case ErrorNoCall:
            lua_pushliteral(L, "Trying to perform call action while not in a call.");
            break;

        case ErrorInvalidState:
            lua_pushliteral(L, "Trying to perform call action while in invalid.");
            break;

        case ErrorNoRtpSession:
            lua_pushliteral(L, "Trying to perform rtp action on invalid session.");
            break;

        case ErrorAudioPacketLost:
            lua_pushliteral(L, "Packet loss.");
            break;

        case ErrorStartingAudioRtp:
            lua_pushliteral(L, "Error in toxav_prepare_transmission().");
            break;

        case ErrorStartingVideoRtp:
            lua_pushliteral(L, "Error in toxav_prepare_transmission().");
            break;

        case ErrorTerminatingAudioRtp:
            lua_pushliteral(L, "Error in toxav_kill_transmission().");
            break;

        case ErrorTerminatingVideoRtp:
            lua_pushliteral(L, "Error in toxav_kill_transmission().");
            break;

        case ErrorPacketTooLarge:
            lua_pushliteral(L, "Buffer exceeds size while encoding.");
            break;

        case ErrorInvalidCodecState:
            lua_pushliteral(L, "Codec state not initialized.");
            break;

        case ErrorInternal:
        default:
            lua_pushliteral(L, "Internal error.");
            break;
    }
    return 2;
}

/*************************
 *                       *
 * Video/Audio settings  *
 *                       *
 *************************/

static ToxAvCSettings *lua_get_settings(lua_State *L, int index) {
    LToxAv *lav = toLToxAv(L,1);
    return &lav->settings;
}

int setSettings(lua_State *L) {
    lua_pushstring(L, "setSettings: No yet implemented");
    lua_error(L);

    ToxAvCSettings *settings = lua_get_settings(L, 1);
    // TODO: get values from a table
    lua_settop(L,0);
    return 0;
}
int getSettings(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L, 1);
    lua_settop(L,0);
    lua_newtable(L);
    lua_pushstring(L, "videoBitrate");
    lua_pushnumber(L, settings->video_bitrate);
    lua_settable(L,-3);
    lua_pushstring(L, "videoWidth");
    lua_pushnumber(L, settings->max_video_width);
    lua_settable(L,-3);
    lua_pushstring(L, "videoHeight");
    lua_pushnumber(L, settings->max_video_height);
    lua_settable(L,-3);
    lua_pushstring(L, "audioBitrate");
    lua_pushnumber(L, settings->audio_bitrate);
    lua_settable(L,-3);
    lua_pushstring(L, "audioFrameDuration");
    lua_pushnumber(L, settings->audio_frame_duration);
    lua_settable(L,-3);
    lua_pushstring(L, "audioSampleRate");
    lua_pushnumber(L, settings->audio_sample_rate);
    lua_settable(L,-3);
    lua_pushstring(L, "audioChannels");
    lua_pushnumber(L, settings->audio_channels);
    lua_settable(L,-3);

    // not declared in settings
    lua_pushstring(L, "jbufCapacity");
    lua_pushnumber(L, av_jbufdc);
    lua_settable(L,-3);
    lua_pushstring(L, "VADTolerance");
    lua_pushnumber(L, av_VADd);
    lua_settable(L,-3);

    return 1;
}

int setVideoBitrate(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L, 1);
    uint32_t bitrate = luaL_checknumber(L,2);
    lua_settop(L,0);
    settings->video_bitrate = bitrate;
    return 0;
}
int getVideoBitrate(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L, 1);
    lua_settop(L,0);
    lua_pushnumber(L, settings->video_bitrate);
    return 1;
}

int setVideoWidth(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,1);
    uint16_t max_video_width = luaL_checknumber(L,2);
    lua_settop(L,0);
    settings->max_video_width = max_video_width;
    return 0;
}
int getVideoWidth(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,2);
    lua_settop(L,0);
    lua_pushnumber(L, settings->max_video_width);
    return 1;
}

int setVideoHeight(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,1);
    uint16_t max_video_height = luaL_checknumber(L,2);
    lua_settop(L,0);
    settings->max_video_height = max_video_height;
    return 0;
}
int getVideoHeight(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,2);
    lua_settop(L,0);
    lua_pushnumber(L, settings->max_video_height);
    return 1;
}

int setAudioBitrate(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,1);
    uint32_t audio_bitrate = luaL_checknumber(L,2);
    lua_settop(L,0);
    settings->audio_bitrate = audio_bitrate;
    return 0;
}
int getAudioBitrate(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,2);
    lua_settop(L,0);
    lua_pushnumber(L, settings->audio_bitrate);
    return 1;
}

int setAudioFrame_duration(lua_State *L) {
    LToxAv *lav = checkLToxAv(L, 1);
    uint16_t audio_frame_duration = luaL_checknumber(L,2);
    lua_settop(L,0);
    if( audio_frame_duration != lav->settings.audio_frame_duration)
        lav->settings.audio_frame_duration = audio_frame_duration;
    return 0;
}
int getAudioFrame_duration(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,2);
    lua_settop(L,0);
    lua_pushnumber(L, settings->audio_frame_duration);
    return 1;
}

int setAudioSample_rate(lua_State *L) {
    //ToxAvCSettings *settings = lua_get_settings(L,1);
    LToxAv *lav = checkLToxAv(L, 1);
    uint32_t audio_sample_rate = luaL_checknumber(L,2);
    lua_settop(L,0);
    if( audio_sample_rate != lav->settings.audio_sample_rate)
        lav->settings.audio_sample_rate = audio_sample_rate;
    return 0;
}
int getAudioSample_rate(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,2);
    lua_settop(L,0);
    lua_pushnumber(L, settings->audio_sample_rate);
    return 1;
}

int setAudioChannels(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,1);
    uint32_t audio_channels = luaL_checknumber(L,2);
    lua_settop(L,0);
    settings->audio_channels = audio_channels;
    return 0;
}
int getAudioChannels(lua_State *L) {
    ToxAvCSettings *settings = lua_get_settings(L,2);
    lua_settop(L,0);
    lua_pushnumber(L, settings->audio_channels);
    return 1;
}

// not in settings
int getJbufCapacity(lua_State *L) {
    lua_pushnumber(L, av_jbufdc);
    return 1;
}

int getVADTolerence(lua_State *L) {
    lua_pushnumber(L, av_VADd);
    return 1;
}


/*************************
 *                       *
 * ToxAv callbacks       *
 *                       *
 *************************/

// TODO: API changes now gives *av, avoid using arg and use *av instead

// when invited, call_index is provided here
void callback_OnInvite(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_invite) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnInvite", 0, 1);
    }
}

void callback_OnStart(void *av, int32_t call_index, void* arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_start) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnStart", 0, 1);
    }
}

void callback_OnCancel(void *av, int32_t call_index, void* arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_cancel) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnCancel", 0, 1);
    }
}

void callback_OnReject(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_reject) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnReject", 0, 1);
    }
}

void callback_OnEnd(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_end) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnEnd", 0, 1);
    }
}

void callback_OnRinging(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_ringing) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnRinging", 0, 1);
    }
}

void callback_OnStarting(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_starting) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnStarting", 0, 1);
    }
}

void callback_OnEnding(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_ending) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnEnding", 0, 1);
    }
}

/* API changes -- replaced by what ?
void callback_OnError(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_error) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnError", 0, 1);
    }
}
*/

void callback_OnRequestTimeout(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_request_timeout) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnRequestTimeout", 0, 1);
    }
}

void callback_OnPeerTimeout(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_peer_timeout) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnPeerTimeout", 0, 1);
    }
}

void callback_OnMediaChange(void *av, int32_t call_index, void *arg) {
    LToxAv *lav = (LToxAv*)arg;
    if(lav->callbacks.on_media_change) {
        lua_pushnumber(Ls, call_index);
        call_cb(Ls, lav, "OnMediaChange", 0, 1);
    }
}

int lua_toxav_register_callstate_callback(lua_State* L) {
    LToxAv *lav = checkLToxAv(L,1);
    ToxAvCallbackID cb = (ToxAvCallbackID)luaL_checknumber(L,2);
    switch(cb) {
        case av_OnInvite:
            set(L, lav, "OnInvite", 3);
            lav->callbacks.on_invite = 1;
            toxav_register_callstate_callback(lav->av, callback_OnInvite, av_OnInvite, lav);
            break;
        case av_OnStart:
            set(L, lav, "OnStart", 3);
            lav->callbacks.on_start = 1;
            toxav_register_callstate_callback(lav->av, callback_OnStart, av_OnStart, lav);
            break;
        case av_OnCancel:
            set(L, lav, "OnCancel", 3);
            lav->callbacks.on_cancel = 1;
            toxav_register_callstate_callback(lav->av, callback_OnCancel, av_OnCancel, lav);
            break;
        case av_OnReject:
            set(L, lav, "OnReject", 3);
            lav->callbacks.on_reject = 1;
            toxav_register_callstate_callback(lav->av, callback_OnReject, av_OnReject, lav);
            break;
        case av_OnEnd:
            set(L, lav, "OnEnd", 3);
            lav->callbacks.on_end = 1;
            toxav_register_callstate_callback(lav->av, callback_OnEnd, av_OnEnd, lav);
            break;
        case av_OnRinging:
            set(L, lav, "OnRinging", 3);
            lav->callbacks.on_ringing = 1;
            toxav_register_callstate_callback(lav->av, callback_OnRinging, av_OnRinging, lav);
            break;
        case av_OnStarting:
            set(L, lav, "OnStarting", 3);
            lav->callbacks.on_starting = 1;
            toxav_register_callstate_callback(lav->av, callback_OnStarting, av_OnStarting, lav);
            break;
        case av_OnEnding:
            set(L, lav, "OnEnding", 3);
            lav->callbacks.on_ending = 1;
            toxav_register_callstate_callback(lav->av, callback_OnEnding, av_OnEnding, lav);
            break;
        //case av_OnError:
        //    set(L, lav, "OnError", 3);
        //    lav->callbacks.on_error = 1;
        //    toxav_register_callstate_callback(lav->av, callback_OnError, av_OnError, lav);
        //    break;
        case av_OnRequestTimeout:
            set(L, lav, "OnRequestTimeout", 3);
            lav->callbacks.on_request_timeout = 1;
            toxav_register_callstate_callback(lav->av, callback_OnRequestTimeout, av_OnRequestTimeout, lav);
            break;
        case av_OnPeerTimeout:
            set(L, lav, "OnPeerTimeout", 3);
            lav->callbacks.on_peer_timeout = 1;
            toxav_register_callstate_callback(lav->av, callback_OnPeerTimeout, av_OnPeerTimeout, lav);
            break;
        case av_OnMediaChange:
            set(L, lav, "av_OnMediaChange", 3);
            lav->callbacks.on_media_change = 1;
            toxav_register_callstate_callback(lav->av, callback_OnMediaChange, av_OnMediaChange, lav);
            break;

    }
    lua_settop(L,0);
    return 0;
}

// FIXME: untested
void callback_OnAudioRecv(ToxAv *av, int32_t call_index, int16_t *data, int length, void *userdata) {
    if(retrieve(Ls, av, "OnAudioRecvData")) {
        LToxAv *lav = checkLToxAv(Ls,1);
        lua_pop(Ls,1);

        if(lav->callbacks.on_audio_recv) {
            lua_pushnumber(Ls, call_index);
            call_cb(Ls, lav, "OnAudioRecv", 0, 1);
        }
    }
}
int lua_toxav_audio_recv_callback(lua_State *L) {
    LToxAv *lav = checkLToxAv(L,1);
    set(L, lav, "OnAudioRecv", 2);
    store(L, lav->av, "OnAudioRecvData", lav);
    // TODO: get userdata
    void *userdata = NULL;
    toxav_register_audio_recv_callback(lav->av, callback_OnAudioRecv, userdata);
    lua_settop(L,0);
    return 0;
}

void callback_OnVideoRecv(ToxAv *av, int32_t call_index, vpx_image_t *data, void *userdata) {
    if(retrieve(Ls, av, "OnVideoRecvData")) {
        LToxAv *lav = checkLToxAv(Ls,1);
        lua_pop(Ls,1);

        if(lav->callbacks.on_video_recv) {
            lua_pushnumber(Ls, call_index);
            call_cb(Ls, lav, "OnVideoRecv", 0, 1);
        }
    }
}
int lua_toxav_video_recv_callback(lua_State *L) {
    lua_pushstring(L, "luatoxav: video recv callback: not implemented");
    lua_error(L);

    LToxAv *lav = checkLToxAv(L,1);
    set(L, lav, "OnVideoRecv", 2);
    store(L, lav->av, "OnVideoRecvData", lav);
    // TODO: get userdata
    void *userdata = NULL;
    toxav_register_video_recv_callback(lav->av, callback_OnVideoRecv, userdata);
    lua_settop(L,0);
    return 0;
}

/*************************
 *                       *
 * ToxAv wrapped methods *
 *                       *
 *************************/

HandleCall *newCall(LToxAv *lav, int32_t call_index, ToxAvCSettings *csettings) {
    // TODO: if we decide to manage a list of calls, that's where we'll add it
    HandleCall *callHandle = (HandleCall*)malloc(sizeof(HandleCall));
    callHandle->lav = lav;
    callHandle->call_index = call_index;
    callHandle->from_string = 0;
    callHandle->frame = NULL;
    callHandle->dec_frame = NULL;
    callHandle->csettings = csettings;
    return callHandle;
}

// TODO: simplify csettings handling -- just get the call_type ?
//       we don't want to pass this any time we call something, that's not much a payload,
//       but payload anyway
//

ToxAvCSettings *parseSettings(lua_State *L, int index) {
    if (! lua_istable(L,index) )
        luaL_argerror(L, index-1, "Expected LToxAvCSettings.");

    ToxAvCSettings *settings = (ToxAvCSettings*)malloc(sizeof(ToxAvCSettings));

    int n = 0;
    lua_getfield(L, index, "callType");
    settings->call_type = (ToxAvCallType)luaL_checknumber(L,-1);
    ++n;
    
    lua_getfield(L, index, "videoBitrate");
    settings->video_bitrate = (uint32_t)luaL_checknumber(L,-1);
    ++n;
    lua_getfield(L, index, "videoWidth");
    settings->max_video_width = (uint16_t)luaL_checknumber(L,-1);
    ++n;
    lua_getfield(L, index, "videoHeight");
    settings->max_video_height = (uint16_t)luaL_checknumber(L,-1);
    ++n;

    lua_getfield(L, index, "audioBitrate");
    settings->audio_bitrate = (uint32_t)luaL_checknumber(L,-1);
    ++n;
    lua_getfield(L, index, "audioFrameDuration");
    settings->audio_frame_duration = (uint16_t)luaL_checknumber(L,-1);
    ++n;
    lua_getfield(L, index, "audioSampleRate");
    settings->audio_sample_rate = (uint32_t)luaL_checknumber(L,-1);
    ++n;
    lua_getfield(L, index, "audioChannels");
    settings->audio_channels = (uint32_t)luaL_checknumber(L,-1);
    ++n;

    lua_pop(L,n);
    return settings;
}

int lua_toxav_call(lua_State* L) {
    LToxAv *lav = checkLToxAv(L, 1);
    int user = luaL_checknumber(L,2);
    ToxAvCSettings *csettings = parseSettings(L, 3);

    int ringing_seconds = luaL_checknumber(L,4);
    lua_settop(L,0);
    int32_t call_index;
    int r = toxav_call(lav->av, &call_index, user, csettings, ringing_seconds);
    if( r!= 0)
        return throw_error(L, r);

    HandleCall *callHandle = newCall(lav, call_index, csettings);

    lua_pushlightuserdata(L, callHandle);
    lua_pushnumber(L, call_index);
    return 2;
}

int lua_toxav_hangup(lua_State* L) {
    LToxAv *lav = checkLToxAv(L,1);
    int32_t call_index = luaL_checknumber(L,2);
    lua_settop(L,0);
    int r = toxav_hangup(lav->av, call_index);
    if(r!=0)
        return throw_error(L, r);
    lua_pushboolean(L, 1);
    return 1;
}

// for receiver, HandleCall is created here
int lua_toxav_answer(lua_State* L) {
    LToxAv *lav = checkLToxAv(L,1);
    int32_t call_index = luaL_checknumber(L,2);
    ToxAvCSettings *csettings = parseSettings(L, 3);
    lua_settop(L,0);

    int r = toxav_answer(lav->av, call_index, csettings);
    if(r != 0)
        return throw_error(L, r);

    HandleCall *callHandle = newCall(lav, call_index, csettings);
    lua_pushlightuserdata(L, callHandle);
    lua_pushnumber(L, call_index);
    return 2;
}

int lua_toxav_reject(lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    int32_t call_index = luaL_checknumber(L,2);
    const char* reason = lua_tostring(L,3);
    lua_settop(L,0);

    int r = toxav_reject(av, call_index, reason);
    if(r!=0)
        return throw_error(L, r);

    lua_pushboolean(L, 1);
    return 1;
}

int lua_toxav_cancel(lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    int peer_id = luaL_checknumber(L,3);
    const char* reason = lua_tostring(L,4);
    lua_settop(L,0);
    
    int r = toxav_cancel(av, handle->call_index, peer_id, reason);
    if(r!=0)
        return throw_error(L, r);

    lua_pushboolean(L, 1);
    return 1;
}

int lua_toxav_stop_call(lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    int32_t call_index = luaL_checknumber(L,2);
    lua_settop(L,0);
    int r = toxav_stop_call(av, call_index);
    if (r != 0)
        return throw_error(L, r);

    lua_pushboolean(L, 1);
    return 1;
}

int lua_toxav_prepare_transmission(lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    //ToxAvCSettings *settings = lua_get_settings(L,1);
    HandleCall* handle       = (HandleCall*)lua_touserdata(L,2);
    uint32_t jbuf_size       = luaL_checknumber(L, 3);
    uint32_t VAD_threshold   = luaL_checknumber(L,4);
    int support_video        = lua_toboolean(L,5);
    lua_settop(L,0);

    int r = toxav_prepare_transmission(av, handle->call_index, jbuf_size, VAD_threshold, support_video);

    int frame_size = (handle->csettings->audio_sample_rate * handle->csettings->audio_frame_duration / 1000);
    handle->frame_size = frame_size;
    if(!handle->dec_frame)
        handle->dec_frame = (int16_t*)malloc(frame_size * sizeof(int16_t));
    
    if(r==0) {
        lua_pushboolean(L, 1);
        return 1;
    }
    else {
        lua_pushnil(L);
        lua_pushnumber(L, r);
        return 2;
    }
}

// clear all pending calls, free mem if required, etc.
// TODO: should be merged with hangup or vice-versa
int lua_toxav_kill_transmission(lua_State* L) {
    LToxAv *lav = checkLToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    lua_settop(L,0);
    int32_t call_index = handle->call_index;
    int r = toxav_kill_transmission(lav->av, handle->call_index);

    // free handle
    if(handle->from_string && handle->frame)
        free(handle->frame);
    
    if(handle->dec_frame)
        free(handle->dec_frame);
    
    if(handle->csettings)
        free(handle->csettings);
    
    free(handle);

    if( r!= 0)
        return throw_error(L, r);
    lua_pushboolean(L, 1);
    return 1;
}

// NOTE: API changes, A/V are now callbacks
/*
int lua_toxav_recv_video (lua_State* L) {
    lua_pushstring(L, "recvVideo: no implemented");
    lua_error(L);

    ToxAv *av = checkToxAv(L,1);
    int32_t call_index = luaL_checknumber(L,2);
    lua_settop(L,0);
    // TODO
    vpx_image_t *output;
    int r = toxav_recv_video(av, call_index, &output);
    // return table of pointer ?
    // what's image_t made for ?
    if(r==0) {
        lua_newtable(L);
        lua_pushnumber(L, output->w);
        lua_setfield(L,-2, "w");
        // ...
        return 1;
    }
    else {
        lua_pushnil(L);
        lua_pushnumber(L, r);
        return 2;
    }
}

// return PCM_frame, dec_frame_length
int lua_toxav_recv_audio(lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    lua_settop(L,0);
    int r = toxav_recv_audio(av, handle->call_index, handle->frame_size, handle->dec_frame);
    if(r<0)
        lua_pushnil(L);
    else
        lua_pushlightuserdata(L, handle->dec_frame);
    lua_pushnumber(L, r);
    return 2;
}

int lua_toxav_send_video (lua_State* L) {
    lua_pushstring(L, "sendVideo: no implemented");
    lua_error(L);

    ToxAv *av = checkToxAv(L,1);
    lua_settop(L,0);
    return 0;
}
*/

// TODO: should merge prepare_audio and send_audio
int lua_toxav_send_audio (lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    unsigned int payload_size = (unsigned int)luaL_checknumber(L,3);
    lua_settop(L,0);
    int r = toxav_send_audio(av, handle->call_index, handle->payload, payload_size);
    if (r != 0)
        return throw_error(L, r);
    lua_pushboolean(L, 1);
    return 1;
}

int lua_toxav_prepare_video_frame(lua_State* L) {
    lua_pushstring(L, "prepareVideoFrame: no implemented");
    lua_error(L);

    ToxAv *av = checkToxAv(L,1);
    lua_settop(L,0);
    return 0;
}


// TODO: should we return the payload as well ?
// when would that be needed ?
int lua_toxav_prepare_audio_frame(lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    int payload_size = luaL_checknumber(L,3);
    //int16_t *frame = (int16_t*)lua_touserdata(L,4);
    if(lua_type(L,4) == LUA_TUSERDATA) {
        handle->frame = (int16_t*)lua_touserdata(L,4);
    }
    else {
        // FIXME: do we need this ? need to get sure it's int16_t
        handle->from_string = 1;
        if(! handle->frame)
            handle->frame = (int16_t*)malloc(handle->frame_size * sizeof(int16_t));
        size_t len;
        const void* f = lua_tolstring(L,4, &len);
        memcpy(handle->frame, f, (len<handle->frame_size) ? len : handle->frame_size);
    }
    lua_settop(L,0);
    int r = toxav_prepare_audio_frame(av, handle->call_index,
                                            handle->payload, payload_size, handle->frame, handle->frame_size);

    if(r<=0)
        return throw_error(L, r);
    lua_pushnumber(L, r);
    return 1;
}

int lua_toxav_get_peer_csettings (lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    int peer = luaL_checknumber(L,3);
    lua_settop(L,0);
    int r = toxav_get_peer_csettings(av, handle->call_index, peer, handle->csettings);
    if(r<0)
        return throw_error(L, r);
    lua_pushnumber(L, r);
    return 1;
}

int lua_toxav_get_peer_id (lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    int peer = luaL_checknumber(L,3);
    lua_settop(L,0);
    int r = toxav_get_peer_id(av, handle->call_index, peer);
    if(r<0)
        return throw_error(L, r);

    lua_pushnumber(L, r);
    return 1;
}

int lua_toxav_capability_supported (lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    ToxAvCapabilities cap = (ToxAvCapabilities)luaL_checknumber(L,3);
    lua_settop(L,0);
    int r = toxav_capability_supported(av, handle->call_index, cap);
    lua_pushboolean(L,r);
    return 1;
}

/*
int lua_toxav_set_audio_queue_limit (lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    int64_t limit = luaL_checknumber(L,3);
    lua_settop(L,0);
    toxav_set_audio_queue_limit(av, handle->call_index, limit);
    return 0;
}

int lua_toxav_set_video_queue_limit (lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    int64_t limit = luaL_checknumber(L,3);
    lua_settop(L,0);
    toxav_set_video_queue_limit(av, handle->call_index, limit);
    return 0;
}
*/

int lua_toxav_get_call_state(lua_State *L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    lua_settop(L,0);
    ToxAvCallState state = toxav_get_call_state(av, handle->call_index);

    lua_pushnumber(L,state);
    return 1;
}

int lua_toxav_change_settings(lua_State *L) {
    ToxAv *av = checkToxAv(L,1);
    HandleCall* handle = (HandleCall*)lua_touserdata(L,2);
    ToxAvCSettings *csettings = parseSettings(L,3);
    lua_settop(L,0);

    ToxAvCSettings *oldSettings = handle->csettings;
    handle->csettings = csettings;
    int state = toxav_change_settings(av, handle->call_index, handle->csettings);

    if(state != 0)
        return throw_error(L, state);

    if(oldSettings)
        free(oldSettings);

    lua_pushboolean(L, 1);
    return 1;
}

int lua_toxav_get_tox(lua_State* L) {
    ToxAv *av = checkToxAv(L,1);
    lua_settop(L,0);
    return 0;
}

/************************************
 *                                  *
 * lua module loader and destructor *
 *                                  *
 ************************************/

int lua_toxav_gc(lua_State* L) {
    LToxAv *lav = checkLToxAv(L,1);
    lua_settop(L,0);

    // TODO: kill all running calls first, if any
    //       maintain list in registry, or something
    
    // free remaining calls and malloc's
    if(lav->calls && lav->nb_calls) {
        for(int i=0;i<lav->nb_calls;++i) {
            // TODO: could kill + hangup too ?
            // should do...
            Call call = lav->calls[i];
            if(call.handle) {
                if(call.handle->from_string && call.handle->frame)
                    free(call.handle->frame);
                if(call.handle->dec_frame)
                    free(call.handle->dec_frame);
                free(call.handle);
            }
        }
    }
    free(lav->calls);

    // clear registry
    unreg(L, lav);

    if(lav->av!=NULL)
        toxav_kill( lav->av );

    return 0;
}

static int lua_toxav_tostring(lua_State* L) {
    char buf[32];
    sprintf(buf, "%p", checkToxAv(L,1));
    lua_pushfstring(L, "ToxAv (%s)", buf);
    return 1;
}

int lua_toxav_new(lua_State* L) {
    LTox *ltox = checkLTox(L,1);
    int32_t max_calls = luaL_checknumber(L,2);
    lua_settop(L,0);
    Tox *tox = ltox->tox;
    LToxAv *lav = pushToxAv(L, tox, max_calls);
    
    lav->max_calls = max_calls;
    lav->calls = (Call*)calloc(max_calls, sizeof(Call));
    lav->nb_calls = 0;

    lav->callbacks.on_invite = 0;
    lav->callbacks.on_start = 0;
    lav->callbacks.on_cancel = 0;
    lav->callbacks.on_reject = 0;
    lav->callbacks.on_end = 0;
    lav->callbacks.on_ringing = 0;
    lav->callbacks.on_starting = 0;
    lav->callbacks.on_ending = 0;
    //lav->callbacks.on_error = 0;
    lav->callbacks.on_request_timeout = 0;
    lav->callbacks.on_peer_timeout = 0;
    lav->callbacks.on_audio_recv = 0;
    lav->callbacks.on_video_recv = 0;
    lav->callbacks.on_media_change = 0;

    reg(L, lav);
    return 1;
}
int lua_toxav_new_self(lua_State* L) {
    lua_remove(L,1);
    return lua_toxav_new(L);
}
// TODO: improve: set methods only when new is called
static const luaL_Reg toxav_methods[] = {
    {"new", lua_toxav_new},

    {"setSettings", setSettings},
    {"getSettings", getSettings},
    {"setVideoBitrate", setVideoBitrate},
    {"getVideoBitrate", getVideoBitrate},
    {"setVideoWidth", setVideoWidth},
    {"getVideoWidth", getVideoWidth},
    {"setVideoHeight", setVideoHeight},
    {"getVideoHeight", getVideoHeight},
    {"setAudioBitrate", setAudioBitrate},
    {"getAudioBitrate", getAudioBitrate},
    {"setAudioFrame_duration", setAudioFrame_duration},
    {"getAudioFrame_duration", getAudioFrame_duration},
    {"setAudioSample_rate", setAudioSample_rate},
    {"getAudioSample_rate", getAudioSample_rate},
    {"setAudioChannels", setAudioChannels},
    {"getAudioChannels", getAudioChannels},
    //{"setJbufCapacity", setJbufCapacity},
    //{"getJbufCapacity", getJbufCapacity},

    {"call", lua_toxav_call},
    {"hangup", lua_toxav_hangup},
    {"answer", lua_toxav_answer},
    {"reject", lua_toxav_reject},
    {"cancel", lua_toxav_cancel},
    {"stopCall", lua_toxav_stop_call},
    {"prepareTransmission", lua_toxav_prepare_transmission},
    {"killTransmission", lua_toxav_kill_transmission},
    //{"recvVideo", lua_toxav_recv_video},
    //{"recvAudio", lua_toxav_recv_audio},
    {"sendVideo", lua_toxav_send_video},
    {"sendAudio", lua_toxav_send_audio},
    {"prepareVideoFrame", lua_toxav_prepare_video_frame},
    {"prepareAudioFrame", lua_toxav_prepare_audio_frame},
    {"getPeerTransmissionType", lua_toxav_get_peer_csettings},
    {"getPeerId", lua_toxav_get_peer_id},
    {"capabilitySupported", lua_toxav_capability_supported},
    //{"setAudioQueueLimit", lua_toxav_set_audio_queue_limit},
    //{"setVideoQueueLimit", lua_toxav_set_video_queue_limit},

    {"getCallState", lua_toxav_get_call_state},

    {"registerCallback", lua_toxav_register_callstate_callback},
    {"recvVideo", lua_toxav_audio_recv_callback},
    {"recvAudio", lua_toxav_video_recv_callback},
    
    {NULL, NULL}
};

static const luaL_Reg toxav_meta[] = {
    {"__gc", lua_toxav_gc},
    {"__tostring", lua_toxav_tostring},
    {NULL,NULL}
};

#define TOXAV_CLASS "ToxAvClass"
static const luaL_Reg class_meta[] = {
    {"__call", lua_toxav_new_self},
    {NULL,NULL}
};

int lua_toxav_register(lua_State* L) {
    Ls = L;
    lua_newtable(L);
    for(int f = 0; toxav_methods[f].name != NULL; ++f) {
        lua_pushstring(L, toxav_methods[f].name);
        lua_pushcclosure(L, toxav_methods[f].func, 0);
        lua_settable(L, -3);
    }

    // callbacks
    lua_pushstring(L, "cb");
    lua_newtable(L);
    lua_pushnumber(L, av_OnInvite);
    lua_setfield(L, -2, "OnInvite");
    lua_pushnumber(L, av_OnStart);
    lua_setfield(L, -2, "OnStart");
    lua_pushnumber(L, av_OnCancel);
    lua_setfield(L, -2, "OnCancel");
    lua_pushnumber(L, av_OnReject);
    lua_setfield(L, -2, "OnReject");
    lua_pushnumber(L, av_OnEnd);
    lua_setfield(L, -2, "OnEnd");
    lua_pushnumber(L, av_OnRinging);
    lua_setfield(L, -2, "OnRinging");
    lua_pushnumber(L, av_OnStarting);
    lua_setfield(L, -2, "OnStarting");
    lua_pushnumber(L, av_OnEnding);
    lua_setfield(L, -2, "OnEnding");
    //lua_pushnumber(L, av_OnError);
    //lua_setfield(L, -2, "OnError");
    lua_pushnumber(L, av_OnRequestTimeout);
    lua_setfield(L, -2, "OnRequestTimeout");
    lua_pushnumber(L, av_OnPeerTimeout);
    lua_setfield(L, -2, "OnPeerTimeout");
    lua_pushnumber(L, av_OnMediaChange);
    lua_setfield(L, -2, "OnMediaChange");
    lua_settable(L, -3);

    // errors
    lua_pushstring(L, "errors");
    lua_newtable(L);
    lua_pushstring(L, "None");
    lua_pushnumber(L, ErrorNone);
    lua_settable(L,-3);
    lua_pushstring(L, "Internal");
    lua_pushnumber(L, ErrorInternal);
    lua_settable(L,-3);
    lua_pushstring(L, "AlreadyInCall");
    lua_pushnumber(L, ErrorAlreadyInCall);
    lua_settable(L,-3);
    lua_pushstring(L, "NoCall");
    lua_pushnumber(L, ErrorNoCall);
    lua_settable(L,-3);
    lua_pushstring(L, "InvalidState");
    lua_pushnumber(L, ErrorInvalidState);
    lua_settable(L,-3);
    lua_pushstring(L, "NoRtpSession");
    lua_pushnumber(L, ErrorNoRtpSession);
    lua_settable(L,-3);
    lua_pushstring(L, "AudioPacketLost");
    lua_pushnumber(L, ErrorAudioPacketLost);
    lua_settable(L,-3);
    lua_pushstring(L, "StartingAudioRtp");
    lua_pushnumber(L, ErrorStartingAudioRtp);
    lua_settable(L,-3);
    lua_pushstring(L, "StartingVideoRtp");
    lua_pushnumber(L, ErrorStartingVideoRtp);
    lua_settable(L,-3);
    lua_pushstring(L, "TerminatingAudioRtp");
    lua_pushnumber(L, ErrorTerminatingAudioRtp);
    lua_settable(L,-3);
    lua_pushstring(L, "TerminatingVideoRtp");
    lua_pushnumber(L, ErrorTerminatingVideoRtp);
    lua_settable(L,-3);
    lua_pushstring(L, "PacketTooLarge");
    lua_pushnumber(L, ErrorPacketTooLarge);
    lua_settable(L,-3);
    lua_settable(L,-3);

    // call type
    lua_pushstring(L, "calltype");
    lua_newtable(L);
    lua_pushstring(L, "audio");
    lua_pushnumber(L, TypeAudio);
    lua_settable(L,-3);
    lua_pushstring(L, "video");
    lua_pushnumber(L, TypeVideo);
    lua_settable(L,-3);
    lua_settable(L,-3);

    // call state
    lua_pushstring(L, "state");
    lua_newtable(L);
    lua_pushstring(L, "inviting");
    lua_pushnumber(L, av_CallInviting);
    lua_pushstring(L, "starting");
    lua_pushnumber(L, av_CallStarting);
    lua_pushstring(L, "active");
    lua_pushnumber(L, av_CallActive);
    lua_pushstring(L, "hold");
    lua_pushnumber(L, av_CallHold);
    lua_pushstring(L, "hangedUp");
    lua_pushnumber(L, av_CallHanged_up);
    lua_settable(L,-3);
    lua_settable(L,-3);


    // capabilities
    lua_pushstring(L, "capabilities");
    lua_newtable(L);
    lua_pushstring(L, "AudioEncoding");
    lua_pushnumber(L, AudioEncoding);
    lua_settable(L, -3);
    lua_pushstring(L, "AudioDecoding");
    lua_pushnumber(L, AudioDecoding);
    lua_settable(L, -3);
    lua_pushstring(L, "VideoEncoding");
    lua_pushnumber(L, VideoEncoding);
    lua_settable(L, -3);
    lua_pushstring(L, "VideoDecoding");
    lua_pushnumber(L, VideoDecoding);
    lua_settable(L, -3);
    lua_settable(L, -3);

    luaL_newmetatable(L, TOXAV_STR);
    for(int f = 0; toxav_meta[f].name != NULL; ++f) {
        lua_pushstring(L, toxav_meta[f].name);
        lua_pushcclosure(L, toxav_meta[f].func, 0);
        lua_settable(L, -3);
    }

    lua_pushliteral(L, "__index");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    lua_pushliteral(L, "__metatable");
    lua_pushvalue(L, -3);
    lua_rawset(L, -3);

    lua_pop(L,1); // drop metatable

    luaL_newmetatable(L, TOXAV_CLASS);
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
   
    lua_setmetatable(L,-2);

    return 1;
}

int luaopen_toxav(lua_State* L) {
    return lua_toxav_register(L);
}

#ifdef __cplusplus
}
#endif
