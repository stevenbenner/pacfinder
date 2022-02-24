# PacFinder Release Process

This file documents the PacFinder release process to ensure consistent releases.
It is only of use to the maintainer.

## Release checklist

Complete these steps in order.

  1. Verify that `make distcheck` passes
  2. Verify po files are up to date: `make -C po update-po`
  3. Update NEWS file
  4. Bump version number in configure.ac file (see: Versioning)
  5. Make sure you're configured with optimizations: `./configure`
  6. Run `make dist`
  7. Sign dist file: `gpg --detach-sign pacfinder-[VERSION].tar.zst`
  8. Generate checksum file: `b2sum -b pacfinder-* > B2SUMS`
  9. Tag version bump commit: `git tag -s vX.Y -m 'PacFinder vX.Y release'`
 10. Create GitHub release based on git tag
 11. Upload the following files to the GitHub release:
     - pacfinder-[VERSION].tar.zst
     - pacfinder-[VERSION].tar.zst.sig
     - B2SUMS
 12. Update AUR package

## Versioning

Version scheme: [MAJOR].[MINOR]

 - **MAJOR:** Bump when something the user interacts with has functional changes
              from the previous version (e.g. GUI, CLI, input, output).

 - **MINOR:** Bump when the changes only fix bugs in user-facing elements, or
              the changes do not alter UI structure, behavior, or output.

Reset the MINOR version number to `0` when bumping the MAJOR version.
