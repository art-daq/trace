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
    mkdir build; cd build
    cmake .. -DCMAKE_INSTALL_PREFIX=$PWD
    make install
    export PATH=$PWD/bin:$PATH PYTHONPATH=$PWD/python LD_LIBRARY_PATH=$PWD/lib64
    . etc/profile.d/trace_functions.sh
    tcntl TRACE INFO hello
    python -c 'import TRACE;TRACE.INFO("hello")'
```

### spack - work-in-progress

```
    spack find --format "{name}@{version}%{compiler}/{hash} {arch}={platform}-{os}-{target}" trace
    cd trace
    spack repo add $PWD/spack
    spack install --reuse trace@develop arch=`uname -m`    # 

    # for "No valid compiler version found..." do:
    spack compiler find  # and then redo spack install... (above)

    # for other errors you may have to:
    spack uninstall trace@develop # and/or:
    spack repo rm trace # and then do spack repo add... (above)

    spack load trace@develop
```
