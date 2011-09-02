tfig: tfig.c
	cc tfig.c -std=c99 -o tfig -lncurses -lm

run: tfig
	seq2 -s 1 -e 5 | awk '{print $$1, $$1*$$1}' | ./tfig -x 0 -X 5 -y 0 -Y 5 -h 5 -w 5 -s ?

clean:
	rm tfig
