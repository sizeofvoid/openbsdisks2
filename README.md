# OpenBSDisks2 - UDisks2 service implementation for OpenBSD
An UDisks2 service implementation for OpenBSD forked from FreeBSD (bsdutils/bsdisks)

## OpenBSD bsdutils/bsdisks fork

The sourcecode based on commit [3d3439348ba00ae972e146c5bf28cb42949e24f](https://foss.heptapod.net/bsdutils/bsdisks/-/commit/93d3439348ba00ae972e146c5bf28cb42949e24f) from [bsdutils/bsdisks](https://foss.heptapod.net/bsdutils/bsdisks).

## Build on OpenBSD
```bash
$ doas pkg_add qtbase
$ Qt5_DIR=/usr/local/lib/qt5/cmake cmake -G Ninja -DCMAKE_BUILD_TYPE=Debug ~/src/github/openbsdisks2
$ ninja -v
# Run install to dbus-1 files
$ ninja install
# Run a simple test
$ ./openbsdisks2 -d -v
# Run clang-tidy
$ doas pkg_add clang-tools-extra
$ clang-tidy --list-checks -checks='*' | grep "modernize"
$ run-clang-tidy -header-filter='.*' -checks='-*,modernize-use-nullptr'
# and fix it
$ run-clang-tidy -header-filter='.*' -checks='-*,modernize-use-nullptr' -fix
```

### Feature List
 - [X] Simple DBus org.freedesktop.UDisks2.service
 - [X] Provides `org.freedesktop.UDisks2.Block`
 - [X] Provides `org.freedesktop.UDisks2.Drive`
 - [X] Provides `org.freedesktop.UDisks2.Filesystem` (Not all information yet)
 - [X] Logging via `stdout` and syslog
 - [X] cd(4) support
 - [X] mount(2), umount(2) support (ffs only)
 - [ ] Privilege separation
 - [ ] Device information updates

## UDisks2

- http://storaged.org/doc/udisks2-api/latest/
