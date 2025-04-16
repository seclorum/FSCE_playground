all:	setup supersaw thxsnd wavegen

setup:
	mkdir -p bin/

supersaw:	supersaw.c
	gcc supersaw.c -o bin/supersaw

supersaw_chord:	supersaw_chord.c
	gcc supersaw_chord.c -o bin/supersaw_chord

supersaw_test:	supersaw
	./bin/supersaw | sox -t raw -r 8000 -e unsigned-integer -b 8 -c 1 - -d

supersaw_chord_test:	supersaw_chord
	./bin/supersaw_chord | sox -t raw -r 8000 -e unsigned-integer -b 8 -c 1 - -d

wavegen:	wavegen.c
	gcc wavegen.c -o bin/wavegen -lm

wavegen_test:	wavegen
	timeout 1 ./bin/wavegen sine | sox -t raw -r 8000 -e unsigned-integer -b 16 -c 1 - -d
	timeout 1 ./bin/wavegen square | sox -t raw -r 8000 -e unsigned-integer -b 8 -c 1 - -d
	timeout 1 ./bin/wavegen fm | sox -t raw -r 8000 -e unsigned-integer -b 16 -c 1 - -d
	timeout 1 ./bin/wavegen sample | sox -t raw -r 8000 -e unsigned-integer -b 16 -c 1 - -d
	timeout 1 ./bin/wavegen triangle | sox -t raw -r 8000 -e unsigned-integer -b 16 -c 1 - -d

thxsnd:	thxsnd.c
	gcc thxsnd.c -o bin/thxsnd -Wno-unsequenced

thxsnd_test:	thxsnd
	./bin/thxsnd | sox -t raw -r 8000 -e unsigned-integer -b 8 -c 1 - -d

clean:
	rm -rf bin/* *.?~
