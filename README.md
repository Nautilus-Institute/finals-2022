# The DEF CON CTF 2022 Final #

This repository contains the open source release for Nautilus Institute's 2022
DEF CON CTF final.

We are releasing all of the source code for every challenge that was released
during the game. In most cases, this also includes all of the code required to
build that source code into a working challenge (such as `Makefile`s and
`Dockerfile`s). It *does not* include the infrastructure required to *host*
those challenges (e.g. our CI/CD pipeline, deployment scripts, and so on).

## Files ##

Each folder in this repository is the full source for a single challenge.
These challenges were, in release order:

* `Web4Factory`
* `router-pi`
* `Perplexity`
* `mambo`
* `corewar-nii`
* `nivisor`

The `NautilusRand` library is also included, which was a dependency for the
`Perplexity` challenge.

## License ##

Everything in this repository, unless otherwise stated, is being released under
the MIT license. See [`LICENSE.md`](./LICENSE.md) for more details.

## Contact ##

Questions, comments, and/or concerns can be sent to
[@fuzyll](https://github.com/fuzyll), who is happy to direct things to the
appropriate party from there.
