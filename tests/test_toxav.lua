--[[
 
  Lua binding for ToxAC.
 
   Copyright (C) 2014 Peersuasive Technologies All Rights Reserved.
 
   This file is part of LuaTox.
 
   LuaTox is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
 
   LuaTox is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
 
   You should have received a copy of the GNU General Public License
   along with LuaTox.  If not, see <http://www.gnu.org/licenses/>.
 
--]]


local Tox = require"tox"
local ToxAv = require"toxav"

-- init
local function c_sleep(n)
    os.execute("sleep "..tostring(n/1000))
end

local bootstrap_node = Tox{ipv6enabled=false};
local Alice = Tox{ipv6enabled=false}
local Bob = Tox{ipv6enabled=false}
print("Alice:", Alice)
print("Bob:", Bob)

local to_compare = "abcdefgh"

assert(bootstrap_node and Alice and Bob, "Failed to create 3 tox instances");

local function test_cb()
    local tx = Tox{ipv6enabled=false}
    local av = ToxAv(tx, 1),
    print("registering callbacks")
    av:registerCallback( function()end, ToxAv.cb.OnStart )
    av:registerCallback( function()end, ToxAv.cb.OnCancel )
    av:registerCallback( function()end, ToxAv.cb.OnReject )
    av:registerCallback( function()end, ToxAv.cb.OnEnd )
    av:registerCallback( function()end, ToxAv.cb.OnInvite )
    av:registerCallback( function()end, ToxAv.cb.OnRinging )
    av:registerCallback( function()end, ToxAv.cb.OnStarting )
    av:registerCallback( function()end, ToxAv.cb.OnEnding )
    av:registerCallback( function()end, ToxAv.cb.OnRequestTimeout )
    av:registerCallback( function()end, ToxAv.cb.OnMediaChange )
    av:registerRecvAudio( function()end )
    av:registerRecvVideo( function()end )
    av:kill()
    tx:kill()
    tx = nil
end

