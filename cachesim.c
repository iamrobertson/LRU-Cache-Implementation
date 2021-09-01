#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Log base 2 function
int log2(int n){
    int r=0;
    while(n>>=1) r++;
    return r;
}
// Simulated memory
char sim_memory[(1<<24)];

// Cache data structure
// Each way is represented as a struct
typedef struct cache_way
{
    int tag;
    char data[1024];
    int age;
    int valid;
} cache_way;

// Helper functions
bool in_cache(int curr_tag, int set, int assoc, int now, cache_way **setarrayptr){
   // Search cache
   for (int i = 0; i<assoc; i++){
        if(((setarrayptr[set][i].valid)==1) && ((setarrayptr[set][i].tag)==curr_tag)){
            // Update age
            setarrayptr[set][i].age = now;
            return true; 
        }
   }
    return false;
}

int choose_victim(int set, int assoc, cache_way **setarrayptr){
    int min_age = 2147483647;
    int min_tag;
    for (int c = 0; c<assoc; c++){
        if((setarrayptr[set][c].age)<min_age){
            min_age = setarrayptr[set][c].age;
            min_tag = setarrayptr[set][c].tag;
        }
    }
    return min_tag;
}

void add_to_cache(int curr_tag, int set, int blocksize, int newaddr, int assoc, int now, cache_way **setarrayptr){
    char dat_amount[blocksize]; 
    int address_buf = newaddr;
    // Grab data from sim_memory
    strncpy(dat_amount, (sim_memory + address_buf), blocksize);

    // Check if there is any space available in cache set
    for (int p = 0; p<assoc; p++){
        if((setarrayptr[set][p].valid)==0){ // if space is available
            setarrayptr[set][p].tag = curr_tag;
            strcpy(setarrayptr[set][p].data, dat_amount);
            setarrayptr[set][p].age = now;
            setarrayptr[set][p].valid = 1;
            return;
        }
    }
    // Search for replacement if no space is available in cache set
    int remtag = choose_victim(set, assoc, setarrayptr);
    for (int f = 0; f<assoc; f++){
        if((setarrayptr[set][f].tag)==remtag){
            setarrayptr[set][f].tag = curr_tag;
            strcpy(setarrayptr[set][f].data, dat_amount);
            setarrayptr[set][f].age = now;
            return;
        }
    }
}

