/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_abs_q31.c    
*    
* Description:	Q31 vector absolute value.    
*    
* Target Processor: Cortex-M4/Cortex-M3/Cortex-M0
*  
* Redistribution and use in source and binary forms, with or without 
* modification, are permitted provided that the following conditions
* are met:
*   - Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   - Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in
*     the documentation and/or other materials provided with the 
*     distribution.
*   - Neither the name of ARM LIMITED nor the names of its contributors
*     may be used to endorse or promote products derived from this
*     software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
* COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
* ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE. 

 Modifications 2017  Mostafa Saleh       (Ported to RISC-V PULPino)
* -------------------------------------------------------------------- */

#include "riscv_math.h"

/**    
 * @ingroup groupMath    
 */

/**    
 * @addtogroup BasicAbs    
 * @{    
 */


/**    
 * @brief Q31 vector absolute value.    
 * @param[in]       *pSrc points to the input buffer    
 * @param[out]      *pDst points to the output buffer    
 * @param[in]       blockSize number of samples in each vector    
 * @return none.    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function uses saturating arithmetic.    
 * The Q31 value -1 (0x80000000) will be saturated to the maximum allowable positive value 0x7FFFFFFF.    
 */

void riscv_abs_q31(
  q31_t * pSrc,
  q31_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  q31_t in,out;                                      /* Input value */



#if defined (USE_DSP_RISCV)

  /*loop Unrolling */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = |A| */
    /* Calculate absolute value of the input (if -1 then saturated to 0x7fffffff) and then store the results in the destination buffer. */
    in = *pSrc++; /*read from source buffer and increment it for next read */
   /*check for saturation*/
    if(in == 0x80000000)
    {
	*pDst++ = INT32_MAX; 
    }
    else
    {
    /*inline assembly to preform abs with in one instruction*/
         asm volatile ("p.abs %[d], %[a]"
                : [d] "=r" (out)
                : [a] "r"  (in));
         *pDst++ = out; /*write the output to destination buffer*/
     }
    blkCnt--; /*decrement loop counter*/
  }


#else

  /* Initialize blkCnt with number of samples */
blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = |A| */
    /* Calculate absolute value of the input (if -1 then saturated to 0x7fffffff) and then store the results in the destination buffer. */
    in = *pSrc++;
    *pDst++ = (in > 0) ? in : ((in == INT32_MIN) ? INT32_MAX : -in);

    /* Decrement the loop counter */
    blkCnt--;
  }
#endif 
}

/**    
 * @} end of BasicAbs group    
 */
