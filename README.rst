======
LuaTox
======
---------------------
A Lua binding for Tox
---------------------

This is a LuaTox, a Lua binding for `Tox <https://tox.im/>`__, a distributed, secure messenger with audio and video chat capabilities.

This is alpha code.

Tests are passing all OK but it hasn't been tested with a real implementation yet,
so bugs are highly expected.

There's a known bug with toxAV: it's crashing on exit with pthreads. Annoying, but doesn't seem to affect the process itself.
Lua 5.2 seems not to be affected by this bug though, but lua 5.1 and luajit 2.0.X are.

A Luce IHM should come any time soon.
