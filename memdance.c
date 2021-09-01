#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

// memdance.c by Tyler Bletsch (Tyler.Bletsch@duke.edu) for ECE250
// Benchmark the rate of basically random memory access to a block of memory for a given amount of a time

const int ACCESSES_PER_STEP = 1000000; // needs to be big enough that most of the time is spent hitting memory rather than computing time of day
const int IDX_DELTA = 60029; // a fairly big prime so it won't wrap neatly for any common work size

// settings to use in default mode
const size_t DEFAULT_SIZES[] = {1*1024*1024, 2*1024*1024, 4*1024*1024, 8*1024*1024, 16*1024*1024, 32*1024*1024, 64*1024*1024, 128*1024*1024};
const int NUM_DEFAULT_SIZES = sizeof(DEFAULT_SIZES)/sizeof(*DEFAULT_SIZES);

const int DEFAULT_EXECUTION_TIME = 3;

// get current time as a double
double get_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + tv.tv_usec/1000000.0;;
}

// run one test with the buffer size and runtime chosen; print resulting stats
void run_test(size_t buffer_size, int execution_time) {
    int steps = 0; // our counter

    double time_start = get_time();
    char* buf = (char*) malloc(buffer_size);
    int idx = 0;
    
    // outer loop: keep going until we're out of time
    while (get_time() < time_start+execution_time) {
        // inner loop: do a bunch of accesses as fast as possible
        // this inner loop is here so the overhead of determining the time doesn't significantly affect our measurement
        for (int s=0; s<ACCESSES_PER_STEP; s++) {
            buf[idx]++;
            idx = (idx+IDX_DELTA) % buffer_size; // a really cheap form of "random", just advance by a fairly large prime number
        }
        steps++;
    }
    double elapsed = get_time() - time_start;
    double rate = (double)ACCESSES_PER_STEP*steps/elapsed;
    printf("Buffer size %6zd kB. %3d*%d accesses in %.1f seconds, %.3f MB/s.\n",buffer_size/1024,steps,ACCESSES_PER_STEP,elapsed,rate/1024/1024);
    
    
}

int main(int argc, char* argv[]) {
    size_t buffer_size=0; // bytes, given as argument
    int execution_time; // seconds, given as argument
    if (argc == 2 && strcmp(argv[1],"default")==0) {
        for (int i=0; i<NUM_DEFAULT_SIZES; i++) {
            run_test(DEFAULT_SIZES[i], DEFAULT_EXECUTION_TIME);
        }
    } else if (argc == 3 && sscanf(argv[1],"%zd",&buffer_size)==1 && sscanf(argv[2],"%d",&execution_time)==1) {
        run_test(buffer_size, execution_time);
    } else {
        printf("Benchmark the rate of basically random memory access to a block of memory for a given amount of a time.\n\n");
        printf("Run the default suite:  %s default\n",argv[0]);
        printf("Run a specific test:    %s <size-in-bytes> <execution-time>\n",argv[0]);
        return 1;
    }
    
    
}
