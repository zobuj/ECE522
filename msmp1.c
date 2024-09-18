#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <assert.h>

// Helpful MACROS
#define ITER   10
#define MAX_N  64*1024*1024 
#define MB     (1024*1024)
#define KB     1024

// Actual Cache Statistics
#define L1_CACHE_LINE_SIZE 64.0
#define L1_CACHE_SIZE (48.0 * KB)
#define L1_CACHE_ASSOC 12.0

#define L2_CACHE_LINE_SIZE 64.0
#define L2_CACHE_SIZE (1280.0 * KB)
#define L2_CACHE_ASSOC 10.0 

#define LLC_CACHE_LINE_SIZE 64.0
#define LLC_CACHE_SIZE (24.0 * MB)
#define LLC_CACHE_ASSOC 12.0

// LLC Parameters assumed
#define START_SIZE 1*MB
#define STOP_SIZE  24*MB

// Test Flags
#define DUMMY_TEST 0
#define CACHE_LINE_TEST 0
#define CACHE_SIZE_TEST 0
#define MEMORY_TIMING_TEST 0
#define CACHE_ASSOC_TEST 1

// Cache Size Test Flag
#define LOG_SLOWDOWN 0

// Enable Assertions
#define ENABLE_ASSERTIONS 1

char array[MAX_N];
char garbage_array[MAX_N];


/////////////////////////////////////////////////////////
// Provides elapsed Time between t1 and t2 in milli sec
/////////////////////////////////////////////////////////

