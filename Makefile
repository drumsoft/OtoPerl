MAKE = make
CP = cp

all: otoperld otoperld-start.pl

otoperld: otoperld-src/otoperld
	$(CP) $^ .

otoperld-start.pl: otoperld-src/otoperld-start.pl
	$(CP) $^ .

otoperld-src/otoperld: otoperld-src/*.h otoperld-src/*.c
	$(MAKE) -C otoperld-src

clean: 
	rm otoperld otoperld-start.pl
	$(MAKE) clean -C otoperld-src

run: all
	./otoperld
