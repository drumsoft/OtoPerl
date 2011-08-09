CC = $(shell perl -MConfig -e "print \$$Config{cc}")
PERLCCFLAGS = $(shell perl -MExtUtils::Embed -e ccopts)
PERLLDFLAGS = $(shell perl -MExtUtils::Embed -e ldopts)

FRAMEWORKS = -framework CoreServices -framework CoreAudio -framework AudioUnit

CFLAGS = -g -Wall
OBJS = c-otoperld.o

c-otoperld: $(OBJS)
	$(CC) $(PERLLDFLAGS) $(FRAMEWORKS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) $(PERLCCFLAGS) -c $<

clean: 
	rm $(OBJS)

test: c-otoperld
	./c-otoperld 2
