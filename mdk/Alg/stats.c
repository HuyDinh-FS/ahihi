//
//  stats.c
//  stepcount
//
//  Created by james ochs on 5/9/14.
//  Copyright (c) 2014 Misfit. All rights reserved.
//

#include <stdio.h>
#include "scalgtypes.h"
#include "stats.h"
#include <math.h>

real_t max(real_t A, real_t B)
{
    return B>A ? B : A;
}

//copy x - mean(x) into vector y
void removeDCCopy(const real_t *x, real_t *y, int32_t windowLen)
{
    int32_t i;
    real_t m = mean(x,windowLen);
    for(i=0; i<windowLen; i++)
    {
        y[i] = x[i]-m;
    }
    
}

real_t mean(const real_t *windowData, int32_t windowLen)
{
    int i;
    real_t res = 0;
    
    for (i = 0; i < windowLen; i++)
    {
        res += windowData[i];
    }
    
    return (res / windowLen);
}


real_t variance(const real_t *windowData, int windowLen)
{
    int i;
    real_t sum = 0;
    real_t m = mean(windowData, windowLen);
    
    for (i = 0; i < windowLen; i++)
    {
        sum += (windowData[i] - m) * (windowData[i] - m);
        
    }
    
    return (sum / (windowLen - 1));
}

//NOTE: This function assumes that wdata is zero mean
void autocorr(const real_t *wdata, real_t *correlation, int32_t maxLag, int32_t signalLen)
{
/* This function is highly optimized, somewhat at the sake of readability.  It is the most intensive
   operation we perform in shine_J ...here is the original code with comments as to why it is suboptimal*/
  
   //ORIGNAL CODE STARTS HERE....
   /*    
    int i, j; 
    for (i = 0; i < maxLag; i++)
    {
        correlation[i] = 0;
        for (j = 0; j < signalLen - i; j++)
        {
            //this line in the inner loop is executed a lot
            //it can be optimized by
            //1.) not using correlation[i] but instead using a temp "sum" variable
            //    using correlation[i] means that we need to increment the address
            //    where the result is added 
            //2.) Not doing [j+i] every time in the second term of the multiply.  Instead
            //we could create a second array that starts at wdata[i] (e.g, array2 = &wdata[i]  and increment that
            correlation[i] += wdata[j] * wdata[j + i];  
            
        }
    }
    */
    //ORIGINAL CODE ENDS HERE....

    //OPTIMIZED CODE...  this is ~44% faster on the M0 with our without compiler optimization    
    int32_t lag,sum;
    const real_t *pSignal=wdata;
    const real_t *pShiftedSignal;
    const real_t *pEndOfSignal = pSignal+signalLen;
    
    for(lag=0; lag<maxLag; lag++)
    {
      sum = 0;
      pSignal=wdata;
      pShiftedSignal = pSignal+lag;
      while(pShiftedSignal < pEndOfSignal)
      {
        //this line is doing:  correlation[i] += wdata[i] * wdata[i + lag];
        //where pSignal = wdata and pShiftedSignal = wdata[lag]        
        sum += *pSignal++ * *pShiftedSignal++;
      }
      correlation[lag] = sum;
    }
}


//mean absolute deviation
float mad(int32_t *v, int32_t len)
{
  float ave = 0.0, mad = 0.0;
  for(int i=0; i < len; i++)
  {
    ave += v[i];
  }
  ave /= len;
  
  for(int i=0; i<len; i++)
  {
    mad += fabs(ave - v[i]);
  }
  mad /= len;
  return mad;
}

//////////////////////////////////////////////////////////////////
//	median
//
//	Description:	calculate median value of an array, based on Quickselect method
//                runs in average linear time
//
//	Inputs:	x - input array
//			n - signal length
//	Outputs: Returns a[median] median value of array
//			x - partially sorted array
//
//	Note:	output will not be fully sorted as this is not necessary for median calculation
//////////////////////////////////////////////////////////////////
float median(int32_t *x, int n)
{
  int k = n/2;
  
  if(n==0)
  {
    return 0; //?
  }
  else if(n == 1)
  {
    return x[0];
  }
  else if(n == 2)
  {
    return (x[0] + x[1])/2.0;
  }
  
  int left = 0;
  int right = n-1;
  
  //we stop when our indicies have crossed
  while (left < right)
  {
    
    int pivot = (left + right)/2; //this can be whatever
    int pivotValue = x[pivot];
    int storage=left;
    
    x[pivot] = x[right];
    x[right] = pivotValue;
    for(int i =left; i < right; i++){//for each number, if its less than the pivot, move it to the left, otherwise leave it on the right
      if(x[i] < pivotValue){
        int temp =x[storage];
        x[storage] = x[i];
        x[i]=temp;
        storage++;
      }
    }
    x[right]=x[storage];
    x[storage]=pivotValue;//move the pivot to its correct absolute location in the list
    
    //pick the correct half of the list you need to parse through to find your K, and ignore the other half
    if(storage < k)
      left = storage+1;
    else//storage>= k
      right = storage;
    
  }
  
  if(n % 2 == 0)
  {
    int maxVal = x[k-1];
    for(int i=0; i<k; i++)
    {
      if(x[i] > maxVal)
      {
        maxVal = x[i];
      }
    }
    return (float)(x[k] + maxVal)/2.0f;
  }
  else
  {
    return x[k];
  }
  
}



