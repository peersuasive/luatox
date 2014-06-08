local Tox = require"tox"
local ToxAv = require"toxav"

-- init
local function c_sleep(n)
    os.execute("sleep "..tostring(n/1000))
end

local bootstrap_node = Tox(0);
local Alice = Tox(0);
local Bob = Tox(0);
assert(bootstrap_node and Alice and Bob, "Failed to create 3 tox instances");

local function test_init()
    local function accept_friend_request(pub, data)
        local pat = "gentoo"
        if (#data == #pat) and ("gentoo"==data) then
            local r = Alice:addFriendNorequest(pub)
            if(r<0)then print("FAILED: addFriendNorequest" ) error("FAILED") end
        end
    end
    Alice:callbackFriendRequest(accept_friend_request)

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

        if (Alice:getFriendConnectionStatus(0) == 1) and (Bob:getFriendConnectionStatus(0) == 1) then
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
        },
        Bob = { 
            status = "none", 
            av = ToxAv(Bob, 1),
            CallStarted = nil,
            call_index = -1,
        },
    }

    local function resetStates()
        -- reset states
        status_control.Alice.status = "none"
        status_control.Alice.CallStarted = nil
        status_control.Alice.call_index = -1

        status_control.Bob.status = "none"
        status_control.Bob.CallStarted = nil
        status_control.Bob.call_index = -1
    end

    local CallStatus = {
        none = "none",
        InCall = "InCall",
        Ringing = "Ringing",
        Ended = "Ended",
        Rejected = "Rejected",
        Cancel = "Cancel",
    }

    -- set some variables
    local AliceAV, BobAV = status_control.Alice.av, status_control.Bob.av

    local settings = AliceAV:getSettings()
    local payload_size, frame_size = 0, (settings.audioSampleRate * settings.audioFrameDuration / 1000);
    local sample_payload = string.rep(string.byte('1'), frame_size)
 
    --
    -- Callbacks
    --

    local function callback_recv_invite(call_index)
        print(" Bob receives an invitation to a call...")
        status_control.Bob.status = CallStatus.Ringing
        status_control.Bob.call_index = call_index
    end

    local function callback_recv_ringing(call_index)
        -- Alice always sends invite
        status_control.Alice.status = CallStatus.Ringing
    end

    local function callback_recv_starting(call_index)
        status_control.Alice.status = CallStatus.InCall
        local r, e = AliceAV:prepareTransmission(status_control.Alice.call_index, false)
        if not(r) then
            print(string.format("ERROR: failed to prepare Alice's transmission: %s", tostring(e)))
            error("ERROR")
        end
    end

    local function callback_recv_ending(call_index)
        if (status_control.Alice.status == CallStatus.Rejected) then
            print(" Call ended for Bob!")
            status_control.Bob.status = CallStatus.Ended
        else
            print(" Call ended for Alice!")
            status_control.Alice.status = CallStatus.Ended
        end
    end

    local function callback_recv_error(call_index)
        print("!!!!! AV ERROR !!!!!")
    end

    local function callback_call_started(call_index)
        status_control.Bob.status = CallStatus.InCall
        local r, e = BobAV:prepareTransmission(status_control.Bob.call_index, true)
        if not(r) then
            print(string.format("ERROR: failed to prepare Bob's transmission: %s", tostring(e)))
            error("ERROR")
        end
    end

    local function callback_call_canceled(call_index)
        print(" Call Canceled for Bob!")
        status_control.Bob.status = CallStatus.Cancel
    end

    local function callback_call_rejected(call_index)
        print(string.format(" Call rejected by Bob!\n Call ended for Alice!"))
        -- If Bob rejects, call is ended for alice and she sends ending
        status_control.Alice.status = CallStatus.Rejected
    end

    local function callback_call_ended(call_index)
        print(" Call ended for Bob!")
        status_control.Bob.status = CallStatus.Ended
    end

    local function callback_request_timeout(call_index)
        print("!!!! No answer !!!!!")
    end

    -- register Bob's callbacks
    BobAV:registerCallback(ToxAv.cb.OnInvite, callback_recv_invite)
    BobAV:registerCallback(ToxAv.cb.OnStart, callback_call_started)
    BobAV:registerCallback(ToxAv.cb.OnCancel, callback_call_canceled)
    BobAV:registerCallback(ToxAv.cb.OnEnd, callback_call_ended)
    BobAV:registerCallback(ToxAv.cb.OnEnding, callback_recv_ending)
    BobAV:registerCallback(ToxAv.cb.OnError, callback_recv_error)

    -- register Alice's callbacks
    AliceAV:registerCallback(ToxAv.cb.OnStarting, callback_recv_starting)
    AliceAV:registerCallback(ToxAv.cb.OnRinging, callback_recv_ringing)
    AliceAV:registerCallback(ToxAv.cb.OnEnding, callback_recv_ending)
    AliceAV:registerCallback(ToxAv.cb.OnReject, callback_call_rejected)
    AliceAV:registerCallback(ToxAv.cb.OnError, callback_recv_error)

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
            local id, err = AliceAV:call(0, ToxAv.calltype.audio, 10);
            if not(id)then
                error(string.format("Couldn't get call id: %s", (err or "[no msg]")))
            end
            status_control.Alice.call_index = id

            step = step + 1

        elseif(1==step)then -- Bob
            if (status_control.Bob.status == CallStatus.Ringing) then
                print(" Bob answers...")
                local id, err = BobAV:answer(status_control.Bob.call_index, ToxAv.calltype.audio)
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

                -- Both receive
                -- Payload from Bob
                local storage, recved = AliceAV:recvAudio(status_control.Alice.call_index)
                assert(storage, "Payload from Bob is invalid")

                local storage, recved = BobAV:recvAudio(status_control.Bob.call_index)
                assert(storage, "Payload from Alice is invalid")

                if ( (os.time() - cur_time) > 10) then -- Transmit for 10 seconds
                    print(" killing Alice call...")
                    local r, call_id, e = AliceAV:killTransmission(status_control.Alice.call_index)
                    print(" killing Bob call...")
                    local r, call_id, e = BobAV:killTransmission(status_control.Bob.call_index)

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
            local id, call_index, err = AliceAV:call(0, ToxAv.calltype.audio, 10);
            if not(id)then
                error(string.format("Couldn't get call id: %s", (err or "[no msg]")))
            end
            status_control.Alice.call_index = id

            step = step + 1

        elseif(1==step)then -- Bob
            if (status_control.Bob.status == CallStatus.Ringing) then
                print( "Bob answers...")
                local id, call_index, err = BobAV:answer(status_control.Bob.call_index, ToxAv.calltype.audio)
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

                -- Both receive
                -- Payload from Bob
                local storage, recved = AliceAV:recvAudio(status_control.Alice.call_index)
                assert(storage, "Payload from Bob is invalid")

                local storage, recved = BobAV:recvAudio(status_control.Bob.call_index)
                assert(storage, "Payload from Alice is invalid")

                if ( (os.time() - cur_time) > 10) then -- Transmit for 10 seconds
                    print(" killing Alice call...")
                    local r, call_id, e = AliceAV:killTransmission(status_control.Alice.call_index)
                    print(" killing Bob call...")
                    local r, call_id, e = BobAV:killTransmission(status_control.Bob.call_index)

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
                local id, call_index, err = AliceAV:call(0, ToxAv.calltype.audio, 10);
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
                local id, call_index, err = AliceAV:call(0, ToxAv.calltype.audio, 10);
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

end

-- TESTS

test_init()


print("END")