double elapsedTime(struct timeval t1, struct timeval t2){
  double delta;
  delta = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  delta += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  return delta; 
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

double DummyTest(void)
{    
  struct timeval t1, t2;
  int ii, iterid;

  // start timer
  gettimeofday(&t1, NULL);

  for(iterid=0;iterid<ITER;iterid++){
    for(ii=0; ii< MAX_N; ii++){
      array[ii] += rand();
    }
  }

  // stop timer
  gettimeofday(&t2, NULL);
 
  return elapsedTime(t1,t2);
}



/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

// Helper function to load data into all the caches
void populate_caches_with_garbage_data(){
  for(int i=0;i<MAX_N;i++){
    garbage_array[i]++;
  }
}

double LineSizeTest(void)
{    
  double retval; // Return Value
  struct timeval t1, t2; // Time Structures
  
  int stride;
  int i;
  int iter;

  int strides[] = {1, 2, 4, 8, 16, 32, 64, 128, 256};
  double stride_times[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  for(iter = 0; iter < ITER; iter++){ 
      for(stride = 0; stride < 9; stride++){
        populate_caches_with_garbage_data(); // Fill caches with garbage    
        gettimeofday(&t1, NULL);
        for(i = 0; i < MAX_N; i+=strides[stride]){
            array[i]++;
        }
        gettimeofday(&t2, NULL);
        stride_times[stride] += elapsedTime(t1,t2);  
      }  
  }

  for(stride = 0; stride < 9; stride++){
    stride_times[stride] /= ITER;
    printf("Stride: %d, Access Time: %lf ms\n", strides[stride], stride_times[stride]);
  }
  
  for(stride = 0; stride < 8; stride++){
    if(stride_times[stride] - stride_times[stride+1] < 1){
        retval = (double)strides[stride+1];
    }
  }
  
  return retval; 
}


/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double CacheSizeTest(void)
{    
  double retval;
  struct timeval t1, t2;
  int i;
  int iter;
  int cache;  
  int num_caches = 13;
  int cache_sizes[] = {16 * KB, 32 * KB, 64 * KB, 128 * KB, 256 * KB, 512 * KB,
                       1024 * KB, 2048 * KB, 4096 * KB, 8192 * KB, 16384 * KB,
                       32768 * KB, 65536 * KB};  
  double cache_times[] = {0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0};  

  int cache_line_size = LLC_CACHE_LINE_SIZE; // We know that the cacheline size is 64 Bytes from previous tests
        
  for(iter = 0; iter < ITER; iter++){
      for(cache = 0; cache < num_caches; cache++){
        
        // Load Caches with Garbage
        populate_caches_with_garbage_data();   
        
        // Load potential cache size by reading in one cache line in at a time
        for(i = 0; i < cache_sizes[cache]; i+=64){
            array[i]++;
        }
        
        gettimeofday(&t1, NULL);
        // Measure time to access data
        for(i = 0; i < cache_sizes[cache]; i+=64){
            array[i]--;
        }
        gettimeofday(&t2, NULL);
        
        cache_times[cache]+=elapsedTime(t1,t2);
      }
  }
  
  for(cache = 0; cache < num_caches; cache++){
    cache_times[cache] /= ITER;
    printf("Cache Size: %d KB, Access Time: %lf\n", cache_sizes[cache] / KB, cache_times[cache]);
  } 

  double prev_slowdown = 0;    
  double curr_slowdown;  
  for(cache = 1; cache < num_caches; cache++){
    curr_slowdown = cache_times[cache] / cache_times[cache-1];
    #if LOG_SLOWDOWN
    printf("Slowdown from %d KB to %d KB: %lf\nPrev Slowdown: %lf\nCurr Slowdown: %lf\n", cache_sizes[cache-1] / KB, cache_sizes[cache] / KB, cache_times[cache]/cache_times[cache-1],
    prev_slowdown, curr_slowdown);
    #endif
    if(curr_slowdown > prev_slowdown){
        retval = cache_sizes[cache-1] / KB;
    }
    prev_slowdown = curr_slowdown;
    
  } 
 
  return retval; 
}




/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double MemoryTimingTest(void)
{    
  double retval;
  struct timeval t1, t2;
  int i;
  
  // Populate the last level cache 
  for(i = 0; i < LLC_CACHE_SIZE; i++){
    array[i]++;
  }

  // Access a point in memory not within the last level cache
  // This will cause a LLC Miss and will need to go to memory to retrieve the data
  // This then means that the time to access this element will be the memory latency
  gettimeofday(&t1, NULL);
  array[MAX_N-1]++;
  gettimeofday(&t2, NULL);

  retval = elapsedTime(t1,t2);

  return retval; 
}


/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double CacheAssocTest(void)
{    
  double retval;
  struct timeval t1, t2;

  // L1_NUMBER_OF_SET_BITS = log2(64) = 6
  // L1_NUMBER_OF_OFFSET_BITS = log(64) = 6
  // (52 Tag Bits) | (6 Set Bits) | (6 Offset Bits)

  // L2_NUMBER_OF_SET_BITS = log2(2048) = 11
  // L2_NUMBER_OF_OFFSET_BITS = log(64) = 6
  // (47 Tag Bits) | (11 Set Bits) | (6 Offset Bits)

  // LLC_NUMBER_OF_SET_BITS = log2(32768) = 15
  // LLC_NUMBER_OF_OFFSET_BITS = log2(64) = 6
  // (43 Tag Bits) | (15 Set Bits) | (6 Offset Bits)

  double associativities[37];

  int assoc_i;
  for(assoc_i = 0; assoc_i < 37; assoc_i++){
    associativities[assoc_i] = 0.0;
  }

  int i;
  int assoc;
  int iter;

  for(iter = 0;iter < ITER; iter++){
      for(assoc = 0; assoc <= 36; assoc++){
          populate_caches_with_garbage_data();
          // Populate set 0 of the L1 cache
          for(i = 0; i <= assoc; i++){
             array[2*i << 12]++;
          }
          // If the assoc we are testing is less than the actual associativity
          // Then we will see fast access times. On the other hand, a higher associativity
          // would result in slower access times. 
          gettimeofday(&t1, NULL);  
          for(i = 0; i <= assoc; i++){
            array[2*i << 12]--;
          }
          gettimeofday(&t2, NULL);
          
          associativities[assoc] += elapsedTime(t1,t2);
      }
  }

  double max_assoc_time = 0;
 
  for(assoc = 0; assoc < 37; assoc++){
    associativities[assoc] /= ITER; 
    printf("Associativity: %d, Access Time: %lf ms\n", assoc, associativities[assoc]);
    if(associativities[assoc] > max_assoc_time) {
        max_assoc_time = associativities[assoc];
        retval = assoc;
    }
 } 

  return retval; 
}



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int main(){
  
#if DUMMY_TEST
  printf("Starting Test:\n");
  printf("Test took %lf seconds\n", DummyTest()/1000.0);
#endif

#if CACHE_LINE_TEST
  printf("Starting Cache Line Test...\n");
  printf("Actual LLC Cache Line Size: %lf\n", LLC_CACHE_LINE_SIZE);
  double cache_line_size = LineSizeTest();
  printf("\nLast Level Cache Line Size: %lf\n", cache_line_size);
 #if ENABLE_ASSERTIONS
     assert(cache_line_size == LLC_CACHE_LINE_SIZE);
 #endif
#endif

#if CACHE_SIZE_TEST
  printf("Starting Cache Size Test...\n");
  printf("Actual L1 Cache Size: %lf KB\nActual L2 Cache Size: %lf KB\nActual LLC Cache Size: %lf KB\n", L1_CACHE_SIZE / KB, L2_CACHE_SIZE / KB, LLC_CACHE_SIZE / KB);
  double cache_size = CacheSizeTest();
  printf("\nLast Level Cache Size: Greater than %lf KB and less than %lf KB\n", cache_size, cache_size * 2);
  #if ENABLE_ASSERTIONS
    assert((LLC_CACHE_SIZE / KB) > cache_size && (LLC_CACHE_SIZE / KB) < cache_size * 2); 
  #endif
#endif

#if MEMORY_TIMING_TEST
  printf("Starting Memory Timing Test...\n");
  double memory_access_latency = MemoryTimingTest();
  printf("\nMemory Access Latency: %lf ms\n", memory_access_latency); 
#endif 

#if CACHE_ASSOC_TEST
  printf("Starting Cache Associativity Test...\n");
  printf("Actual LLC Associativity: %lf\n", LLC_CACHE_ASSOC);
  double cache_associativity = CacheAssocTest();
  printf("\nLast Level Cache Associativity: %lf\n", cache_associativity);
  #if ENABLE_ASSERTIONS
    assert(cache_associativity == LLC_CACHE_ASSOC);
  #endif
#endif

  printf("End of Tests.\n");

}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
