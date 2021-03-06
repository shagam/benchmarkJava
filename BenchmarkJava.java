/*
 * Jewel Store Copyright (C) 2011-2016  TechoPhil Ltd
 *
 * 
 * Licensed under MIT license
 *
 */
package benchmarkJava;

import java.util.HashMap;
import java.util.Random;
import java.util.TreeMap;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;






/**
 *
 * @author eli
 */

/*
 *
 */
public class BenchmarkJava implements Runnable {

    /**
     * @param args the command line arguments
     */
    //static final int ARRAY_SIZ_MAX = 100;
  
    static final int THREAD_MAX = 8;
    static final boolean THREAD_POOL = true;
    //static final boolean WITH_LOCK = true;
    static final int TREE_KEY_MAX = 100 * 1000;
    static AtomicLong  s_commonCounter = new AtomicLong();
    static long s_grossLoops[];    
    //static AtomicInteger s_count = new AtomicInteger(); 

    public static enum Test {
        tree,
        alloc,
        queue,
        hash,
        noLockTree,
        copy,
        prime,
        count,
    }
    
    public static Test s_test;
    static SafeArray [] s_arr;
    static SafeTree [] s_treeArray;
    static SafeHash [] s_hasharray;
    static long s_copyArea [][];
    static final int QUEUE_SIZE = 10000;
    static BlockingQueue <Struc>  s_strucQ; // = new ArrayBlockingQueue <>(QUEUE_SIZE);
  
    static Random s_random = new Random();
    static int s_threadCnt = THREAD_MAX;
    static long s_startTimeMili;
    static int s_arraySize;
    static int s_chunkAlloc;
    static int s_delay;
    static boolean s_verbose;
    static boolean s_help;
    static Thread [] s_thread;

    Randomizer m_randomizer = new Randomizer(); // faster randomizer 232000
    
    //static Randomizer m_randomizer = new Randomizer(); // 132000   
    //Random m_randomizer = new Random();                // 160000
    //static Random m_randomizer = new Random();         // 2588
    
    static int s_arrayNum;
    static boolean s_noLock;
    
    int m_id;
    int m_found = 0;
    int m_notFound = 0;
    
    
    public BenchmarkJava(int id) {
        m_id = id;
    }
    

    static String help () {
        String str = "";
        str += "\n  java -jar dist/overhead.jar  tree|hash|noLock|alloc|queue|prime|count [threads=8]  [size=1000000] [array=8] ";
        str += "\n  java -jar dist/overhead.jar  tree  threads=8  size=1000000 array=8 ";

        
        str += "\ntestList:   ";
        for (Test test : Test.values())
            str += test + ", ";
        str += "\n   ";
        return str;
    }
    
