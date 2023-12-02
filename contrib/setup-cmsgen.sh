#!/bin/bash

# Boolector: Satisfiablity Modulo Theories (SMT) solver.
#
# Copyright (C) 2007-2021 by the authors listed in the AUTHORS file.
#
# This file is part of Boolector.
# See COPYING for more information on using this software.
#

source "$(dirname "$0")/setup-utils.sh"

CMSGEN_DIR=${DEPS_DIR}/cmsgen

# download_github "meelgroup/cmsgen" "1.1" "$CMSGEN_DIR"
cd "${DEPS_DIR}"

git clone https://github.com/meelgroup/cmsgen.git

cd "${CMSGEN_DIR}"

mkdir build
cd build
cmake -DSTATICCOMPILE=ON \
      -DNOM4RI=ON \
      -DONLY_SIMPLE=ON \
      -DCMAKE_INSTALL_PREFIX="${INSTALL_DIR}" \
      ..
make -j${NPROC} install
