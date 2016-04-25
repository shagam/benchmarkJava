#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <queue>

#include <sys/time.h>
#include <pthread.h>
#include <time.h>
#include <map>
#include "args.h"
#include <unordered_map>
#include <limits.h>

//#include <hash_map>

//* Jewel Store Copyright (C) 2011-2014  TechoPhil Ltd

using namespace std;

long getTimeMili () {
    timeval time;
    gettimeofday(&time, NULL);
    long millis = (time.tv_sec * 1000.0) + (time.tv_usec / 1000.0) + 0.5;
    return millis;
}

class Struc {
    long *m_longArray;
    char m_byte;
    char *m_str;
public:
    Struc () {
        int siz = (rand() % 10) * 4;
        m_longArray = new long [siz];
        m_byte = 0;
        m_str = 0;       
         //memset ((char*)a, 0, sizeof(a));
        //memset ((char*)b, 0, sizeof(b));
    }
    ~Struc () {
        delete [] m_longArray;
    }
    
};


const int ARRAY_SIZE = 100000;
const int THREAD_MAX = 100;
const int ARRAY_MAX = 100;

const int QUEUE_MAX = 1000;
    

static long  s_startMili = getTimeMili(); ;
static const char* s_testName = NULL;
static int s_threadCnt = 0;
static int s_arrayNum = s_threadCnt;
static int s_arraySize;
static int s_verbose = 0;
static int s_help = 0;
static int s_delay;

static long ** s_copyArrays;

static pthread_t               s_thread_id[THREAD_MAX];

const char * help ();
int getInt (char* th);

enum Test {
    t_alloc,
    t_tree,
    t_hash,
    t_treeNoLock,
    t_queue,
    t_copy,
    t_prime,
    t_count,
};

int s_test;

inline long InterlockedIncrement(long* p, int delta)
{
    return __atomic_add_fetch(p, delta, __ATOMIC_SEQ_CST);
}

static long s_grossLoops;

long seed = getTimeMili ();    
    
long random () {
      long x = seed;
      x ^= (x << 21);
      x ^= (x >> 35);
      x ^= (x << 4);
      seed = x;
      if (x < 0)
          x = 0 - x;
      return x;
} 

int formatLong (char * str, long n) {
    char * str_save = str;
    const int BILION = 1000000000; 
    const int MILION = 1000000; 
    const int KILO = 1000; 
    int fill = 0;

    if (n > BILION) {
        //str += n / BILION + ",";
        str += sprintf (str, "%ld,", n / BILION);
        n -= n / BILION * BILION;
        fill = 1;
    }
    if (n > MILION) {
        //str += n / MILION + ",";
        if (fill)
            str += sprintf (str, "%03ld,", n / MILION);
        else
            str += sprintf (str, "%ld,", n / MILION);
        n -= n / MILION * MILION;
        fill = 1;
    }        
    if (n > KILO) {
        //str += n / KILO + ",";
        if (fill)
            str += sprintf (str, "%03ld,", n / KILO);
        else
            str += sprintf (str, "%ld,", n / KILO);
        n -= n / KILO * KILO;
        fill = 1;            
    }
    if (fill)
        str += sprintf(str, "%03ld", n);
    else
        str += sprintf(str, "%3ld", n);

    return str - str_save;
}

class SafeTree {
    
    pthread_mutex_t   m_mutex;
    map <int, int> m_tree;

public:    
    SafeTree () {
        m_mutex = PTHREAD_MUTEX_INITIALIZER;        
        int stat = pthread_mutex_init (& m_mutex, NULL);        
    }
    void put (int key, int val) {

        if (s_threadCnt > 1) {
            int stat = pthread_mutex_lock (&m_mutex);
            if (stat)
                exit(-3);
        }
        
        m_tree.insert (make_pair(key, val));        
        if (s_threadCnt > 1) {
            int stat1 = pthread_mutex_unlock (&m_mutex);         
            if (stat1)
                exit(-4);
        }
    }
    
    int get (int key) {
        int val = -1;
        map<int,int>::iterator it;        
        if (s_threadCnt > 1) {
            int stat = pthread_mutex_lock (&m_mutex);
            if (stat)
                exit(-3);
        }
        it = m_tree.find(key);
        if (s_threadCnt > 1) {
            int stat1 = pthread_mutex_unlock (&m_mutex);         
            if (stat1)
                exit(-4);
        }
        if( it != m_tree.end())
            val = it->second;
        else
            val = INT_MAX;
        return val;
    }
    
    