    public static void main (String[] args) {
        //UtilsNumbers.formatIntUtest();
        //Args.utest();        
        String [] tests = {Test.tree.toString(), Test.noLockTree.toString(), Test.hash.toString(), Test.queue.toString(),
            Test.alloc.toString(),Test.copy.toString(),
            Test.prime.toString(),Test.count.toString()};
        String [] testHelp = {
            "tree retrieve; multi tree, multi thread",
            "noLockTree retrieve without lock; each thread access private tree",
            "hash retrieve; multi hush, multi thread",
            "queue get/enque; multi queue, multi thread", 
            "alloc new / delete (With variable arraySize)",
            "copy mem (Bandwidth) private area for each thread",            
            "prime compute intensive; no closions",
            "common counter increment by many threads",
        };
        String test = Args.getEnum ("testName", args, tests, testHelp, "test selection ");
        
        s_threadCnt = Args.getInteger("threads", args, "number of concurrent threads");
        if (s_threadCnt == Integer.MAX_VALUE)
            s_threadCnt = Runtime.getRuntime().availableProcessors();
        s_thread = new Thread [s_threadCnt];
        s_grossLoops = new long [s_threadCnt];

        s_arrayNum = Args.getInteger("arrays", args, "number of arrays, accessed by threads");
        if (s_arrayNum == Integer.MAX_VALUE)     
            s_arrayNum = s_threadCnt;
        
        s_arraySize = Args.getInteger("size", args, "size of each array/tree");      
        
        s_chunkAlloc = Args.getInteger("chunkAlloc", args, "chunk size of alloc");
        if (s_chunkAlloc == Integer.MAX_VALUE)
            s_chunkAlloc = 1000;

        s_delay = Args.getInteger("delay", args, "duration of test");
        if (s_delay == Integer.MAX_VALUE)
            s_delay = 5000;
        
        boolean verbose = Args.getBool ("verbose", args, "print test param during run");
        if (verbose)
            s_verbose = true;
        
        s_help = Args.getBool("help", args, "help text");
        if (s_help) {
            System.err.print (Args.optionalParams());
            Args.exit ("", 0);            
        }
                
        if (test != null ) {
            for (Test test1 : Test.values()) {
                if (test.matches (test1.toString())) {
                    s_test = test1;
                    break;
                }
            }
        }
        else {
            System.err.print ("\n*** Error missing testName=...\n\n");
            System.err.print (Args.optionalParams());
            Args.exit ("", 1);
        }
            
        if (s_arraySize == Integer.MAX_VALUE) {
            if (s_test == Test.prime || s_test == Test.alloc)
                s_arraySize = 1000;
            else
                s_arraySize = TREE_KEY_MAX;
        }
        
        Args.showAndVerify (s_help);
        if (s_help)
            System.exit(0);
        
        s_arr = new SafeArray [s_arrayNum];
        s_treeArray = new SafeTree [s_arrayNum];
        s_hasharray = new SafeHash [s_arrayNum];
        if (s_test == null) {
            String str ="missing testName, see examples: " + help();
            Args.showAndVerify (false);
            Args.exit (str, 1);
        }
        switch (s_test) {
            case alloc:
                for (int i = 0; i < s_arrayNum; i++) {
                    s_arr[i] = new SafeArray (s_arraySize);
                }
                break;
        
            case tree:
            case noLockTree:
                if (s_test == Test.noLockTree && s_arrayNum != s_threadCnt) {
                    String str ="\n######  noLock requires #array==#threads\n " + help();
                    Args.exit (str, 1);                
                }
            
                for (int i = 0; i < s_arrayNum; i++) {
                    s_treeArray[i] = new SafeTree();            
                    for (int n = 0; n < s_arraySize; n++) {
                        int in = s_random.nextInt(s_arraySize);
                        s_treeArray[i].put(in, in);
                    }
                }
                break;

            case hash:
                for (int i = 0; i < s_arrayNum; i++) {
                    s_hasharray[i] = new SafeHash(TREE_KEY_MAX * 5);            
                    for (int n = 0; n < s_arraySize/20; n++) {
                        int val = s_random.nextInt(s_arraySize);
                        s_hasharray[i].put(val, val);
                    }
                }
                break;
       
            case copy:
                s_copyArea = new long [s_arrayNum * 2][s_arraySize];
                break;
                
            case queue:
                s_strucQ = new ArrayBlockingQueue <>(s_arraySize);
                for (int n = 0; n < s_arraySize; n++) {
                    Struc struc = new Struc();
                    s_strucQ.add(struc);
                }
                break;
                
            case prime:
            case count:
                break;

            default:
                Args.showAndVerify (true);
                Args.exit ("invalid arg: " + help(), 2);
        }
        
        if (s_arraySize == Integer.MAX_VALUE)    
            s_arraySize = TREE_KEY_MAX; 
        
        s_startTimeMili = System.currentTimeMillis();
        System.err.print(show());

        if (THREAD_POOL) {
            ExecutorService executor = Executors.newFixedThreadPool(s_threadCnt);
            //ExecutorService executor = Executors.newSingleThreadExecutor();
            for (int n = 0; n < s_threadCnt; n++) {
                Runnable worker = new BenchmarkJava (n);
                executor.execute(worker);
            }

            executor.shutdown();
            try {
                while (!executor.awaitTermination(24L, TimeUnit.HOURS)) {
                    System.out.println("Not yet. Still waiting for termination");
                }
            } catch (InterruptedException e) {
                e.printStackTrace();
            }
        }
        else {
            for (int n = 0; n < s_threadCnt; n++) {
                Runnable runable = new BenchmarkJava (n);
                s_thread[n] = new Thread (runable);
                s_thread[n].start();
            }
            for (int i = 0; i < s_threadCnt; i++){
                try {
                    s_thread[i].join();
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
        }

        long grossLoopCunt = 0;
        for (int n = 0; n < s_threadCnt; n++)
            grossLoopCunt += s_grossLoops[n];
        
        long delayMili = System.currentTimeMillis() - s_startTimeMili;  
        System.err.print("\n\napplication exit; grossLoops=" + formatInt(grossLoopCunt));
        long mbperSec = grossLoopCunt * s_arraySize * 8 * 2 / 1000000 / (delayMili / 1000);
        if (s_test == Test.copy)
            System.err.print("      grossMBperSec=" + formatInt(mbperSec) + "\n\n");
        System.err.print("\n\n");
        show();
    }
    

    
    public void run () {
        //Random m_generator = new Random();
        for (long loops = 1;  ; loops++) {
            if ((loops & 0xf) == 0) {
                s_grossLoops[m_id] += 0x10;
                if (System.currentTimeMillis() - s_startTimeMili > s_delay)
                    break;
            }
            int key = -1;
            int arr_num = -1;

            key = m_randomizer.nextInt(s_arraySize);
            if (s_test == Test.copy && s_threadCnt == s_arrayNum) 
                arr_num = m_id; // each thread uses its own buffers
            else
                arr_num = m_randomizer.nextInt(s_arrayNum);

            Integer val;
            switch (s_test) {
                case tree:
                    val = s_treeArray[arr_num].get(key);
                    if (val == null)
                        m_notFound ++;
                    else
                        m_found ++;
                    break;
                    
                case noLockTree:
                    val = s_treeArray[m_id].get_notSynchronized(key);
                    break;
                    
                case hash:
                    val = s_hasharray[arr_num].get(key);
                    break;
                    
                case alloc: // alloc array
                    int indx = m_randomizer.nextInt(s_arraySize);
                    assert indx >= 0 : indx;
                    Struc a = new Struc();
                    s_arr[arr_num].put (indx, a);
                    break;
                    
                case copy:
                    //System.arraycopy(s_copyArea[arr_num], 0, s_copyArea[arr_num + s_arrayNum], 0, s_arraySize);
                    for (int i = 0; i < s_arraySize; i++)
                        s_copyArea[arr_num][i] = s_copyArea[arr_num + s_arrayNum][i]; 
                    break;
                    
                case queue:
                    Struc struc = s_strucQ.poll();
                    if (struc != null)
                        s_strucQ.add (struc);
                   
                    break;
                    
                case prime:
                    int count = PrimeNumbers.countPrimes(s_arraySize);               
                    break;
                    
                case count: //s_grossLoops is incremented
                    s_commonCounter.addAndGet(1);
                    break;
                                       
                default:
                    assert false: "invalid test";
            }           
        }
    }   
    
    static String show () {
        String txt = "benchmark java: test=" + s_test.toString();
        txt += " delay=" + s_delay;
        txt += " threads=" + s_threadCnt;
        txt += " arrayNum=" + s_arrayNum;
        txt += " size=" + s_arraySize;
        if (s_noLock)
            txt += " noLock=" + s_noLock;
        if (s_test == Test.alloc)
            txt += " chunkAlloc=" + s_chunkAlloc;
        
        return txt;
    }
    
   public static String formatInt (long n) {
        final int BILION = 1000000000; 
        final int MILION = 1000000; 
        final int KILO = 1000; 
        boolean fill = false;
        String str = "";
        if (n > BILION) {
            //str += n / BILION + ",";
            str += String.format("%d,", n / BILION);
            n -= n / BILION * BILION;
            fill = true;
        }
        if (n > MILION) {
            //str += n / MILION + ",";
            if (fill)
                str += String.format("%03d,", n / MILION);
            else
                str += String.format("%d,", n / MILION);
            n -= n / MILION * MILION;
            fill = true;
        }        
        if (n > KILO) {
            //str += n / KILO + ",";
            if (fill)
                str += String.format("%03d,", n / KILO);
            else
                str += String.format("%d,", n / KILO);
            n -= n / KILO * KILO;
            fill = true;            
        }
        if (fill)
            str += String.format("%03d", n);
        else
            str += n;
        
        return str;
    }

        
}

class Struc {
    int siz = 10;
    long m_l [];
    public Struc () {
        
        if (BenchmarkJava.s_test == BenchmarkJava.Test.alloc)
            siz += BenchmarkJava.s_random.nextInt(BenchmarkJava.s_chunkAlloc);
        else
            siz += 100; //BenchmarkJava.s_random.nextInt(40);        
        m_l = new long[siz];
    }


    byte m_b;
    String m_str;
}

class SafeTree {
    TreeMap <Integer, Integer> s_tree = new TreeMap<>();
    
    public synchronized void put (int key, int val) {
        s_tree.put(key, val);
    }
    
    public synchronized Integer get (int key) {
        return s_tree.get(key);
    }   

    public Integer get_notSynchronized (int key) {
        return s_tree.get(key);
    }  

}

class SafeHash {
    HashMap <Integer, Integer> s_hash;
    
    SafeHash (int siz) {
        s_hash = new HashMap<>(siz);
    }
    
    public synchronized void put (int key, int val) {
        s_hash.put(key, val);
    }
    
    public synchronized Integer get (int key) {
        return s_hash.get(key);
    }   
}

class SafeArray {
    Struc [] s_array;
    
    SafeArray (int siz) {
        s_array = new Struc[siz];
    }
    
    public synchronized void put (int indx, Struc val) {
        s_array[indx] = val;
    }
    
    public synchronized Struc get (int indx) {
        return s_array[indx];
    }   
}


