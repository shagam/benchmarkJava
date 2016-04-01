/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package benchmarkJava;

/**
 *
 * @author eli
 */
public class PrimeNumbers {
    

   public static int countPrimes (int range)
   {		
       int i =0;
       int primeCount = 0;
       int num =0;
       //Empty String
       //String  primeNumbers = "";

       for (i = 1; i <= range; i++)
       { 		  	  
          int counter=0; 	  
          for(num =i; num>=1; num--)
	  {
             if(i%num==0)
	     {
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
//       System.out.println("Prime numbers from 1 to 100 are :");
//       System.out.println(primeNumbers);
   }
}