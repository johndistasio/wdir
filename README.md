# wdir

This is a dinky tool for clearing the contents of the current working directory.

Like `rm`, it won't remove hidden files.

Inspired by my old habit of not paying attention:

```
~$ cd oldtempfir   // oops, typo, that's supposed to be 'dir', not 'fir'
~$ rm -rf ./*      // :(
```

It solves this problem by refusing to wipe your home directory without a flag.

**Usage:**

```
-h, --help:                print this message and exit
-v, --verbose:             print what wdir is doing
-d, --dry-run:             print what wdir would do
-u, --no-preserve-home:    wipe user's home
-r, --no-preserve-root:    wipe /
-f, --force:               don't prompt before wipe
-V, --version:             print the current version and exi
```
