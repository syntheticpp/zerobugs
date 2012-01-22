#! /usr/bin/env bash

LIB=lib
ARCH=`uname -m`
if test "$ARCH" = "x86_64"; then
    LIB=lib64
fi

function read_param()
{
    if [ -z "${!1}" ]
    then
        echo -n "$3 ["$2"] ";
        read $1
    fi
    if [ -z "${!1}" ]
        then export $1="$2"
    fi
}

for argc in $@ 
do
    case $argc in
    --prefix=*) PREFIX=${argc/--prefix=/};;
    --wrap) WRAP=1
    esac
done

read_param PREFIX /usr/local "Please enter directory where you want to install"

FILES=`mktemp files.XXXXXX`

DISASM=zdisasm.so

function install_file()
{
    DEST=$PREFIX/$2
    mkdir -p `dirname "$DEST"`
    cp -r -v -u $SRC "$DEST" 2>/dev/null 
    echo $DEST >> files.lst
}


# Install a wrapper script that conveniently sets
# the environment variables

function install_wrapper_script()
{
echo "#! /usr/bin/env bash
# Wrapper script for the Zero Debugger
# Copyright 2006 - 2010  Cristian Vlasceanu

# Workaround broken __mt_alloc
export GLIBCXX_FORCE_NEW=1
export GLIBCPP_FORCE_NEW=1

# Gotta be careful with empty LD_LIBRARY_PATH, because of
# this bug: http://sourceware.org/bugzilla/show_bug.cgi?id=4776
#
if test -n "$LD_LIBRARY_PATH"; then
  export LD_LIBRARY_PATH=\$LD_LIBRARY_PATH:$PREFIX/zero/lib
else
  export LD_LIBRARY_PATH=$PREFIX/zero/lib
fi

#save settings and history in current user's HOME by default
if [ -z \"\$ZERO_CONFIG_PATH\" ]; then 
    export ZERO_CONFIG_PATH=~
fi

if [ -z \"\$ZERO_PLUGIN_PATH\" ]; then 
    export ZERO_PLUGIN_PATH=$PREFIX/zero/plugin
fi

export ZERO_HELP_PATH=$PREFIX/zero/help

#start debugging at main() rather than at CRT start;
#comment this out if your debugged programs don't have a 
# main() function
#if [ -z \"\$ZERO_START_MAIN\" ]; then
#    export ZERO_START_MAIN=1
#fi 

#set this to use stack frame information from plugins
#note: if not set, it defaults to true on x86_64
#export ZERO_USE_FRAME_HANDLERS=1

#use hardware breakpoints whenever available (default);
#turn it off on Virtual PC
if [ -z \"\$ZERO_HARDWARE_BREAKPOINTS\" ]; then 
    export ZERO_HARDWARE_BREAKPOINTS=1
fi

#Use expensive type lookups with DWARF
# If set, attempt to find the full type definition, rather than just
# being happy with a forward declaration that matches.
# If a full definition is not found in the current module, lookup all
# program modules.
export ZERO_EXPENSIVE_TYPE_LOOKUPS=2

#do not dive into assembly when stepping from a portion
#where we do have C/C++ source code, into another one
#where source is not available
if [ -z \"\$ZERO_SOURCE_STEP\" ]; then 
    export ZERO_SOURCE_STEP=1
fi

if [ -z \"\$ZERO_QUICK_SHUTDOWN\" ]; then 
    export ZERO_QUICK_SHUTDOWN=1
fi

if [ -z \"\$ZERO_SYMBOL_DB\" ]; then 
    export ZERO_SYMBOL_DB=false
fi

" > "$PREFIX"/bin/zero

if [ "$WRAP" ]
then echo "###" >> "$PREFIX"/bin/zero
fi

echo "$PREFIX/bin/zero-bin --no-banner \$@" >> "$PREFIX"/bin/zero

if chmod +x "$PREFIX/bin/zero"; then
    if [ -x /sbin/chcon ]
        then /sbin/chcon -t textrel_shlib_t "$PREFIX/zero/plugin"
    fi
    echo "INSTALLATION SUCCESSFUL"

    rm $FILES
fi
}


if test -n "$WRAP"; then 
    install_wrapper_script; exit 0
fi

rm -rf $PREFIX/zero
rm -f $PREFIX/bin/zero
rm -f $PREFIX/bin/zero-bin

FILE_LIST="
bin/zero                bin/zero-bin\n
bin/zserver             bin/zserver\n
plugin/$DISASM          zero/plugin/$DISASM\n
plugin/zdwarf.so        zero/plugin/zdwarf.so\n
plugin/zstabs.so        zero/plugin/zstabs.so\n
plugin/zgui.so          zero/plugin/zgui.so\n
plugin/zpython.so       zero/plugin/zpython.so\n
plugin/zremote-proxy.so zero/plugin/zremote-proxy.so\n
plugin/update.py        zero/plugin/update.py\n
plugin/.zero.py         zero/plugin/.zero.py\n
misc/zero-gtkrc-2.0     zero/plugin/zero-gtkrc-2.0\n
lib/libdwarf.so         zero/lib/libdwarf.so\n
lib/libdemangle_d.so    zero/lib/libdemangle_d.so\n
help/          			zero/help\n
zeroicon.png            zero/zeroicon.png\n
LIBDWARFCOPYRIGHT       zero/LIBDWARFCOPYRIGHT"


echo -e $FILE_LIST > $FILES

#for i in lib/libgtksourceviewmm-1.0.so*
for i in lib/libgtksourceviewmm-1.0.so.1
do
    echo "$i    zero/lib/`basename $i`" >> $FILES
done


cat $FILES | while read SRC DEST 
do
    install_file $SRC $DEST 
done
mkdir -p /usr/share/applications
cp zero.desktop /usr/share/applications

rm -rf $PREFIX/zero/help/.svn

install_wrapper_script

