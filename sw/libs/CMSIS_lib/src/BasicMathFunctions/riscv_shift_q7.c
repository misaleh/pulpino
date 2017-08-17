/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_shift_q7.c    
*    
* Description:	Processing function for the Q7 Shifting    
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
 * @addtogroup shift        
 * @{        
 */


/**        
 * @brief  Shifts the elements of a Q7 vector a specified number of bits.        
 * @param[in]  *pSrc points to the input vector        
 * @param[in]  shiftBits number of bits to shift.  A positive value shifts left; a negative value shifts right.        
 * @param[out]  *pDst points to the output vector        
 * @param[in]  blockSize number of samples in the vector        
 * @return none.        
 *    
 * \par Conditions for optimum performance    
 *  Input and output buffers should be aligned by 32-bit    
 *    
 *        
 * <b>Scaling and Overflow Behavior:</b>        
 * \par        
 * The function uses saturating arithmetic.        
 * Results outside of the allowable Q7 range [0x8 0x7F] will be saturated.        
 */

void riscv_shift_q7(
  q7_t * pSrc,
  int8_t shiftBits,
  q7_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */
  uint8_t sign;                                  /* Sign of shiftBits */


  /* Getting the sign of shiftBits */
  sign = (shiftBits & 0x80);

#if defined (USE_DSP_RISCV)

  charV *VectInA;
  charV VectInC,VectInB; 
  blkCnt = blockSize >> 2u;
  if(sign == 0u)
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (q7_t) clip(((q15_t) * pSrc++ << shiftBits), -128 , 127);
      *pDst++ = (q7_t) clip(((q15_t) * pSrc++ << shiftBits), -128 , 127);
      *pDst++ = (q7_t) clip(((q15_t) * pSrc++ << shiftBits), -128 , 127);
      *pDst++ = (q7_t) clip(((q15_t) * pSrc++ << shiftBits), -128 , 127);
      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {

    /* Initialize blkCnt with number of samples */
    VectInB = pack4(-shiftBits,-shiftBits,-shiftBits,-shiftBits);
    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      VectInA = (charV*)pSrc; 
      VectInC = sra4(*VectInA,VectInB); 
      *(charV*)pDst =  VectInC;
      pDst+=4;
      pSrc+=4;
     
      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  blkCnt = blockSize % 0x4u;
#else
    blkCnt = blockSize;
#endif
  /* If the shift value is positive then do right shift else left shift */
  if(sign == 0u)
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A << shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (q7_t) __SSAT(((q15_t) * pSrc++ << shiftBits), 8);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }
  else
  {
    /* Initialize blkCnt with number of samples */
    while(blkCnt > 0u)
    {
      /* C = A >> shiftBits */
      /* Shift the input and then store the result in the destination buffer. */
      *pDst++ = (*pSrc++ >> -shiftBits);

      /* Decrement the loop counter */
      blkCnt--;
    }
  }

}

/**        
 * @} end of shift group        
 */
