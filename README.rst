This is a Lua binding for `Tox <https://tox.im/>`__, decentralised Instant Messaging.

This is alpha code.
Tests are passing all OK but it hasn't been used for a real implemenation yet,
so unknown bugs are highly expected.

There's a known bug with toxAV: it's crashing on exit with pthreads.
Lua 5.2 seems not affected by the bug, but lua 5.1 and luajit 2.0.X are.

A Luce IHM is to be expected very soon.

