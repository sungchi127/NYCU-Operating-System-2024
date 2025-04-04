.PHONY: clean stat

all: compile test

compile: sched_demo_313560005.c
	gcc sched_demo_313560005.c -lpthread  -o sched_demo_313560005

run:
	sudo ./sched_demo -n 3 -t 1.0 -s NORMAL,FIFO,FIFO -p -1,10,30   		
	sudo ./sched_demo -n 4 -t 0.5 -s NORMAL,FIFO,NORMAL,FIFO -p -1,10,-1,30  

test: 
	sudo ./sched_test.sh ./sched_demo ./sched_demo_313560005

clean:
	rm -f sched_demo_313560005 