    int getNoLock (int key) {
        int val = -1;
        map<int,int>::iterator it;        
        it = m_tree.find(key);
        if( it != m_tree.end())
            val = it->second;
        else
            val = INT_MAX;
        return val;
    }
};

class SafeHash {
    
    pthread_mutex_t   m_mutex;
    std::unordered_map <int, int> m_hash;

public:    
    SafeHash () {
        m_mutex = PTHREAD_MUTEX_INITIALIZER;        
        int stat = pthread_mutex_init (& m_mutex, NULL);        
    }
    void put (int key, int val) {
        std::unordered_map<int,int>::iterator it;
        int stat = pthread_mutex_lock (&m_mutex);
        if (stat)
            exit(-3);
        
        m_hash.insert (make_pair(key, val));
        
        int stat1 = pthread_mutex_unlock (&m_mutex);         
        if (stat1)
            exit(-4);
    }
    
    int get (int key) {
        int val = -1;
        std::unordered_map<int,int>::iterator it;        
        int stat = pthread_mutex_lock (&m_mutex);
        if (stat)
            exit(-3);
        
        it = m_hash.find(key);
        
        int stat1 = pthread_mutex_unlock (&m_mutex);         
        if (stat1)
            exit(-4);
        if( it != m_hash.end())
            val = it->second;
        else
            val = INT_MAX;
        return val;
    }   
};

class SafeArray {
    pthread_mutex_t   m_mutex;    
    Struc* s_array[ARRAY_SIZE];
public:
    SafeArray () {
        m_mutex = PTHREAD_MUTEX_INITIALIZER;        
        int stat = pthread_mutex_init (& m_mutex, NULL);
        memset (s_array, 0, sizeof (s_array));
    }
    
    void put (int indx, Struc* val) {
        int stat = pthread_mutex_lock (&m_mutex);
        if (stat)
            exit(-3);
        if (s_array[indx] != NULL)
            delete (s_array[indx]);
        s_array[indx] = val;
        stat = pthread_mutex_unlock (&m_mutex);
        if (stat)
            exit(-3);        
    }
    
    Struc* get (int indx) {
        int stat = pthread_mutex_lock (&m_mutex);
        if (stat)
            exit(-3);
        
        Struc *struc = s_array[indx];
        s_array[indx]= NULL;
        stat = pthread_mutex_unlock (&m_mutex);
        if (stat)
            exit(-3);
        return struc;        
    }   
};

class SafeQueue {
    pthread_mutex_t   m_mutex;    
    std::queue <Struc*> myQueue;
public:
    SafeQueue () {
        m_mutex = PTHREAD_MUTEX_INITIALIZER;        
        int stat = pthread_mutex_init (& m_mutex, NULL);
    }
    
    void enque (Struc* val) {
        int stat = pthread_mutex_lock (&m_mutex);
        if (stat)
            exit(-3);
        myQueue.push (val);
        stat = pthread_mutex_unlock (&m_mutex);
        if (stat)
            exit(-3);        
    }
    
    Struc* get () {
        int stat = pthread_mutex_lock (&m_mutex);
        if (stat)
            exit(-3);
        Struc* s = myQueue.front();
        myQueue.pop();
        stat = pthread_mutex_unlock (&m_mutex);
        if (stat)
            exit(-3);
        return s;
    }   
};

SafeTree *s_treeArray [ARRAY_MAX];

SafeHash *s_hashArray [ARRAY_MAX];

SafeArray *s_arr [ARRAY_MAX];

SafeQueue *s_queue = new SafeQueue(); 

int show (void) {
     int siz = fprintf (stderr, "\nbenchmark c: test=%s delay=%d threads=%d arrayNum=%d size=%d \n",
            s_testName, s_delay, s_threadCnt, s_arrayNum, s_arraySize);
     return siz;
}

 int countPrimes (int range) {		
       int i =0;
       int primeCount = 0;
       int num =0;

       for (i = 1; i <= range; i++) { 		  	  
          int counter=0; 	  
          for(num =i; num>=1; num--)  {
             if(i%num==0){
 		counter = counter + 1;
	     }
	  }
	  if (counter ==2)
	  {
	     //Appended the Prime number to the String
	     //primeNumbers = primeNumbers + i + " ";
             primeCount ++;
	  }	
       }
       return primeCount;
   }

 int randPositive () {
     int rnd = random();
     if (rnd < 0)
         rnd = 0 - rnd;
     return rnd;
 }
 
