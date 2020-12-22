CFLAGS=-Wall -Werror -std=gnu99

SRC=src
OUT=target

wdir:
	mkdir -p $(OUT)
	cc $(CFLAGS) $(SRC)/wdir.c -o $(OUT)/wdir

.PHONY: clean

clean:
	rm -rf $(OUT)/*
