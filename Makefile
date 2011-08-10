MAKE = make
CP = cp

otoperld: otoperld-src/otoperld
	$(CP) $^ .

otoperld-start.pl: otoperld-src/otoperld-start.pl
	$(CP) $^ .

otoperld-src/otoperld:
	$(MAKE) -C otoperld-src

clean: 
	$(MAKE) clean -C otoperld-src

test: otoperld otoperld-start.pl
	./otoperld
