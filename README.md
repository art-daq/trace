# TRACE Front Page

## About the toolkit

The _artdaq_ toolkit is a data-acquisition framework designed for high-energy physics experiments. It provides a flexible, reliable backbone for data transfers and has several locations where users can perform custom analysis tasks using the _art_ framework.

The _artdaq_ suite consists of the following packages:

* [trace](https://art-daq.github.io/artdaq_doxygen/trace): High-performance message logging
* [artdaq-core](https://art-daq.github.io/artdaq_doxygen/artdaq-core): Data formats used by the artdaq toolkit
* [artdaq-utilities](https://art-daq.github.io/artdaq_doxygen/artdaq-utilities): Online tools, primarily metrics reporting
* [artdaq-mfextensions](https://art-daq.github.io/artdaq_doxygen/artdaq-mfextensions): Extensions to the MessageFacility product which are useful in DAQ context
* [artdaq](https://art-daq.github.io/artdaq_doxygen/artdaq): Application and data transfer framework
* [artdaq-core-demo](https://art-daq.github.io/artdaq_doxygen/artdaq-core-demo): Data formats used by the artdaq demonstration system
* [artdaq-demo](https://art-daq.github.io/artdaq_doxygen/artdaq-demo): "User" implementations for the artdaq demonstration system
* [artdaq-daqinterface](https://art-daq.github.io/artdaq_doxygen/artdaq-daqinterface): Command line run control and example configurations
* [artdaq-database](https://art-daq.github.io/artdaq_doxygen/artdaq-database): Bindings for MongoDB or local "filesystemdb" configuration databases
* [artdaq-epics-plugin](https://art-daq.github.io/artdaq_doxygen/artdaq-epics-plugin): Metric endpoint for the EPICS control system

## About this package

This package consists of the TRACE high-performance message logging package. It can be built and used independently from the rest of the _artdaq_ suite.

To get started, see the [quick-start.txt](doc/quick-start.txt) file.

Please note that using a version of gcc that supports the c11 and/or c++11
standard is desirable.

RPM packaging provided by Pat Riehecky on the Scientific Linux team.

Public readonly GIT access via:
  `git clone https://github.com/art-daq/trace`

tar files with binaries at:
  `https://scisoft.fnal.gov/scisoft/packages/TRACE/`

Public svn export
  `svn export http://cdcvs.fnal.gov/subversion/trace-svn/trunk/ trace`

Authorized read-write SVN access via:
  `svn co svn+ssh://p-trace@cdcvs.fnal.gov:/cvs/projects/trace-svn/trunk trace`

## BUILDING:

### make:

```
    cd trace
    make OUT=$PWD 
    PATH=$PWD/*/bin:$PATH
    . script/trace_functions.sh
    # if UPS environment, 2 lines above can be replaced by: setup -r$PWD-z$PWD TRACE 
```

### cmake

```
    cd trace
	# next 4 to git :) cetmodules; used in building examples
	git clone git@github.com:FNALssi/cetmodules -b3.27.03 # use tag closest to cmake version
	mkdir cetmodules/build;cd $_
	cmake .. -DCMAKE_INSTALL_PREFIX=$PWD && make install && export CMAKE_PREFIX_PATH=$PWD
	cd ../..
    mkdir build; cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=$PWD
    make install
    echo ":$PATH:" | grep :$PWD/bin: || PATH="$PWD/bin:$PATH"
	export PYTHONPATH=$PWD/python LD_LIBRARY_PATH=`echo $PWD/lib*`
    . etc/profile.d/trace_functions.sh
    tcntl TRACE INFO hello
    python -c 'import TRACE;TRACE.INFO("hello")'
```

  For a system install, use either `-DCMAKE_INSTALL_PREFIX=/usr` or `-DCMAKE_INSTALL_PREFIX=/`.
  Most install destinations follow GNUInstallDirs and land under `/usr` either way (for example, headers in `/usr/include/TRACE`, binaries in `/usr/bin`, docs in `/usr/share`, and source/support trees in `/usr/share/TRACE` by default).

  Two destinations are intentionally special:
  - `trace_functions.sh` is installed to `/etc/profile.d` for system integration.
  - If `-DWANT_KMOD=ON`, `TRACE.ko` installs to `/lib/modules/...` for system prefixes (`/` and `/usr`). For non-system prefixes, it installs under `<prefix>/lib/modules/...`.

  To skip source/support tree installation entirely, configure with `-DTRACE_INSTALL_SOURCE_TREE=OFF`.

### spack - work-in-progress

```
    wget https://raw.githubusercontent.com/art-daq/artdaq_demo/refs/heads/develop/tools/setup_spack_build_system_v0.28.sh
    echo "c70e6c68ec1f7fddbf4afdcd5b24a3b1d9bbb660  setup_spack_build_system_v0.28.sh" | sha1sum -c -
    
    source setup_spack_build_system_v0.28.sh
    # Note that install_spack_build_system sources setup-env.sh
    install_spack_build_system $PWD spack 0 # Installs Spack into the spack/ subdirectory
    spack find --format "{name}@{version}%{compiler}/{hash} {arch}={platform}-{os}-{target}" trace
    spack install --reuse trace@develop ~mf # Don't install MessageFacility dependency

    spack load trace@develop
```
