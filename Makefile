all:	voice_tracker	trafficlight

clean:
	rm -rf trafficlight *.o *.c~ *.h~
	rm -rf voice_tracker *.?~
 
trafficlight:	trafficlight.c
	#gcc -DMAKEBIN trafficlight.c -o trafficlight
	gcc trafficlight.c -o trafficlight

voice_tracker:	voice_tracker.c
	gcc voice_tracker.c -o voice_tracker

voice_tracker.test:	voice_tracker
	./voice_tracker | sort

voice_tracker.lua.test:	voice_tracker.lua
	luajit voice_tracker.lua | sort


