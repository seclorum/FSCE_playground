trafficlight:	trafficlight.c
	#gcc -DMAKEBIN trafficlight.c -o trafficlight
	gcc trafficlight.c -o trafficlight

clean:
	rm -rf trafficlight *.o *.c~ *.h~

