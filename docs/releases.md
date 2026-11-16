# Beta and stable

Every merge that changes an app ships a **beta**: a GitHub pre-release with the next patch version
(0.38.1, 0.38.2, ...). Promotion to **stable** rebuilds a beta's commit as the next minor version (beta
0.38.7 becomes 0.39.0), makes it GitHub's "Latest" and moves the `stable` branch to it. The next beta
is 0.39.1. Beta boards skip 0.39.0: it is code they already run.

- The **Promote** workflow runs each morning. It promotes the newest beta that has been out for 72 hours,
  is newer than stable, and is not yanked. So stable trails beta by about three days.
- To keep a broken beta off stable, open Actions, pick **Yank**, and run it with the tag and what is
  wrong. Its title gets "(yanked)" and Promote skips it.
- To promote now, run **Promote** by hand. With a tag it promotes that tag; with no tag, the newest
  beta that is not yanked, whatever its age.
- To roll stable back, promote an older tag. The old code goes out as the next minor version, so every
  stable board takes it. Betas after a rollback still carry `main`; yank them until the fix lands.

Each release also has `<app>-<version>-ota.bin` (the app image alone) and `<app>-version.txt` (the bare
version string) for boards that update themselves. A rolling `<app>-beta` pre-release holds a copy of the
newest `<app>-version.txt`, and beta boards read it there. `tools/check_channel.sh porthole stable|beta`
finds a channel's version and image the same way a board does. The release and promote workflows run it
after they publish.

Releases are automatic. After CI passes on `main`, each new `feat:` or `fix:` in an app makes a new
patch version of that app, published as a beta with notes and a flashable factory image. See
[Releases](https://github.com/dreamiurg/porthole/releases).
