first: all

LIBNAME = libbigint

#-----------

CFLAGS += -Wall -Wextra -Wpedantic -Wshadow
CFLAGS += -g
# CFLAGS += -DDEBUG

LIBS = -lm

C_SRCS = \
	bigint.c \
	timer.c \

TESTS = \
	t/001_assign \
	t/010_add \
	t/020_mul \
	t/030_factorial \

PERFS = \
	p/mul \
	p/factorial \

C_OBJS = $(C_SRCS:.c=.o)

LIBNAME_AR = $(LIBNAME).a
LIBNAME_SO = $(LIBNAME).so

%.o: %.c
	cc $(CFLAGS) -c -o$@ $^

#
# LIBRARY
#
$(LIBNAME_AR): $(C_OBJS)
	ar cr $@ $^

define make-exe-target
  $1: $1.c $(LIBNAME_AR)
	cc $(CFLAGS) -I. -o$1 $1.c -L. -lbigint $(LIBS)
  $2:: $1
endef

all: $(LIBNAME_AR)

#
# TESTS
#
$(foreach test,$(TESTS),$(eval $(call make-exe-target,$(test),test)))
test::
	@for x in $(TESTS); do ./$$x | awk '{print $$1}' | sort | uniq -c; done

#
# PERFORMANCE
#
$(foreach perf,$(PERFS),$(eval $(call make-exe-target,$(perf),perf)))
perf::
	@for x in $(PERFS); do ./$$x; done

#
# CLERICAL
#
clean:
	rm -f $(C_OBJS) t/*.o p/*.o
	rm -f $(LIBNAME_AR) $(LIBNAME_SO) $(TESTS) $(PERFS)
	rm -fr *.dSYM t/*.dSYM p/*.dSYM
