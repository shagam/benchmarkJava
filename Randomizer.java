/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package benchmarkJava;

import java.util.Random;

/**
 *
 * @author eli
 * 
 * non synchronized 
 */
public class Randomizer {
    
    static final boolean SHIFT_RANDOM = true;
    long seed = System.currentTimeMillis();
    Random m_rand = new Random(); // old randomizer
    
    
    public long nextLong() {
        if (! SHIFT_RANDOM)
            return m_rand.nextLong();
        
          long x = seed;
          x ^= (x << 21);
          x ^= (x >>> 35);
          x ^= (x << 4);
          seed = x;
          return x;
    } 
        
    public int nextIntMask(int nbits) {
          long x = nextLong();
          x &= ((1L << nbits) - 1);
          return (int) x;
    }

    public int nextInt() {
        int x = (int) nextLong();
          if (x < 0)
              x = 0 - x;
          if (x < 0)
            assert false : x; 
          return x;
    }
    
    public int nextInt(int devider) {
          int num = (int) nextLong();
          int result  = num % devider;
          if (result < 0)
              result = 0 - result;
          assert result >= 0;
          return result;
     }
    
    
}

