all: mtudetect

CFLAGS += -Wall -O2
RM="rm"

mtudetect: mtudetect.h mtudetect.c main.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -fPIC -DPERFORMANCE -D_GNU_SOURCE -o $@ $^

clean:
	$(RM) *.o > /dev/null 2>&1
	$(RM) mtudetect
