# Boolector: Satisfiablity Modulo Theories (SMT) solver.
#
# Copyright (C) 2007-2021 by the authors listed in the AUTHORS file.
#
# This file is part of Boolector.
# See COPYING for more information on using this software.
#

# Find UniGen
# UniGen_FOUND - found UniGen lib
# UniGen_INCLUDE_DIR - the UniGen include directory
# UniGen_LIBRARIES - Libraries needed to use UniGen

find_path(UniGen_INCLUDE_DIR NAMES unigen/unigen.h)
find_library(UniGen_LIBRARIES NAMES unigen)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UniGen
  DEFAULT_MSG UniGen_INCLUDE_DIR UniGen_LIBRARIES)

mark_as_advanced(UniGen_INCLUDE_DIR UniGen_LIBRARIES)
if(UniGen_LIBRARIES)
  message(STATUS "Found UniGen library: ${UniGen_LIBRARIES}")
endif()
