/*
 * Jewel Store Copyright (C) 2011-2014  TechoPhil Ltd
 *
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package benchmarkJava;

import java.util.HashMap;
import java.util.Random;
import java.util.TreeMap;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicLong;
import share.Randomizer;
import share.UtilsNumbers;

import share.cli.Args;





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
    static final int ARRAY_SIZ_MAX = 100;
  
    static final int THREAD_MAX = 8;
    static final boolean THREAD_POOL = true;
    //static final boolean WITH_LOCK = true;
    static final int TREE_KEY_MAX = 100 * 1000;
    static AtomicLong  s_loops = new AtomicLong();
    static long s_printFilter;
    static final int PRINT_FILTER = 5000000;
        
    //static AtomicInteger s_count = new AtomicInteger(); 

    enum Test {
        tree,
        alloc,
        queue,
        hash,
        treeNoLock,
        copy,
        prime,
        loops,
    }
    
    static Test s_test;
    static SafeArray [] s_arr;
    static SafeTree [] s_treeArray;
    static SafeHash [] s_hasharray;
    static long s_copyArea [][];
    static final int QUEUE_SIZE = 10000;
    static BlockingQueue <Struc>  s_strucQ = new ArrayBlockingQueue <>(QUEUE_SIZE);

    static BlockingQueue <Struc>  s_strucQ_final = new ArrayBlockingQueue <>(QUEUE_SIZE);    
//static ReadWriteLock [] lock = new ReadWriteLock [ARRAY_NUM];
    static Random s_random = new Random();
    static int s_threadCnt = THREAD_MAX;
    static long s_startTimeMili;
    static int s_arraySize;
    static long s_primeDelay;
    static boolean s_verbose;
    
    long m_timeOfPrint;
    long m_loopsOfPrint;

    Randomizer m_randomizer = new Randomizer(); // faster randomizer 232000
    
    //static Randomizer m_randomizer = new Randomizer(); // 132000   
    //Random m_randomizer = new Random();                // 160000
    //static Random m_randomizer = new Random();         // 2588
    
    static int s_arrayNum;
    static boolean s_noLock;
    
    int m_id;
    
    
    public BenchmarkJava(int id) {
        m_id = id;
    }
    

    static String help () {
        String str = "";
        str += "\n  java -jar dist/overhead.jar  tree|hash|noLock|alloc|queue [threads=8]  [size=1000000] [array=8] ";
        str += "\n  java -jar dist/overhead.jar  tree  threads=8  size=1000000 array=8 ";
        str += "\n  java -jar dist/overhead.jar  alloc  "; 
        str += "\n  java -jar dist/overhead.jar  queue  ";
       
        str += "\ntestList:   ";
        for (Test test : Test.values())
            str += test + ", ";
        str += "\n   ";
        return str;
    }
    
    public static void main (String[] args) {
        //UtilsNumbers.formatIntUtest();
        //Args.utest();        
        int threads = Args.getInteger("threads", args, "number of concurrent threads");
        if (threads != Integer.MAX_VALUE)
            s_threadCnt = threads;
        else
            s_threadCnt = Runtime.getRuntime().availableProcessors();

        int arrays = Args.getInteger("arrays", args, "number of arrays, accessed by threads");
        if (arrays != Integer.MAX_VALUE)
            s_arrayNum = arrays;
        else       
            s_arrayNum = s_threadCnt;
        
        int size = Args.getInteger("size", args, "size of each array/tree");
        if (size != Integer.MAX_VALUE)
            s_arraySize = size;
        else       
            s_arraySize = TREE_KEY_MAX;        

        int print_filter = Args.getInteger("printFilter", args, "print filter (fraction of loops that print)");
        if (print_filter != Integer.MAX_VALUE)
            s_printFilter = print_filter;
        else       
            s_printFilter = PRINT_FILTER;

        boolean tree = Args.getBool ("tree", args, "test==tree");
        if (tree) {
            assert s_test == null : s_test;            
            s_test = Test.tree;
        }
        
        boolean alloc = Args.getBool ("alloc", args, "test==alloc");
        if (alloc) {
            assert s_test == null : s_test;            
            s_test = Test.alloc;
        }
        
        s_noLock = Args.getBool ("noLock", args, "tree no lock (Each thread has private array)");
        if (s_noLock) {
            if (s_arrayNum != s_threadCnt) {
                String str ="\n######  noLock requires #array==#threads\n " + help();
                Args.exit (str, 1);                
            }
            if (s_test == Test.tree)
                s_test = Test.treeNoLock;
        }
        
        boolean queue =   Args.getBool ("queue", args, "test==queue");
        if (queue) {
            assert s_test == null : s_test;            
            s_test = Test.queue;
        }
        boolean hash = Args.getBool ("hash", args, "test==hash");
        if (hash) {
            assert s_test == null : s_test;
            s_test = Test.hash;
        }
        boolean copy = Args.getBool ("copy", args, "test==copy_memory (measure bandwidth)");
        if (copy) {
            assert s_test == null : s_test;
            s_test = Test.copy;
        }
        boolean prime = Args.getBool ("prime", args, "test==prime");
        if (prime) {
            assert s_test == null : s_test;
            s_test = Test.prime;
            if (s_arraySize == TREE_KEY_MAX)
                s_arraySize = 5000;
        }
        
        boolean loop = Args.getBool ("loops", args, "test==loops");
        if (loop) {
            assert s_test == null : s_test;
            s_test = Test.loops;
        }
        boolean verbose = Args.getBool ("verbose", args, "print test param during run");
        if (verbose)
            s_verbose = true;
        
        Args.showAndVerify (true);
        
        s_arr = new SafeArray [s_arrayNum];
        s_treeArray = new SafeTree [s_arrayNum];
        s_hasharray = new SafeHash [s_arrayNum];
        if (s_test == null) {
            String str ="missing testName, see examples: " + help();
            Args.exit (str, 1);
        }
        switch (s_test) {
            case alloc:
                for (int i = 0; i < s_arrayNum; i++) {
                    s_arr[i] = new SafeArray (s_arraySize);
                }
                break;
        
            case tree:
            case treeNoLock:
                for (int i = 0; i < s_arrayNum; i++) {
                    s_treeArray[i] = new SafeTree();            
                    for (int n = 0; n < s_arraySize/8; n++) {
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
                s_printFilter = 10000;
                break;
                
            case queue:
                s_printFilter = 10000;                
                for (int n = 0; n < 1000000; n++) {
                    if (n % 1000 == 0) {
                        if (s_strucQ_final.size() > 0)
                            break;
                    }
                    Struc struc = new Struc(); // create and discard
                }
                for (int n = 0; n < QUEUE_SIZE; n++) {
                    Struc struc = new Struc();
                    s_strucQ.add(struc);
                }
                break;
                
            case prime:
                s_printFilter = 50;
                break;
                
            case loops:
                break;

            default:
                Args.exit ("invalid arg: " + help(), 2);
        }       
        
        s_startTimeMili = System.currentTimeMillis();
        System.err.print("\n" + show());

        if (THREAD_POOL) {
            ExecutorService executor = Executors.newFixedThreadPool(s_threadCnt);
            //ExecutorService executor = Executors.newSingleThreadExecutor();
            for (int n = 0; n < s_threadCnt; n++) {
                Runnable worker = new BenchmarkJava (n);
                executor.execute(worker);
            }                
        }
        else {
            for (int n = 0; n < s_threadCnt; n++) { 
                Runnable runable = new BenchmarkJava (n);
                Thread thread = new Thread (runable);
                thread.start();
            }
        }
    }
    

    
    public void run () {
        //Random m_generator = new Random();
        for (long loops = 1; ; loops++) {
            if ((loops & 0xf) == 0)
                s_loops.addAndGet(0x10);
            int key = -1;
            int arr_num = -1;
            if (s_test != Test.loops) {
                key = m_randomizer.nextInt(s_arraySize);
                if (s_test == Test.copy && s_threadCnt == s_arrayNum) 
                    arr_num = m_id; // each thread uses its own buffers
                else
                    arr_num = m_randomizer.nextInt(s_arrayNum);
            }
            Integer val;
            switch (s_test) {
                case tree:
                    val = s_treeArray[arr_num].get(key);
                    break;
                    
                case treeNoLock:
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
                    System.arraycopy(s_copyArea[arr_num], 0, s_copyArea[arr_num + s_arrayNum], 0, s_arraySize);
//                    for (int i = 0; i < s_arraySize; i++)
//                        s_copyArea[arr_num][i] = s_copyArea[arr_num + s_arrayNum][i]; 
                    break;
                    
                case queue:
                    Struc struc = s_strucQ_final.poll();
                    if (struc == null)
                        struc = s_strucQ.poll();
                    if (struc != null)
                        s_strucQ.add (struc);

                    for (int j = 0; j < 100; j++) {
                        struc = new Struc();
                    }                    
                    break;
                    
                case prime:
                    long startMili = System.currentTimeMillis();
                    int count = PrimeNumbers.countPrimes(s_arraySize);
                    s_primeDelay = System.currentTimeMillis() - startMili;
                    count += 1;                
                    break;
                    
                case loops:
                    int rand = m_randomizer.nextInt();
                    int rand1 = m_randomizer.nextInt();
                    int rand2 = m_randomizer.nextInt();
                    int rand3 = m_randomizer.nextInt();
                    break;
                    
                    
                default:
                    assert false: "invalid test";
            }
            
            if ((loops % s_printFilter) == 0) {            
                long delay = System.currentTimeMillis() - s_startTimeMili;
                long delayPrint = System.currentTimeMillis() - m_timeOfPrint;
                String txt = "\n";
                if (s_verbose)
                    txt += show();

                txt += " loops=" + UtilsNumbers.formatInt(loops);
                txt += " miliSec=" + UtilsNumbers.formatInt(delay);
                txt += " thread=" + m_id;
                if (delayPrint> 0)
                txt += " loopsPerMiliPerThread=" + (loops - m_loopsOfPrint) / delayPrint;
                //txt += " s_strucQ_final=%d" + overhead.s_strucQ_final.size();
                txt += " grossLoops=" + UtilsNumbers.formatInt(s_loops.get());
                if (delay / 1000 > 0) {
//                    if (s_test == Test.copy || s_test == Test.prime)
//                        txt += " grossLoopsPerSec=" + (s_loops.get() / (delay / 1000));
//                    else
                        txt += " grossLoopsPerMiliSec=" + UtilsNumbers.formatInt (s_loops.get() / delay);
                }
                if (s_test == Test.prime)
                    txt += " delay=" + s_primeDelay;    
                System.err.print(txt);
                
                m_timeOfPrint = System.currentTimeMillis();
                m_loopsOfPrint = loops;
            }
        }
      
    }   
    
    static String show () {
        String txt = "java test=" + s_test.toString();
        txt += " threads=" + s_threadCnt;
        txt += " arrayNum=" + s_arrayNum;
        txt += " size=" + s_arraySize;
        txt += " filter=" + s_printFilter;
        txt += " noLock=" + s_noLock;
        
        return txt;
    }
        
}

class Struc {
    int siz = BenchmarkJava.s_random.nextInt(10) * 4;
    long m_l [] = new long[siz];
    byte m_b;
    String m_str;
    protected void finelize () {
        if (BenchmarkJava.s_test == BenchmarkJava.Test.loops)
            BenchmarkJava.s_strucQ_final.add (this);
        
    }
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


