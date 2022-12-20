# This file (package.py) was created by Ron Rechenmacher <ron@fnal.gov> on
# Dec 16, 2022. "TERMS AND CONDITIONS" governing this file are in the README
# or COPYING file. If you do not have such a file, one can be obtained by
# contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
# $RCSfile: .emacs.gnu,v $
# rev="$Revision: 1.34 $$Date: 2019/04/22 15:23:54 $";

from spack import *
import os
from spack.util.environment import EnvironmentModifications

class Trace(CMakePackage):
    """TRACE is a logging package. It features 2 paths - slow and fast.
The slow path can be configure to be your favorite logger.
The fast path is a circular memory buffer. There is a separate utility to
print log messages in the circular buffer. THere are several other features."""

    homepage = "https://github.com/art-daq/trace"
    git      = "https://github.com/art-daq/trace.git"

    version('3.17.06') # commit hash can be added when the package is in another spack repo

    def setup_run_environment(self, env):
        file_to_source = "bin/trace_functions.sh"
        try:
            env.extend(EnvironmentModifications.from_sourcing_file(
                file_to_source, clean=True
            ))
        except Exception as e:
            msg = 'unexpected error when sourcing file [{0}]'
            print(msg.format(str(e)))
