# GNU make only :argh:
#

OS := $(shell uname -s | tr "[:upper:]" "[:lower:]")

tfig: tfig.c ll.c
	cc tfig.c ll.c -std=c99 -o tfig -lm

run: tfig
	awk 'BEGIN { for(i=1;i<=40;i++) {print (i, i/2)}}' | ./tfig -x 10 -X 30 -y 10 -Y 30 -h 20 -w 40 -s ?
	awk 'BEGIN { for(i=1;i<=5;i++) {print (i, i*i)}}' | ./tfig -x 0 -X 10 -y 0 -Y 10 -h 10 -w 10 -s ?
	awk 'BEGIN { for(i=1;i<=20;i++) {print (i, i)}}' | ./tfig -x 0 -X 30 -y 0 -Y 30 -h 20 -w 40 -s ?
	awk 'BEGIN { for(i=1;i<=40;i++) {print (i, i*2)}}' | ./tfig -x 0 -X 30 -y 0 -Y 30 -h 20 -w 40 -s ?
	awk 'BEGIN { for(i=1;i<=11;i++) {print (i)}}' | ./tfig -x 0 -X 30 -y 0 -Y 30 -h 2 -w 4 -s ?

clean:
	rm tfig
