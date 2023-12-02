# Boolector: Satisfiablity Modulo Theories (SMT) solver.
#
# Copyright (C) 2007-2021 by the authors listed in the AUTHORS file.
#
# This file is part of Boolector.
# See COPYING for more information on using this software.
#

# Find CMSGen
# CMSGen_FOUND - found CSMGen lib
# CMSGen_INCLUDE_DIR - the CMSGen include directory
# CMSGen_LIBRARIES - Libraries needed to use CMSGen

find_path(CMSGen_INCLUDE_DIR NAMES cmsgen/cmsgen.h)
find_library(CMSGen_LIBRARIES NAMES cmsgen)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(CMSGen
  DEFAULT_MSG CMSGen_INCLUDE_DIR CMSGen_LIBRARIES)

mark_as_advanced(CMSGen_INCLUDE_DIR CMSGen_LIBRARIES)
if(CMSGen_LIBRARIES)
  message(STATUS "Found CMSGen library: ${CMSGen_LIBRARIES}")
endif()
