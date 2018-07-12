# Galaxy42 - deterministic builds info for developers

## Important scripts / structure of builds

### Terms

- **Gitian**: Tool that build packages in a secure deterministic fashion inside a virtual machine:
    - [official site](https://gitian.org/)
    - [github page](https://github.com/devrandom/gitian-builder)

- **Gitian yaml:** <a name="gitian_yml"></a>
    Startup file in .yml format, where all gitian builds starts. It contain package names that should be installed after lxc machine setup, also gitian machine basic configuration like distribution name, version, remote git data, or if machine will require sudo and script section for maintain further actions inside ready to use virtual machine.  
    _Example for windows build:_

    ```yaml
    ---
    name: "galaxy42-windows"
    sudo: true
    enable_cache: true
    suites:
    - "xenial"
    architectures:
    - "amd64"
    packages:
    - "g++-5"
    - "mingw-w64"
    - "g++-mingw-w64"
    - "mingw-w64-tools"
    - "cmake"
    - "autoconf"
    - "automake"
    - "libtool"
    - "build-essential"
    - "libfftw3-dev"
    - "scons" # installer - for nsis build
    - "zlib1g-dev" # for nsis build -- unclear why it is needed  bug(GALAXY-226)
    - "ca-certificates"  #  because of: Problem with the SSL CA cert (path? access rights?)
    - "faketime" # used to force date
    - "zip"
    - "gettext-base" # some scripts/tools used in build could be using gettext translations
    - "gettext" # for msgfmt and other advanced tools to compile language .po to .mo
    reference_datetime: "2016-08-01 00:00:00"
    remotes:
    - "dir": "galaxy42"
      "url": "https://github.com/yedino/galaxy42.git" # this can be overwritten e.g. by gbuild called from ./build-gitian
    # this is temporary, TODO, in the end use branch name / tag name
    files: []
    script: |
      datenow="$(date)" ; echo "Gitian (Windows cross-build) moves to the Script section, in PWD=$PWD, at date=$datenow" ; datenow=""
      echo "ls:" ; ls
      ./galaxy42/contrib/gitian-descriptors/main-build/galaxy-windows-script-master -6 -3
      datenow="$(date)" ; echo "Gitian (Windows cross-build) finished it's Script section, in PWD=$PWD, at date=$datenow" ; datenow=""
    ```


### Multi-Architecture components

- **Global variables:**  <a name="global_vars"></a>  
    `./contrib/gitian-descriptors/main-build/galaxy-linux.yml`

    File that contain important gitian variables. This variables are used in different builds. Example of import in bash script:
    ```
    # source global variables
    source "${HOME}/build/galaxy42/contrib/gitian-descriptors/gitian_global-variables"
    ```

    - Content:

    ```bash
    #!/usr/bin/env bash

    set -o errexit
    set -o nounset

    readonly PROJECT_NAME="galaxy42"
    readonly REFERENCE_DATETIME="2016-08-01 00:00:00"

    readonly GALAXY_DIR="$HOME/build/galaxy42"
    readonly BUILD_DIR="$HOME/build" # where deps are built
    readonly WRAP_DIR="$HOME/wrapped"
    readonly OUTDIR="$HOME/out"

    # Parse Options
    # This variables could be changed manually or by parent script for example by parsing command line arguments
    option_allow_boost=1
    option_allow_sodium=1
    option_shell_in_sodium=0
    option_download_skip=0
    option_verbose=0
    ```

- **faketime wrappers:** <a name="faketime"></a>  
    `./share/script/lib/create_faketime_wrappers.sh`  
    Key functions that determine the deterministic result of builds. Wrap binaries needed in build process with faketime.

- **build-boost:** <a name="boost"></a>  
    `./contrib/gitian-descriptors/libboost-build/dl-boost`  
    `./contrib/gitian-descriptors/libboost-build/dl-boost-submodules`  
    `./contrib/gitian-descriptors/libboost-build/build-boost-multiarch`  
    Collection of three script that clone and verifies main boost, download necessary submodules and build boost libraries. Final libraries depend on the target architecture.

- **build-sodium** <a name="sodium"></a>  
    `contrib/gitian-descriptors/libsodium-build/dl-sodium`  
    `contrib/gitian-descriptors/libsodium-build/build-sodium`  
    Two scripts for download/verify libsodium sources, and built from sources. Final libraries depends on the target architecture.

### Linux
- [Gitian yaml](#gitian_yml):  
    `./contrib/gitian-descriptors/main-build/galaxy-linux.yml`
- Main bash script executed in `gitian-linux.yml`:  
    `./contrib/gitian-descriptors/main-build/galaxy-linux-script`

    - uses:
        - [global variables](#global_vars)
        - [faketime wrappers](#faketime)
    - calls:
        - [boost build](#boost)
        - build pack for linux:  
        `./contrib/tools/build-pack-linux.sh`
    - script responsibility:
        - build deterministic static linux version of galaxy42 using `do` script, then pack it to .tar.gz archive.

### Mac OSX
- [Gitian yaml](#gitian_yml):  
    `./contrib/gitian-descriptors/main-build/galaxy-osx.yml `
- Main bash script executed in `gitian-osx.yml`:  
    `./contrib/gitian-descriptors/main-build/galaxy-osx-script`

    - uses:
        - [global variables](#global_vars)
        - [faketime wrappers](#faketime)
    - calls:
        - script that download and build [OS X Cross toolchain](https://github.com/tpoechtrager/osxcross):  
        `./contrib/macdeploy/build-cross-toolchain.sh`
        - [boost build](#boost)
        - [sodium build](#sodium)
        - script that carries out the whole process of creating osx-compatible package (.pkg) and pack it to osx dmg-image format:
        `./contrib/gitian-descriptors/galaxy-osx_pack-to-dmg.sh`
    - script responsibility:
        - build deterministic .dmg image with osx compatible pkg-installer for galaxy42.

### Windows
- [Gitian yaml](#gitian_yml):  
    `./contrib/gitian-descriptors/main-build/galaxy-windows.yml`
- Main bash script executed in `gitian-linux.yml`:  
    `./contrib/gitian-descriptors/main-build/galaxy-windows-script-master`  
    For custom build usually inside gitian machine, master script takes arguments:
    ```bash
    This is the MASTER build script (that calls the other scripts).
    To see more options you can directly call the other script, like:
    /contrib/gitian-descriptors/main-build/galaxy-windows-script -h

    This MASTER script should be called with option:
    -3 for 32bit build
    -6 for 64bit build
    -C skip the cd to directory that is expected inside Gitian. For fast tests outside of Gitian.

    any other FURTHER options will be also passed to the other script (/contrib/gitian-descriptors/main-build/galaxy-windows-script)
    ```

    Windows main script (master) calls twice secondary script, independently for 32-bit and 64-bit versions:  
    `./contrib/gitian-descriptors/main-build/galaxy-windows-script`

    - uses:
        - [global variables](#global_vars)
        - [faketime wrappers](#faketime)
    - calls:
        - [boost build](#boost)
        - [sodium build](#sodium)
        - build [nsis installer](https://github.com/kichik/nsis):  
            `./contrib/nsis-installer-windows/galaxy-windows-installer`  
                - require deterministic [zlib](https://github.com/madler/zlib) build  
                - includes galaxy-windows-service from:   
                `./contrib/windows-service-wrapper/galaxyService.exe`  
                - includes language files generated by:  
                `./contrib/tools/galaxy42-lang-update-all`  
                - it packages the files located in:  
                `./contrib/nsis-installer-windows/bin/`  
                - script that generates a list of files in nsis format:  
                `./contrib/nsis-installer-windows/bin/generate_nsis_filelist.sh`

    - scripts responsibility:
        - build deterministic galaxy42 windows installer for both 32-bit and 64-bit versions.

## Other utilities:
- fail function:  
    `./share/script/lib/fail.sh`
    error exit and debug function.
- fdfix function:  
    `./share/script/lib/fdfix.sh`
    function that fix for /dev/fd interface
