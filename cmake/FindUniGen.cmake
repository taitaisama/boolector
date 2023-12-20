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
find_path(ApproxMC_INCLUDE_DIR NAMES approxmc/approxmc.h)
find_path(Arjun_INCLUDE_DIR NAMES arjun/arjun.h)
find_path(CryptoMiniSat_INCLUDE_DIR NAMES cryptominisat5/cryptominisat.h)
find_library(UniGen_LIBRARIES NAMES unigen)
find_library(ApproxMC_LIBRARIES NAMES approxmc)
find_library(Arjun_LIBRARIES NAMES arjun)
find_library(CryptoMiniSat_LIBRARIES NAMES cryptominisat5)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(UniGen
  DEFAULT_MSG UniGen_INCLUDE_DIR UniGen_LIBRARIES
  ApproxMC_INCLUDE_DIR ApproxMC_LIBRARIES
  Arjun_INCLUDE_DIR Arjun_LIBRARIES
  CryptoMiniSat_INCLUDE_DIR CryptoMiniSat_LIBRARIES)
  
# find_package_handle_standard_args(ApproxMC
#   DEFAULT_MSG ApproxMC_INCLUDE_DIR ApproxMC_LIBRARIES)
# find_package_handle_standard_args(Arjun
#   DEFAULT_MSG Arjun_INCLUDE_DIR Arjun_LIBRARIES)
# find_package_handle_standard_args(CryptoMiniSat
#   DEFAULT_MSG CryptoMiniSat_INCLUDE_DIR CryptoMiniSat_LIBRARIES)

mark_as_advanced(UniGen_INCLUDE_DIR UniGen_LIBRARIES)
mark_as_advanced(ApproxMC_INCLUDE_DIR ApproxMC_LIBRARIES)
mark_as_advanced(Arjun_INCLUDE_DIR Arjun_LIBRARIES)
mark_as_advanced(CryptoMiniSat_INCLUDE_DIR CryptoMiniSat_LIBRARIES)
if(UniGen_LIBRARIES AND ApproxMC_LIBRARIES AND Arjun_LIBRARIES AND CryptoMiniSat_LIBRARIES)
  message(STATUS "Found UniGen library: ${UniGen_LIBRARIES} ${ApproxMC_LIBRARIES} ${Arjun_LIBRARIES} ${CryptoMiniSat_LIBRARIES}")
endif()
