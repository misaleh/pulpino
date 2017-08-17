/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:		arm_offset_q15.c    
*    
* Description:	Q15 vector offset.    
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
 * @addtogroup offset    
 * @{    
 */

/**    
 * @brief  Adds a constant offset to a Q15 vector.    
 * @param[in]  *pSrc points to the input vector    
 * @param[in]  offset is the offset to be added    
 * @param[out]  *pDst points to the output vector    
 * @param[in]  blockSize number of samples in the vector    
 * @return none.    
 *    
 * <b>Scaling and Overflow Behavior:</b>    
 * \par    
 * The function uses saturating arithmetic.    
 * Results outside of the allowable Q15 range [0x8000 0x7FFF] are saturated.    
 */

void riscv_offset_q15(
  q15_t * pSrc,
  q15_t offset,
  q15_t * pDst,
  uint32_t blockSize)
{
  uint32_t blkCnt;                               /* loop counter */

#if defined (USE_DSP_RISCV)
  q31_t inA1, inA2, inB1, inB2;

  /*loop Unrolling */
  blkCnt = blockSize >> 1u;
  while (blkCnt > 0u)
  {
    /* C = A + B */
    /*read 2 elements from source buffer*/
    inA1 = *pSrc++;
    inA2 = *pSrc++;
    /*add the 2 elements to the offset value then saturate*/
    *pDst++ =(q15_t)clip((inA1 + offset),-32768,32767);
    *pDst++ =(q15_t)clip((inA2 + offset),-32768,32767);
    /* Decrement the loop counter */
    blkCnt--;
  }

  blkCnt = blockSize % 0x2u;

  while (blkCnt > 0u)
  {
    /* C = A + B */
    /* Add and then store the results in the destination buffer. */
    *pDst++ = (q15_t)clip((*pSrc++ + offset),-32768,32767);
    /* Decrement the loop counter */
    blkCnt--;
  }

#else

  /* Initialize blkCnt with number of samples */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = A + offset */
    /* Add offset and then store the results in the destination buffer. */
    *pDst++ = (q15_t) __SSAT(((q31_t) * pSrc++ + offset), 16);
    /* Decrement the loop counter */
    blkCnt--;
  }
#endif
}

/**    
 * @} end of offset group    
 */
