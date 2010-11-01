ulimit -c unlimited

rm -f core* a.out.core
./a.out $COREARG
if eval mv core* core
then :
else mv -f a.out.core core
fi
