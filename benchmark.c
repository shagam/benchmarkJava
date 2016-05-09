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
#include <assert.h>

//#include <hash_map>

//* Jewel Store Copyright (C) 2011-2016  TechoPhil Ltd
// Licensed under MIT license

using namespace std;

long getTimeMili () {
    timeval time;
    gettimeofday(&time, NULL);
    long millis = (time.tv_sec * 1000.0) + (time.tv_usec / 1000.0) + 0.5;
    return millis;
}

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
    t_tree,
    t_noLockTree,
    t_hash,
    t_queue,
    t_alloc,
    t_copy,    
    t_prime,
    t_count,
    t_help,
};    

int s_test;

class Struc {
    long *m_longArray;
    char m_byte;
    char *m_str;
public:
    Struc () {
        int siz;
        if (s_test == t_alloc)
            siz = (rand() % s_arraySize);
        else
            siz = (rand() % 40);
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
     else if (s_test == t_noLockTree) {
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
     else if (s_test == t_count) {
         
     }
     else {
         fprintf (stderr, "wrong state %s", help());
         exit (1);         
     }
  }
}

int main(int argc, char * argv[])  {
    //formatIntUtest ();
    //utest ();
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

    if (getBool("verbose", argc, argv, "print test params during test run"))
        s_verbose = 1;    
    if (getBool("help", argc, argv, "print arguments"))
        s_help = 1;
    
    s_delay = getInteger ("delay", argc, argv, "time limit of test ");
    if (s_delay == -1)
        s_delay = 5000;

    const char * tests[] = {"tree","noLock","hash","queue","alloc","copy","prime","count"};
    const char * testHelp[] = {
        "tree retrieve; multi tree, multi thread",
        "noLocktree retrieve without lock; each thread access private tree",
        "hash retrieve; multi hush, multi thread",
        "queue get/enque; multi queue, multi thread", 
        "alloc new / delete (With variable arraySize)",        
        "copy mem (Bandwidth) private area for each thread",
        "prime compute intensive; no closions",
        "common counter increment by many threads",  
    };    
    const char * test = argsGetEnum ("testName", argc, argv, 8, tests, testHelp, "");
    
    if (s_help) {
        args_report();
        exit (0);
    }

    int found;
    if (test != NULL ) {
        for (int testNum = 0; testNum <= t_count; testNum++) {
            if (strcmp (test, tests[testNum]) == 0) {
                s_test = testNum;
                found = 1;
                break;
            }
        }
    }
    else {
        fprintf (stderr, "\nMissing testName=tree|noLockTree|hash|queue|alloc|copy|prime|count or help  \n");
        args_report();        
        exit (2);        
    }
    
    if (found == 0) {
        args_report();
        fprintf (stderr, "\nmissing testName=tree|noLockTree|hash|queue|alloc|copy|prime|count or help  \n");
        exit (5);         
    }    
    // default for all other tests
    if (s_arraySize == -1) {
        if (s_test == t_prime || s_test == t_alloc)
            s_arraySize = 1000;
        else
            s_arraySize = ARRAY_SIZE;
    }
    

    
    // initialize data structures
    switch (s_test) {
        case t_alloc:
            s_testName = "alloc"; 
            for (int i= 0; i < s_arrayNum; i++) {
                 s_arr[i] = new SafeArray();
            }            
            break;
            
        case t_prime:
            s_testName = "prime";            
            break;
            
        case t_noLockTree:
            if (s_arrayNum < s_threadCnt)
                s_arrayNum = s_threadCnt;
            s_testName = "noLockTree";
            // fall through
        case t_tree:
            if (s_testName == NULL)
                s_testName = "tree";
            for (int i= 0; i < s_arrayNum; i++) {
                s_treeArray[i] = new SafeTree();
                for (int n = 0; n < s_arraySize; n++) {
                    int key = rand() % s_arraySize;
                    s_treeArray[i]->put (key, key);
                }
            }            
            break;
            
        case t_hash:
            s_testName = "hash";              
            for (int i= 0; i < s_arrayNum; i++) {         
                 s_hashArray[i] = new SafeHash();
                 for (int n = 0; n < s_arraySize/20; n++) {
                    int key = rand() % s_arraySize;
                    s_hashArray[i]->put (key, key);
                }
             }      
            break;
            
        case t_queue:
            s_testName = "queue";            
            for (int i= 0; i < s_arraySize; i++) {         
                Struc* struc = new Struc();
                s_queue->enque (struc);
            }           
            break;
            
        case t_copy:
            s_testName = "copy";            
            s_copyArrays = new long* [s_arrayNum * 2];
            for(int i = 0; i < s_arrayNum * 2; ++i)
                s_copyArrays[i] = new long [s_arraySize];            
            break;
            
        case t_count:
            s_testName = "count";            
            break;
            
        case t_help:
            break;
            
        default:
            args_report();
            fprintf (stderr, "first param need: tree, alloc, hash, queue, copy, prime, count or help  \n");
            exit (2);             
    }
    show();

    const char * err = verify (argc, argv);
    if (err != NULL) {
        fprintf (stderr, "invalid param=%s  \n", err);
        exit (3);
    }
        
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

