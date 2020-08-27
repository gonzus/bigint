all: libbigint.a

#-----------

CFLAGS += -Wall -g
# CFLAGS += -DDEBUG

TESTS = \
	t/001_assign \
	t/010_add \
	t/011_sub \
	t/020_mul \
	t/030_factorial \

PERFS = \
	p/sub \
	p/mul \
	p/factorial \

#
# LIBRARY
#
bigint.o: bigint.c
	cc $(CFLAGS) -c -o$@ $^

timer.o: timer.c
	cc $(CFLAGS) -c -o$@ $^

libbigint.a: bigint.o timer.o
	ar cr $@ $^

#
# TESTS
#
t/001_assign: t/001_assign.c libbigint.a
	cc $(CFLAGS) -I. -o$@ t/001_assign.c -L. -lbigint -lm

t/010_add: t/010_add.c libbigint.a
	cc $(CFLAGS) -I. -o$@ t/010_add.c -L. -lbigint -lm

t/011_sub: t/011_sub.c libbigint.a
	cc $(CFLAGS) -I. -o$@ t/011_sub.c -L. -lbigint -lm

t/020_mul: t/020_mul.c libbigint.a
	cc $(CFLAGS) -I. -o$@ t/020_mul.c -L. -lbigint -lm

t/030_factorial: t/030_factorial.c libbigint.a
	cc $(CFLAGS) -I. -o$@ t/030_factorial.c -L. -lbigint -lm

test: $(TESTS)
	@for x in $(TESTS); do ./$$x | awk '{print $$1}' | sort | uniq -c; done

#
# PERFORMANCE
#
p/sub: p/sub.c libbigint.a
	cc $(CFLAGS) -I. -o$@ p/sub.c -L. -lbigint -lm

p/mul: p/mul.c libbigint.a
	cc $(CFLAGS) -I. -o$@ p/mul.c -L. -lbigint -lm

p/factorial: p/factorial.c libbigint.a
	cc $(CFLAGS) -I. -o$@ p/factorial.c -L. -lbigint -lm

perf: $(PERFS)
	@for x in $(PERFS); do ./$$x; done

#
# CLERICAL
#
clean:
	rm -f *.o t/*.o
	rm -f libbigint.a $(TESTS) $(PERFS)
	rm -fr *.dSYM t/*.dSYM p/*.dSYM
