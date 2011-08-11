# otoperld

#CFLAGS = -g -Wall
# -arch x86_64 -arch i386 -arch ppc 
CFLAGS = -O3 -Wall 

# perl compile environment
# PERL = /opt/local/bin/perl # for MacPorts perl
PERL = /usr/bin/perl # for Mac OS X perl
CC = $(shell $(PERL) -MConfig -e "print \$$Config{cc}")
PERLCCFLAGS = $(shell $(PERL) -MExtUtils::Embed -e ccopts | sed -e "s/-arch  *[^ ]*//g")
PERLLDFLAGS = $(shell $(PERL) -MExtUtils::Embed -e ldopts | sed -e "s/-arch  *[^ ]*//g")

# Mac OS X frameworks
FRAMEWORKS = -framework CoreServices -framework CoreAudio -framework AudioUnit

# otoperld
OBJS = audiounit.o codeserver.o otoperld.o main.o


all: otoperld

otoperld: $(OBJS)
	$(CC) $(PERLLDFLAGS) $(FRAMEWORKS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) $(PERLCCFLAGS) -c $<

%.o: %.d

%.d: %.c
	 @set -e; rm -f $@; \
	  $(CC) -MM $(CFLAGS) $(PERLCCFLAGS) -c $< > $@.$$$$; \
	  sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	  rm -f $@.$$$$

clean: 
	rm $(OBJS)
	rm $(OBJS:.o=.d)
	rm *.d.[0-9][0-9][0-9][0-9][0-9]
	rm otoperld

include $(OBJS:.o=.d)
