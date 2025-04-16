all:	test_hc advtest_hc multitest_hc embedded_hc coop_hc super_hc mega_hc

test_hc:	test_hc.c
	gcc test_hc.c -o test_hc

advtest_hc:	advtest_hc.c
	gcc advtest_hc.c -o advtest_hc

multitest_hc:	multitest_hc.c
	gcc multitest_hc.c -o multitest_hc

embedded_hc:	embedded_hc.c
	gcc embedded_hc.c -o embedded_hc

coop_hc:	coop_hc.c
	gcc coop_hc.c -o coop_hc

super_hc:	super_hc.c
	gcc super_hc.c -o super_hc

mega_hc:	mega_hc.c
	gcc mega_hc.c -o mega_hc

clean:
	rm -rf *.o test_hc advtest_hc multitest_hc embedded_hc coop_hc super_hc mega_hc
