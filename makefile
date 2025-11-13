CC = gcc
CFLAGS = -Wall -O2

all: process_sum

process_sum: main.c
	$(CC) $(CFLAGS) -o process_sum main.c

run:
	@if [ -z "$(word 2, $(MAKECMDGOALS))" ]; then \
  		echo "Usage: make run <N>"; \
  	else \
  		./process_sum $(word 2, $(MAKECMDGOALS)); \
	fi


%:#this avoids the error: "no rule to make target 'N'
	@:

clean:
	rm -f process_sum