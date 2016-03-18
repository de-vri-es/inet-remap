# Introduction
Force programs to bind to different ports.

`inet-remap` is not meant for regular use.
However, sometimes you're faced with a closed source binary
that just has bind to some specific port.
For whatever reason, you may want to have it bind to a different port.
In such cases, `inet-remap` can help you bend that closed source binary to your will.

Use cases include forcing an application to bind to an unprivileged port
and running the same application twice, each instance binding to different ports.

`inet-remap` uses `LD_PRELOAD` to intercept `bind()` calls.
When a `bind()` call is intercepted, the protocol and port number used
are looked up in a table parsed from the `INET_REMAP` environment variable.
You can specify a remapping as `protocol:old_port:new_port`,
where `protocol` can be either `tcp` or `udp`.
You can specify multiple remappings by separating them with a comma or a space.

For example, to remap TCP port 7331 to 1337:
```
inet-remap -b tcp:7331:1337 ./program
```

Which behind the scenes is the same as:
```
LD_PRELOAD=libinet-remap-preload.so INET_REMAP=tcp:7331:1337 ./program
```

Or remap multiple ports:
```
inet-remap -b tcp:503:2503 -b udp:53:2053  ./program
```

Which comes down to:
```
LD_PRELOAD=libinet-remap-preload.so INET_REMAP=tcp:503:2503,udp:53:2053 ./program
```

Note that as a security precaution `LD_PRELOAD` and thus `inet-remap` do not work with setuid binaries
or binaries with elevated privileges on most UNIX systems.

## TODO:
* ~~Finish setting up build scripts.~~
* ~~Support IPv6.~~
* Make protocol specification in remaps optional.
* Support filtering based on IP address as well as protocol and port number.
* Support for intercepting `connect()` calls to alter remote endpoints.
* Get rid of non-POSIX SO_PROTOCOL socket option.
* ~~Add an executable/script wrapper for easy invocation.~~
