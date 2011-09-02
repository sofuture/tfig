tfig: tfig.c ll.c
	cc tfig.c ll.c -std=c99 -o tfig -lncurses -lm

run: tfig
	seq2 -s 1 -e 5 | awk '{print $$1, $$1*$$1}' | ./tfig -x 0 -X 5 -y 0 -Y 5 -h 20 -w 20 -s ?

clean:
	rm tfig
