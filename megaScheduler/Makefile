all:	test_sched advtest_sched multitest_sched embedded_sched coop_sched super_sched mega_sched

test_sched:	test_sched.c
	gcc test_sched.c -o test_sched

advtest_sched:	advtest_sched.c
	gcc advtest_sched.c -o advtest_sched

multitest_sched:	multitest_sched.c
	gcc multitest_sched.c -o multitest_sched

embedded_sched:	embedded_sched.c
	gcc embedded_sched.c -o embedded_sched

coop_sched:	coop_sched.c
	gcc coop_sched.c -o coop_sched

super_sched:	super_sched.c
	gcc super_sched.c -o super_sched

mega_sched:	mega_sched.c
	gcc mega_sched.c -o mega_sched

clean:
	rm -rf *.o test_sched advtest_sched multitest_sched embedded_sched coop_sched super_sched mega_sched
