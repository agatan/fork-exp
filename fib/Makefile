ifeq ($(NOPIE),1)
	LDFLAGS +=
else
	LDFLAGS += -fPIE -fpie
endif

.PHONY: all
all: main child

main: main.o
	$(CC) $(LDFLAGS) $< -o $@

child: child.o
	$(CC) $(LDFLAGS) $< -o $@

.c.o:
	$(CC) $(LDFLAGS) $< -c

clean:
	$(RM) main *.o
