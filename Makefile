OBJS=HashTables/lockfreetable.o HashTables/lockingtable.o HashTables/probetable.o Helpers/lamport_queue.o Helpers/lf_lists.o Helpers/slists.o hashtable_1.o Tests/performance_tests.o

UTIL_FILES=hashgenerator.c packetsource.c generators.c crc32.c hashpackettest.c hashpacketworker.c hashtable.c seriallist.c fingerprint.c stopwatch.c
UTIL_OBJS=$(UTIL_FILES:%.c=Utils/%.o)

TEST_FILES=main.c
TEST_EXES=$(TEST_FILES:%.c=Tests/%)

# PERF_FILES=packet.c
# PERF_OBJS=$(PERF_FILES:%.c=perf/%.o)

.PHONY: perf tests clean

CC=gcc
override CFLAGS += -O3 -Wall -Werror -std=gnu99 -IUtils
#override CFLAGS += -O0 -g -Wall -Werror -std=gnu99 -IUtils

LDLIBS=-pthread -lm

%.o: %.c %.h
	$(CC) -c $(CFLAGS) $(LDFLAGS) $(LDLIBS) $< -o $@

$(TEST_EXES): %: %.c $(UTIL_OBJS) $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(UTIL_OBJS) $(OBJS) $< -o $@ $(LDLIBS)

tests: $(TEST_EXES)

perf: $(PERF_OBJS) $(OBJS) $(UTIL_OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(PERF_OBJS) $(UTIL_OBJS) $(OBJS) -o tests $(LDLIBS)

clean:
	rm Tests/*.o HashTables/*.o Helpers/*.o Utils/*.o $(TEST_EXES)
