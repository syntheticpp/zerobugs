#!/bin/bash
#
# Run autotools for udis86
#

function search_exe {
  for path in `echo $PATH | sed -e 's/:/ /g'` 
  do
    if [ -e ${path}/$1 ]; then
       return 0
    fi
  done
  return 1
}

function run_auto_tool {
  echo autogen: running ... $*
  $* || {
    echo "autogen: failed to run $1"
    exit 1
  }
}

run_auto_tool aclocal

# run libtoolize
if [ ! \( -e libtoolize \) ]; then
  echo autogen: libtoolize not found, looking for gtoolize 
  search_exe glibtoolize # mac os x
  if [ ! \( $? -eq 0 \) ];
    then echo autogen: glibtoolize not found.
    exit 1
  fi
  run_auto_tool glibtoolize --force --copy
else
  run_auto_tool libtoolize --force --copy
fi

run_auto_tool autoheader
run_auto_tool automake --add-missing --copy --foreign
run_auto_tool autoconf