local function test_init()
    local function accept_friend_request(pub, msg, userdata)
        print("to compare:", userdata)
        local pat = "gentoo"
        if (#msg == #pat) and ("gentoo"==msg) then
            local r = Alice:addFriendNorequest(pub)
            if(r<0)then print("FAILED: addFriendNorequest" ) error("FAILED") end
        end
    end
    Alice:callbackFriendRequest(accept_friend_request, to_compare)

    local address = Alice:getAddress()
    local test, err = Bob:addFriend(address, "gentoo")
    assert( (test == 0), string.format("Failed to add friend error code: %s", (err or "[no msg]")) )

    local off = 1

    print("Connecting people...")
    while (1) do
        bootstrap_node:toxDo()
        Alice:toxDo()
        Bob:toxDo()

        if (bootstrap_node:isConnected() and Alice:isConnected() and Bob:isConnected() and off) then
            print("Toxes are online")
            off = nil
        end

        if (Alice:isOnline(0)) and (Bob:isOnline(0)) then
            print("Alice and Bob are friends")
            break
        end
        c_sleep(20);
    end
 
    -- states
    local status_control = {
        Alice = { 
            status = "none", 
            av = ToxAv(Alice, 1),
            CallStarted = nil,
            call_index = -1,
            call_ids = {},
        },
        Bob = { 
            status = "none", 
            av = ToxAv(Bob, 1),
            CallStarted = nil,
            call_index = -1,
            call_ids = {},
        },
    }

    local function resetStates()
        -- reset states
        status_control.Alice.status = "none"
        status_control.Alice.CallStarted = nil
        status_control.Alice.call_index = -1
        status_control.Alice.call_ids = {}

        status_control.Bob.status = "none"
        status_control.Bob.CallStarted = nil
        status_control.Bob.call_index = -1
        status_control.Bob.call_ids = {}
    end

    local CallStatus = {
        none = "none",
        InCall = "InCall",
        Ringing = "Ringing",
        Ended = "Ended",
        Rejected = "Rejected",
        Cancel = "Cancel",
        TimedOut = "TimedOut",
    }

    -- set some variables
    local AliceAV, BobAV = status_control.Alice.av, status_control.Bob.av

    print("getting default settings")
    local settings = AliceAV:getSettings()
    print("got settings")
    settings.maxVideoHeight = 128
    settings.maxVideoWidth = 128

    local payload_size, frame_size = 0, (settings.audioSampleRate * settings.audioFrameDuration / 1000);
    local sample_payload = string.rep(string.byte('1'), frame_size)
 
    --
    -- Callbacks
    --

    local function callback_recv_invite(call_index, userdata)
        print(" Bob receives an invitation to a call...", userdata)
        status_control.Bob.status = CallStatus.Ringing
        status_control.Bob.call_index = call_index
    end

    local function callback_recv_ringing(call_index, userdata)
        -- Alice always sends invite
        status_control.Alice.status = CallStatus.Ringing
    end

    local function callback_recv_starting(call_index, userdata)
        status_control.Alice.status = CallStatus.InCall
        local r, e = AliceAV:prepareTransmission(status_control.Alice.call_index,
                            settings.jbufCapacity, settings.VADTolerance, false)
        if not(r) then
            print(string.format("ERROR: failed to prepare Alice's transmission: %s", tostring(e)))
            error("ERROR")
        end
    end

    local function callback_recv_ending(call_index, userdata)
        if (status_control.Alice.status == CallStatus.Rejected) then
            print(" Call ended for Bob!", userdata)
            status_control.Bob.status = CallStatus.Ended
        else
            print(" Call ended for Alice!", userdata)
            status_control.Alice.status = CallStatus.Ended
        end
    end

    local function callback_call_started(call_index, userdata)
        status_control.Bob.status = CallStatus.InCall
        local r, e = BobAV:prepareTransmission(status_control.Bob.call_index, 
                            settings.jbufCapacity, settings.VADTolerance, true)
        if not(r) then
            print(string.format("ERROR: failed to prepare Bob's transmission: %s", tostring(e)))
            error("ERROR")
        end
    end

    local function callback_call_canceled(call_index, userdata)
        print(" Call Canceled for Bob!", userdata)
        status_control.Bob.status = CallStatus.Cancel
    end

    local function callback_call_rejected(call_index, userdata)
        print(string.format(" Call rejected by Bob!\n Call ended for Alice! (%s)", userdata))
        -- If Bob rejects, call is ended for alice and she sends ending
        status_control.Alice.status = CallStatus.Rejected
    end

    local function callback_call_ended(call_index, userdata)
        print(" Call ended for Bob!", userdata)
        status_control.Bob.status = CallStatus.Ended
    end

    local function callback_requ_timeout(call_index, userdata)
        print("callback: call timed-out!", userdata);
        status_control.Alice.status = CallStatus.TimedOut
    end

    local function callback_call_type_change_common(csettings, userdata)
        print(string.format("New settings: \n"
            .."Type: %u \n"
            .."Video bitrate: %u \n"
            .."Video height: %u \n"
            .."Video width: %u \n"
            .."Audio bitrate: %u \n"
            .."Audio framedur: %u \n"
            .."Audio sample rate: %u \n"
            .."Audio channels: %u \n"
            .."Jbuf Capacity: %u \n"
            .."VAD Tolerance: %u \n",
            csettings.callType,
            csettings.videoBitrate,
            csettings.maxVideoHeight,
            csettings.maxVideoWidth,
            csettings.audioBitrate,
            csettings.audioFrameDuration,
            csettings.audioSampleRate,
            csettings.audioChannels,
            csettings.jbufCapacity,
            csettings.VADTolerance
            )
        )
    end
    local function callback_call_type_change_Bob(call_index, userdata)
        local csettings = BobAv:getPeerCSettings(status_control.Bob.call_index, 0)
        callback_call_type_change_common(csettings)
    end
    local function callback_call_type_change_Alice(call_index, userdata)
        local csettings = AliceAv:getPeerCSettings(status_control.Alice.call_index, 0)
        callback_call_type_change_common(csettings)
    end
 
    local function callback_audio(call_index, userdata)
        --print("audio callback")
    end
    
    local function callback_video(call_index, userdata)
        --print("video callback")
    end

    print("registering Bob' callbacks")
    -- register Bob's callbacks
    BobAV:registerCallback( callback_call_started,  ToxAv.cb.OnStart, to_compare );
    BobAV:registerCallback( callback_call_canceled, ToxAv.cb.OnCancel, to_compare );
    BobAV:registerCallback( callback_call_rejected, ToxAv.cb.OnReject, to_compare );
    BobAV:registerCallback( callback_call_ended,    ToxAv.cb.OnEnd, to_compare );
    BobAV:registerCallback( callback_recv_invite,   ToxAv.cb.OnInvite, to_compare );

    BobAV:registerCallback( callback_recv_ringing,  ToxAv.cb.OnRinging, to_compare );
    BobAV:registerCallback( callback_recv_starting, ToxAv.cb.OnStarting, to_compare );
    BobAV:registerCallback( callback_recv_ending,   ToxAv.cb.OnEnding, to_compare );

    BobAV:registerCallback( callback_requ_timeout,  ToxAv.cb.OnRequestTimeout, to_compare );
    BobAV:registerCallback( callback_call_type_change_Bob, ToxAv.cb.OnMediaChange, to_compare );

    BobAV:registerRecvAudio( callback_audio, to_compare );
    BobAV:registerRecvVideo( callback_video, to_compare );

    print("registering Alice' callbacks")
    -- register Alice's callbacks
    AliceAV:registerCallback( callback_call_started,  ToxAv.cb.OnStart, to_compare );
    AliceAV:registerCallback( callback_call_canceled, ToxAv.cb.OnCancel, to_compare );
    AliceAV:registerCallback( callback_call_rejected, ToxAv.cb.OnReject, to_compare );
    AliceAV:registerCallback( callback_call_ended,    ToxAv.cb.OnEnd,    to_compare );
    AliceAV:registerCallback( callback_recv_invite,   ToxAv.cb.OnInvite, to_compare );

    AliceAV:registerCallback( callback_recv_ringing,  ToxAv.cb.OnRinging, to_compare );
    AliceAV:registerCallback( callback_recv_starting, ToxAv.cb.OnStarting, to_compare );
    AliceAV:registerCallback( callback_recv_ending,   ToxAv.cb.OnEnding, to_compare );

    AliceAV:registerCallback( callback_requ_timeout,  ToxAv.cb.OnRequestTimeout, to_compare );
    AliceAV:registerCallback( callback_call_type_change_Alice, ToxAv.cb.OnMediaChange, to_compare );

    AliceAV:registerRecvAudio( callback_audio, to_compare );
    AliceAV:registerRecvVideo( callback_video, to_compare );
    
    --
    -- Call with audio only on both sides. Alice calls Bob.
    --

    print("********** Audio call *********")

    local step, running = 0, true
    local cur_time = os.time()
    while (running) do
        bootstrap_node:toxDo()
        Alice:toxDo()
        Bob:toxDo()

        if(0==step)then -- Alice
            print(" Alice is calling...");
            local id, call_index = AliceAV:call(0, settings, 10);
            if not(id)then
                error(string.format("Couldn't get call id: %s", (call_index or "[no msg]")))
            end
            status_control.Alice.call_index = id
            status_control.Alice.call_ids[call_index] = { handle = id, peer = 0 }

            step = step + 1

        elseif(1==step)then -- Bob
            if (status_control.Bob.status == CallStatus.Ringing) then
                print(" Bob answers...")
                local id, err = BobAV:answer(status_control.Bob.call_index, settings)
                if not(id)then
                    error(string.format("Error at Bob answer: %s", (err or "[no msg]")))
                else
                    print(" Bob answered")
                end
                status_control.Bob.call_index = id

                step = step + 1
            end 

        elseif(2==step)then -- RTP transmission
            if (status_control.Bob.status == CallStatus.InCall)
                and (status_control.Alice.status == CallStatus.InCall) then
                -- Both send
                local err
                payload_size, err = AliceAV:prepareAudioFrame(status_control.Alice.call_index, 
                                1000, sample_payload)
                assert(payload_size, string.format("Failed to encode Alice's payload: %s", (err or "[no msg]")))

                local r, e = AliceAV:sendAudio(status_control.Alice.call_index, payload_size)
                assert(r, "Failed to send Alice's audio: %s", tostring(e))

                payload_size = BobAV:prepareAudioFrame(status_control.Bob.call_index, 
                                1000, sample_payload)
                assert(payload_size, string.format("Failed to encode Bob's payload: %s", (err or "[no msg]")))

                local r, e = BobAV:sendAudio(status_control.Bob.call_index, payload_size)
                assert(r, "Failed to send Bob's audio: %s", tostring(e))

                if ( (os.time() - cur_time) > 5) then -- Transmit for 10 seconds
                    print(" killing Alice call...")
                    local call_id, e = AliceAV:killTransmission(status_control.Alice.call_index)
                    print(" killing Bob call...")
                    local r, e = BobAV:killTransmission(status_control.Bob.call_index)

                    -- Call over Alice hangs up
                    print(" Alice's hanging up...")
                    AliceAV:hangup(call_id)
                    step = step+1 -- This terminates the loop
                end
            end

        elseif(3==step)then -- Wait for Both to have status ended
            if (status_control.Alice.status == CallStatus.Ended) 
                            and (status_control.Bob.status == CallStatus.Ended) then
                running = false
            end
        end
        c_sleep(20); 
    end

    --
    -- Call with audio on both sides and video on one side. Alice calls Bob.
    -- TODO: implement video
    --

    print("********** Audio and video call (only audio atm) *********")
    -- reset states
    resetStates()

    local step, running = 0, true
    -- NOTE: keep it even if video's not implemented,
    -- it's also testing if environment is well cleared
    local cur_time = os.time()
    while (running) do
        bootstrap_node:toxDo()
        Alice:toxDo()
        Bob:toxDo()

        if(0==step)then -- Alice
            print(" Alice is calling...");
            local id, call_index, err = AliceAV:call(0, settings, 10);
            if not(id)then
                error(string.format("Couldn't get call id: %s", (err or "[no msg]")))
            end
            status_control.Alice.call_index = id

            step = step + 1

        elseif(1==step)then -- Bob
            if (status_control.Bob.status == CallStatus.Ringing) then
                print( "Bob answers...")
                local id, call_index, err = BobAV:answer(status_control.Bob.call_index, settings)
                if not(id)then
                    error(string.format("Error at Bob answer: %s", (err or "[no msg]")))
                else
                    print(" Bob answered")
                end
                status_control.Bob.call_index = id

                step = step + 1
            end 

        elseif(2==step)then -- RTP transmission
            if (status_control.Bob.status == CallStatus.InCall)
                and (status_control.Alice.status == CallStatus.InCall) then
                -- Both send
                payload_size = AliceAV:prepareAudioFrame(status_control.Alice.call_index,
                               1000, sample_payload)
                assert(payload_size, string.format("Failed to encode Alice's payload: %s", (err or "[no msg]")))

                AliceAV:sendAudio(status_control.Alice.call_index, payload_size)

                payload_size = BobAV:prepareAudioFrame(status_control.Bob.call_index, 
                                1000, sample_payload)
                assert(payload_size, string.format("Failed to encode Bob's payload: %s", (err or "[no msg]")))

                BobAV:sendAudio(status_control.Bob.call_index, payload_size)

                if ( (os.time() - cur_time) > 5) then -- Transmit for 10 seconds
                    print(" killing Alice call...")
                    local call_id, e = AliceAV:killTransmission(status_control.Alice.call_index)
                    print(" killing Bob call...")
                    local r, e = BobAV:killTransmission(status_control.Bob.call_index)

                    -- Call over Alice hangs up
                    print(" Alice's hanging up...")
                    AliceAV:hangup(call_id)
                    step = step+1 -- This terminates the loop
                end
            end

        elseif(3==step)then -- Wait for Both to have status ended
            if (status_control.Alice.status == CallStatus.Ended) 
                            and (status_control.Bob.status == CallStatus.Ended) then
                running = false
            end
        end
        c_sleep(20); 
    end

    print("other flows")
    ---
    -- Other flows
    ---

    --
    -- Call and reject
    --
    print("****** Rejecting a call *******")
    resetStates()

    do
        local step = 0
        local running = true
        while (running) do
            bootstrap_node:toxDo()
            Alice:toxDo()
            Bob:toxDo()

            if(0==step) then -- Alice
                print(" Alice is calling...");
                local id, call_index, err = AliceAV:call(0, settings, 10);
                if not(id)then
                    error(string.format("Couldn't get call id: %s", (err or "[no msg]")))
                end
                status_control.Alice.call_index = id

                step = step + 1

            elseif(1==step)then -- Bob
                if (status_control.Bob.status == CallStatus.Ringing) then
                    print(" Bob rejects...")
                    BobAV:reject(status_control.Bob.call_index, "Who likes D's anyway?")
                    step = step + 1
                end

            elseif(2==step)then -- Wait for Both to have status ended 
                if (status_control.Alice.status == CallStatus.Rejected) 
                    and (status_control.Bob.status == CallStatus.Ended) then
                    running = false
                end
            end
            c_sleep(20)
        end
    end


    --
    -- Call and cancel
    --

    print("****** Canceling a call *******")
    resetStates()

    do
        local step = 0
        local running = true
        while (running) do
            bootstrap_node:toxDo()
            Alice:toxDo()
            Bob:toxDo()

            if(0==step) then -- Alice
                print(" Alice is calling...");
                local id, call_index, err = AliceAV:call(0, settings, 10);
                if not(id)then
                    error(string.format("Couldn't get call id: %s", (err or "[no msg]")))
                end
                status_control.Alice.call_index = id

                step = step + 1

            elseif(1==step)then -- Alice again
                if (status_control.Bob.status == CallStatus.Ringing) then
                    print(" Alice cancels...");
                    AliceAV:cancel(status_control.Alice.call_index, 0, "Who likes D's anyway?")
                    step = step + 1
                end

            elseif(2==step)then -- Wait for Both to have status ended 
                if (status_control.Bob.status == CallStatus.Cancel) then
                    running = false
                end
            end
            c_sleep(20)
        end
    end

    --
    -- Timeout
    --

    print("****** Timeout *******")
    resetStates()

    do
        local step = 0
        local running = true
        while (running) do
            bootstrap_node:toxDo()
            Alice:toxDo()
            Bob:toxDo()

            if(0==step) then
                print(" Alice is calling...");
                local id, call_index, err = AliceAV:call(0, settings, 5);
                if not(id)then
                    error(string.format("Couldn't get call id: %s", (err or "[no msg]")))
                end
                status_control.Alice.call_index = id

                step = step + 1

            elseif(1==step)then
                if (status_control.Alice.status == CallStatus.TimedOut)then
                    running = 0
                    print(" TimedOut...");
                    break
                end
            end
            c_sleep(20)
        end
    end

    print("Calls ended!")
end

-- TESTS

test_cb()
test_init()


print("END")
