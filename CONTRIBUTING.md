## How to Contribute:

Version-agnostic features should be merged into the default and protected `devel` branch via a peer-reviewed MR process.

#### Branching

A good branch name tells someone unfamiliar with the project what that branch does.  Branch names should lead with the following:

* `feat/` -> Feature I'm adding or expanding
* `maint/` -> Refactors, reorganizations
* `fix/` -> Bug fix
* `sandbox/` -> Throwaway branch to experiment and not merge back in
* `v/` -> stable but untested version branch for API updates

and then be followed by a descriptive name separated by hyphens.  Here are some examples:

* `maint/remove-duplicate-steering-tests`
* `fix/broken-cmake-pointer`
* `feat/modularize-diagnostics-tool`
* `sandbox/test-cucumber`
* `v/0.99.99`

#### Versioning

Changes in the [API mobile app requirements](doc/api/API.md) will often incur breaking changes to the main `devel` stream.  To manage those breaking changes, version-specific branches will be leveraged according to these rules:

* Version-specific branches shall be created with a `v/xx.xx.xx` nomenclature to signify that work on that branch is towards a new API release.
* These branches function as a sub-development branch and shall be protected via a peer-reviewed MR process as `devel`.
* The target is eventual merging to `devel` and deletion of version-specific feature branches upon successful integration testing with Android and iOS mobile apps.
* Subsequent bug catches after integration testing can be merged straight to `devel`.  This should not be the norm.  If a version tag had been applied, overwrite it with the `$ git tag -f` command.
* Version-agnostic features shall be merged into the default and protected `devel` branch via a peer-reviewed MR process as described above.

#### Merging to `main`

A version release can be made upon successful integration testing against mobile app by merging `devel` to `main` and applying the `xx.xx.xx` tag to signify that that is the frozen codebase for that specific version.
