CC = gcc
CFLAGS = -O3 -march=native -Wall -Wextra

ordenacao: ordenacao.c
	$(CC) $(CFLAGS) -o ordenacao ordenacao.c

clean:
	rm -f ordenacao
