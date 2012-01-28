# vim: tabstop=4:softtabstop=4:noexpandtab:shiftwidth=4 

clean: $(USER_CLEAN)
	rm -rf $(OBJS) $(TARGET) $(EXTRA_CLEAN)
	rm -rf autom4te.cache config.status config.log configure.lineno
	rm -f depend.stamp

tidy: clean
	rm -f depend.make libdep.make

depend: depend.stamp

depend.stamp:
	rm -f libdep.make
	$(MAKE) libdep.make
	$(CXX) $(CXXFLAGS) -MM $(DEPSRCS) > depend.make
	for f in $(LIB_DIRS); do ($(REALPATH) $$f >> $(LIB_LIST); cd $$f; $(MAKE) depend); done
	touch $@

-include libdep.make
-include depend.make