void *thread ( void *ptr ) {
  long m_printTime = getTimeMili ();
  size_t tmp = (size_t) ptr;
  int id = (int) tmp;
  int m_found = 0;
  int m_notFound = 0;
  
  for (long n = 1;  ; n++) {
      
    if ((n & 0xf) == 0) {
      InterlockedIncrement(& s_grossLoops, 0x10);
      if (getTimeMili () - s_startMili > s_delay)
          break;
    }
    if (s_test == t_tree) {
        int rand = randPositive ();
        int arr_num = rand % s_arrayNum;
        int key = rand % s_arraySize;
        
        int val = s_treeArray[arr_num]->get (key);
        if (val == INT_MAX)
            m_notFound ++;
        else
            m_found ++;
     }
     else if (s_test == t_treeNoLock) {
        int rand = randPositive ();         
        int key = rand % s_arraySize;
        
        int val = s_treeArray[id]->getNoLock (key);
        if (val == INT_MAX)
            m_notFound ++;
        else
            m_found ++;        
     }

     else if (s_test == t_hash) {
        int rand = randPositive ();        
        int arr_num = rand % s_arrayNum;
        int key = rand % s_arraySize;

        int val = s_hashArray[arr_num]->get (key);
        if (val == INT_MAX)
            m_notFound ++;
        else
            m_found ++;
     }
     else if (s_test == t_alloc){ // alloc
        int arr_indx;
        int arr_num;
        int rand = randPositive (); 
        arr_indx = rand % ARRAY_SIZE;
        arr_num = rand % s_arrayNum;

        Struc * struc = s_arr[arr_num]->get(arr_indx);
        if (struc != NULL)
            delete struc;

        struc = new Struc();
        s_arr[arr_num]->put(arr_indx, struc);  
     }
     else if (s_test == t_queue) {
         Struc* struc = s_queue->get();
         s_queue->enque(struc);
     }
     else if (s_test == t_copy) {
         if ((n % 2) == 0) // avoid optimize out
         memcpy (s_copyArrays[id], s_copyArrays[id + s_arrayNum], s_arraySize * sizeof (long));
         else
         memcpy (s_copyArrays[id + s_arrayNum], s_copyArrays[id], s_arraySize * sizeof (long));
/*
         for (int i = 0; i < s_arraySize; i++)
             s_copyArrays[id][i] = s_copyArrays[id + s_arrayNum][i];
*/
     }
     else if (s_test == t_prime) {
         int cnt = countPrimes (s_arraySize);
     }
     else {
         fprintf (stderr, "wrong state %s", help());
         exit (1);         
     }
  }
}

