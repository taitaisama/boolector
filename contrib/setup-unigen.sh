#!/bin/bash

# Boolector: Satisfiablity Modulo Theories (SMT) solver.
#
# Copyright (C) 2007-2021 by the authors listed in the AUTHORS file.
#
# This file is part of Boolector.
# See COPYING for more information on using this software.
#

source "$(dirname "$0")/setup-utils.sh"

function install_dep
{
  local repo_dir="$1"
  local repo="$2"
  local version="$3"
  cd "${DEPS_DIR}"
  download_github "$repo" "$version" "$repo_dir"
  cd "$repo_dir"
  mkdir build && cd build
  cmake -DENABLE_PYTHON_INTERFACE=OFF \
        -DSTATICCOMPILE=ON \
	-DNOM4RI=ON \
	-DONLY_SIMPLE=ON \
	-DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
	-DCMAKE_LIBRARY_PATH="${INSTALL_DIR}" \
	-DCMAKE_PREFIX_PATH="${INSTALL_DIR}" \
	..
  make -j${NPROC} install
}

CDCL_DIR=${DEPS_DIR}/cadical
CDBK_DIR=${DEPS_DIR}/cadiback
CMS_DIR=${DEPS_DIR}/cryptominisat
ARJUN_DIR=${DEPS_DIR}/arjun
APPROXMC_DIR=${DEPS_DIR}/approxmc
UNIGEN_DIR=${DEPS_DIR}/unigen

cd "${DEPS_DIR}"
git clone https://github.com/meelgroup/cadical
cd "${CDCL_DIR}"
git checkout mate-only-libraries-1.8.0
./configure && make -j${NPROC}
cd "${DEPS_DIR}"
git clone https://github.com/meelgroup/cadiback
cd "${CDBK_DIR}"
git checkout mate
./configure && make -j${NPROC}

install_dep ${CMS_DIR} "msoos/cryptominisat" "5.11.21"
install_dep ${ARJUN_DIR} "meelgroup/arjun" "2.5.4"
install_dep ${APPROXMC_DIR} "meelgroup/approxmc" "4.1.24"
install_dep ${UNIGEN_DIR} "meelgroup/unigen" "2.5.7"
