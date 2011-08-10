CC = $(shell perl -MConfig -e "print \$$Config{cc}")
PERLCCFLAGS = $(shell perl -MExtUtils::Embed -e ccopts)
PERLLDFLAGS = $(shell perl -MExtUtils::Embed -e ldopts)

FRAMEWORKS = -framework CoreServices -framework CoreAudio -framework AudioUnit

CFLAGS = -g -Wall
OBJS = audiounit.o codeserver.o otoperld.o main.o

otoperld: $(OBJS)
	$(CC) $(PERLLDFLAGS) $(FRAMEWORKS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) $(PERLCCFLAGS) -c $<

.d.o:

.c.d:
    $(SHELL) -ec '$(CC) -MM $(CFLAGS) $(PERLCCFLAGS) -c $< \
                  | sed '\"s/\($*\)\.o[ :]*/\1.o $@ : /g'\" > $@; \
                  [ -s $@ ] || rm -f $@'

clean: 
	rm $(OBJS)
	rm $(OBJS:.o=.d)
	rm otoperld

include $(OBJS:.o=.d)
