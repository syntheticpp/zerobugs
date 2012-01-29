#!/usr/bin/env bash

# Fix svn:mime-type property

dir=${1:-.}

find $dir -name "*.cpp" -exec svn propset svn:mime-type text/plain {} \;
find $dir -name "*.h" -exec svn propset svn:mime-type text/plain {} \;
find $dir -name "Makefile" -exec svn propset svn:mime-type text/plain {} \;

