function run_valgrind()
{
    NODLCLOSE=1 valgrind -v --num-callers=16 \
		--log-file=log \
		--leak-check=full $@
}