int main(int argc, char * argv[])  {
    //formatIntUtest ();
    if (argc > 1){
        int utest_flag = strcmp (argv[1], "utest");
        //fprintf (stderr,"\n %d %s \n", utest_flag, argv[1]);
        if (utest_flag == 0) {
            utest ();
            exit (0);
        }
    }
    srand ( time(NULL) );      /* initialize random seed: */
    
    s_threadCnt = getInteger ("thread", argc, argv, "concurrent thread count");
    if (s_threadCnt == -1)
        s_threadCnt = getCPUCount();
    
    if (s_threadCnt >= THREAD_MAX) {
        fprintf (stderr, "threadn should be smaller than %d  %s", THREAD_MAX, help());
        exit (1);               
    }
    
    s_arrayNum = getInteger ("array", argc, argv, "array count for threads access");
    if (s_arrayNum == -1)
        s_arrayNum = s_threadCnt;

    s_arraySize = getInteger ("size", argc, argv, "size of each array");
    if (s_arraySize == -1)
        s_arraySize = ARRAY_SIZE;

    if (getBool("verbose", argc, argv, "print test params during test run"))
        s_verbose = 1;    
    if (getBool("help", argc, argv, "print help"))
        s_help = 1;;
    
    s_delay = getInteger ("delay", argc, argv, "time limit of test ");
    if (s_delay == -1)
        s_delay = 5000;
            
    if (getBool("tree", argc, argv, "test==tree search")) {
        if (getBool("nolock", argc, argv, "each thread access private array")) {
            s_test = t_treeNoLock;
            s_testName = "tree_noLock";
            if (s_arrayNum < s_threadCnt)
                s_arrayNum = s_threadCnt;
        }
        else {
            s_test = t_tree;
            s_testName = "tree";
        }            
        for (int i= 0; i < s_arrayNum; i++) {
            s_treeArray[i] = new SafeTree();
            for (int n = 0; n < s_arraySize; n++) {
                int key = rand() % s_arraySize;
                s_treeArray[i]->put (key, key);
            }
         }
    }        

    if (getBool("alloc", argc, argv, "test==mem alloc")) {
        s_test = t_alloc;
        s_testName = "alloc";
        //memset ((char*) s_arr, 0, sizeof(s_arr));        
        for (int i= 0; i < s_arrayNum; i++) {
             s_arr[i] = new SafeArray();
        }
    }
    if (getBool("hash", argc, argv, "test==hash map search")) {
         s_test = t_hash;
         s_testName = "hash";
         for (int i= 0; i < s_arrayNum; i++) {         
             s_hashArray[i] = new SafeHash();
             for (int n = 0; n < s_arraySize/20; n++) {
                int key = rand() % s_arraySize;
                s_hashArray[i]->put (key, key);
            }
         }
    }
    if (getBool("queue", argc, argv, "test==queue insert")) {    
        s_test = t_queue;
        s_testName = "queue";
        for (int i= 0; i < s_arraySize; i++) {         
            Struc* struc = new Struc();
            s_queue->enque (struc);
        }
    }
    if (getBool("copy", argc, argv, "test==memory copy")) {    
        s_test = t_copy;
        s_testName = "copy";
        s_copyArrays = new long* [s_arrayNum * 2];
        for(int i = 0; i < s_arrayNum * 2; ++i)
            s_copyArrays[i] = new long [s_arraySize];
    }
    if (getBool("prime", argc, argv, "test==prime  compute intensive")) {    
        s_test = t_prime;
        s_testName = "prime";
        if (s_arraySize == -1 || s_arraySize == ARRAY_SIZE)
            s_arraySize = 500;
    }
    if (getBool("count", argc, argv, "test==count  many threads increment a common counter")) {
        s_test = t_count;
        s_testName = "count";
    }
    
    if (s_testName == NULL) {
        args_report();
        fprintf (stderr, "first param need: tree, alloc, hash, queue, copy, or prime  \n");
        exit (2);                  
    }
    
    show();
    if (s_help)
        args_report();
    
    for (int n = 0; n < s_threadCnt; n++) {
         size_t tmp = n;
        int stat = pthread_create( &s_thread_id[n], NULL, thread, (void*) tmp);
        if (stat) {
            exit(-2);
        }
    }
         
    for (int n = 0; n < s_threadCnt; n++) {
        pthread_join( s_thread_id[n], NULL);
    }

    char txt [200];
    int ptr = 0;
    ptr += sprintf (txt, "%s", "\nApplication exit grossLoops= ");
    ptr += formatLong (txt + ptr, s_grossLoops);
    if (s_test == t_copy) {
        long delay = getTimeMili () - s_startMili;
        long bw = s_grossLoops * 2 * s_arraySize * sizeof (long) / (delay / 1000);
        ptr += sprintf (txt + ptr, "    grossMBPerSec=");        
        ptr += formatLong (txt + ptr, bw/1000000);
    }
    printf ("%s\n\n", txt);
     
  return 0;
}

const char * help () {
    const char* str ="missing or wrong args, see examples:"
    "\n  ./benchmark  tree [threads=8] [array=8] [size=100]               tree  "
    "\n  ./benchmark  tree nolock  [threads=8] [array=8] [size=100]       tree no lock"
 //   "\n  ./overhead  h  4        hash 4 thread"
    "\n  ./benchmark  alloc  [threads=8] [array=8] [size=100]       alloc" 
    "\n  ./benchmark  queue  [threads=8] [array=8] [size=100]       queue" 
    "\n";
    return str;
}

int getInt (char* th) {
    int sum = 0;
    for (int n = 0; ; n++) {
        if (th[n] == 0)
            return sum;
        if (th[n] >= '0' && th[n] <= '9') {
            sum *= 10;
            sum += th[n] - '0';
            continue;
        }
        return -1;
    }
}

