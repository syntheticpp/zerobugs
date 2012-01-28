ARCH=`which arch 2>/dev/null`
if test -z "$ARCH"; then
        ARCH="uname -m"
fi
ARCH=`${ARCH}`
if test "$ARCH" = x86_64; then
  LIB=lib64
  CFLAGS=-fPIC
fi
if test "$ARCH" = ppc; then
  CFLAGS=-fPIC
fi

