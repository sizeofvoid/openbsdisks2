# OpenBSDisks2 - UDisks2 service implementation for OpenBSD
An UDisks2 service implementation for OpenBSD forked from FreeBSD (bsdutils/bsdisks)

## OpenBSD bsdutils/bsdisks fork

The sourcecode based on commit [3d3439348ba00ae972e146c5bf28cb42949e24f](https://foss.heptapod.net/bsdutils/bsdisks/-/commit/93d3439348ba00ae972e146c5bf28cb42949e24f) from [bsdutils/bsdisks](https://foss.heptapod.net/bsdutils/bsdisks).

### Feature List
 - [x] Simple DBus org.freedesktop.UDisks2.service
 - [X] Provides `org.freedesktop.UDisks2.Block` (Not all information yet)
 - [X] Provides `org.freedesktop.UDisks2.Drive` (Not all information yet)
 - [X] Provides `org.freedesktop.UDisks2.Filesystem` (Not all information yet)
 - [X] Logging via `stdout` and syslog
 - [ ] cd(4) support
 - [ ] mount(2), umount(2) support
 - [ ] Privilege separation
 - [ ] Device information updates

### TODO beta release
 - [x] Remove dead code path
 - [x] Remove new/delete idiom
 - [x] logging
 - [x] debug and info logging
 - [ ] Build port
 - [x] Feature List
 - [ ] Import


## UDisks2

- http://storaged.org/doc/udisks2-api/latest/
