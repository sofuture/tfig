# GNU make only :argh:
#

OS := $(shell uname -s | tr "[:upper:]" "[:lower:]")

tfig: tfig.c ll.c
	cc tfig.c ll.c -std=c99 -o tfig -lm

run: tfig
ifeq (freebsd, $(OS))
	seq2 -s 1 -e 40 | awk '{print $$1, $$1/2}' | ./tfig -x 10 -X 30 -y 10 -Y 30 -h 20 -w 40 -s ?
	seq2 -s 1 -e 5 | awk '{print $$1, $$1**$$1}' | ./tfig -x 0 -X 10 -y 0 -Y 10 -h 10 -w 10 -s ?
	seq2 -s 1 -e 20 | awk '{print $$1, $$1}' | ./tfig -x 0 -X 30 -y 0 -Y 30 -h 20 -w 40 -s ?
	seq2 -s 1 -e 40 | awk '{print $$1, $$1*2}' | ./tfig -x 0 -X 30 -y 0 -Y 30 -h 20 -w 40 -s ?
	seq2 -s 1 -e 11 | ./tfig -x 0 -X 10 -y 0 -Y 10 -h 2 -w 2 -s ?
else
	seq 1 40 | awk '{print $$1, $$1/2}' | ./tfig -x 10 -X 30 -y 10 -Y 30 -h 20 -w 40 -s ?
	seq 1 5 | awk '{print $$1, $$1**$$1}' | ./tfig -x 0 -X 10 -y 0 -Y 10 -h 10 -w 10 -s ?
	seq 1 20 | awk '{print $$1, $$1}' | ./tfig -x 0 -X 30 -y 0 -Y 30 -h 20 -w 40 -s ?
	seq 1 40 | awk '{print $$1, $$1*2}' | ./tfig -x 0 -X 30 -y 0 -Y 30 -h 20 -w 40 -s ?
	seq 1 11 | ./tfig -x 0 -X 10 -y 0 -Y 10 -h 2 -w 2 -s ?
endif

clean:
	rm tfig