int main(int argc, char* argv[]){
    if(argc < 5){
       printf("Usage: cachesim <trace-file> <cache-size-kB> <associativity> <block-size>\n");
       return 0;
    }     
    else if(argc > 5){
        printf("Usage: cachesim <trace-file> <cache-size-kB> <associativity> <block-size>\n");
        return 0;
    }
    else{
        // Declare local variables
        int cache_size_buf;
        int cache_size;
        int cache_exp;
        int assoc;
        int blocksize;
        int b_off_bits;
        int b_off;
        int b_off_mask;
        int num_frames;
        int num_sets;
        int set_bits;
        int index;
        int index_mask;
        int tag_bits;
        int curr_tag;
        char store[6] = "store";
        char load[5] = "load";
        int now = 0;
        bool check_cache;
        char hit[4] = "hit";
        char miss[5] = "miss";

        // Read argvs
        // Read File
        FILE *fpointer;
        fpointer = fopen(argv[1], "r");
        // Scan cache-size
        sscanf(argv[2], "%d", &cache_size_buf);
        // Scan associativity
        sscanf(argv[3], "%d", &assoc);
        // Scan block size
        sscanf(argv[4], "%d", &blocksize);

        // Figure out nature of cache and address bit manipulation
        cache_exp = (log2(cache_size_buf)+10);
        cache_size = (1 << cache_exp);
        b_off_bits = log2(blocksize);
        num_frames = 1 << (cache_exp-b_off_bits);
        num_sets = (num_frames / assoc);
        set_bits = log2(num_sets);
        tag_bits = 24 - (b_off_bits + set_bits);
    
        // Allocate cache data structure
        // Allocate array of pointers(sets)
        cache_way **setarrayptr;
        setarrayptr = (cache_way**)malloc(num_sets * sizeof(cache_way*)); // array of num_sets pointers
        // Declare num of ways in each set
        for (int k = 0; k<num_sets; k++){
            // cache_way way[assoc]; // Declare struct array of ways
            setarrayptr[k] = (cache_way*)malloc(assoc * sizeof(cache_way)); // Assign pointer to struct array 
        }
        // Initialize values in cache
        for(int s = 0; s<num_sets; s++){
            for(int w = 0; w<assoc; w++){
                setarrayptr[s][w].tag = 0;
                setarrayptr[s][w].age = 0;
                setarrayptr[s][w].valid = 0;
            }
        }
        // Read through the trace file

        // Declare reading variables
        char action[10];
        int addr;
        int addr_buf;
        int size_op;
        char hit_or_miss[5];

        // Reading line by line
        while(fscanf(fpointer, "%s", action) != EOF){   // Verb is read here
            
            // Check if verb is store or load
            if((strcmp(action, store)) == 0){
                // Scan the rest of the line
                fscanf(fpointer, "%x", &addr_buf);
                fscanf(fpointer, "%d", &size_op);
                // Split address into corresponding bits for search in cache
                addr = addr_buf;
                // Isolate b_off
                b_off_mask = (1 << b_off_bits) - 1;
                b_off = addr_buf & b_off_mask;
                // Isolate index
                index_mask = (1 << set_bits) - 1;
                index = (addr_buf >> b_off_bits) & index_mask;
                // Isolate tag
                curr_tag = (addr_buf >> (set_bits + b_off_bits));
                // Check if hit or miss
                check_cache = in_cache(curr_tag, index, assoc, now, setarrayptr);
                if(check_cache == false){
                    strcpy(hit_or_miss, miss);
                }
                else{
                    strcpy(hit_or_miss, hit);
                }
                for(int m = 0; m<size_op; m++){
                    // Read each byte and store it in addressed simulated memory slot
                    fscanf(fpointer, "%2hhx", (sim_memory+addr_buf));
                    addr_buf++;
                }
                // Print status
                printf("store 0x%x %s\n", addr, hit_or_miss);
            }
            // load
            else{ 
                // Scan rest of the line
                fscanf(fpointer, "%x", &addr_buf);
                fscanf(fpointer, "%d", &size_op);
                char ret[size_op];
                // Split address into corresponding bits for search in cache
                addr = addr_buf;
                // Isolate b_off
                b_off_mask = (1 << b_off_bits) - 1;
                b_off = addr_buf & b_off_mask;
                // Isolate index
                index_mask = (1 << set_bits) - 1;
                index = (addr_buf >> b_off_bits) & index_mask;
                // Isolate tag
                curr_tag = (addr_buf >> (set_bits + b_off_bits));
                check_cache = in_cache(curr_tag, index, assoc, now, setarrayptr);
                if(check_cache==true){
                    strcpy(hit_or_miss, hit);
                }
                else{
                    strcpy(hit_or_miss, miss);
                    // Make new address to grab block into cache from sim memory
                    int newaddr;
                    newaddr = (addr >> b_off_bits) << b_off_bits;
                    // Add to cache
                    add_to_cache(curr_tag, index, blocksize, newaddr, assoc, now, setarrayptr);
                }
                // Print first part of output
                printf("load 0x%x %s ", addr, hit_or_miss);
                // Print data from sim memory
                for(int v=0; v<size_op; v++){
                    printf("%02hhx", *(sim_memory+addr_buf));
                    addr_buf++;
                }
                printf("\n");
            }
            now++; 
        }
        fclose(fpointer);
        for(int d = 0; d<num_sets; d++){
            free(setarrayptr[d]);
        }
        free(setarrayptr);
        return EXIT_SUCCESS;
    }
}
