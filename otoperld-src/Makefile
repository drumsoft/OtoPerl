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

clean: 
	rm $(OBJS)
	rm otoperld

