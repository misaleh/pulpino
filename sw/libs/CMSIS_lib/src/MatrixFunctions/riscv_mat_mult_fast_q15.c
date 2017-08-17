/* ----------------------------------------------------------------------    
* Copyright (C) 2010-2014 ARM Limited. All rights reserved.    
*    
* $Date:        19. March 2015
* $Revision: 	V.1.4.5
*    
* Project: 	    CMSIS DSP Library    
* Title:	    arm_mat_mult_fast_q15.c    
*    
* Description:	 Q15 matrix multiplication (fast variant)    
*    
* Target Processor: Cortex-M4/Cortex-M3
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
 * @ingroup groupMatrix    
 */

/**    
 * @addtogroup MatrixMult    
 * @{    
 */


/**    
 * @brief Q15 matrix multiplication (fast variant) for Cortex-M3 and Cortex-M4    
 * @param[in]       *pSrcA points to the first input matrix structure    
 * @param[in]       *pSrcB points to the second input matrix structure    
 * @param[out]      *pDst points to output matrix structure    
 * @param[in]		*pState points to the array for storing intermediate results    
 * @return     		The function returns either    
 * <code>RISCV_MATH_SIZE_MISMATCH</code> or <code>RISCV_MATH_SUCCESS</code> based on the outcome of size checking.    
 *    
 * @details    
 * <b>Scaling and Overflow Behavior:</b>    
 *    
 * \par    
 * The difference between the function riscv_mat_mult_q15() and this fast variant is that    
 * the fast variant use a 32-bit rather than a 64-bit accumulator.    
 * The result of each 1.15 x 1.15 multiplication is truncated to        
 * 2.30 format. These intermediate results are accumulated in a 32-bit register in 2.30        
 * format. Finally, the accumulator is saturated and converted to a 1.15 result.        
 *        
 * \par        
 * The fast version has the same overflow behavior as the standard version but provides        
 * less precision since it discards the low 16 bits of each multiplication result.        
 * In order to avoid overflows completely the input signals must be scaled down.        
 * Scale down one of the input matrices by log2(numColsA) bits to        
 * avoid overflows, as a total of numColsA additions are computed internally for each        
 * output element.        
 *        
 * \par    
 * See <code>riscv_mat_mult_q15()</code> for a slower implementation of this function    
 * which uses 64-bit accumulation to provide higher precision.    
 */

riscv_status riscv_mat_mult_fast_q15(
  const riscv_matrix_instance_q15 * pSrcA,
  const riscv_matrix_instance_q15 * pSrcB,
  riscv_matrix_instance_q15 * pDst,
  q15_t * pState)
{
  q31_t sum;                                     /* accumulator */
  q15_t *pSrcBT = pState;                        /* input data matrix pointer for transpose */
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A of Q15 type */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B of Q15 type */
  q15_t *px;                                     /* Temporary output data matrix pointer */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint16_t numRowsB = pSrcB->numRows;            /* number of rows of input matrix A    */
  uint16_t col, i = 0u, row = numRowsB, colCnt;  /* loop counters */
  riscv_status status;                             /* status of matrix multiplication */


  q15_t in;                                      /* Temporary variable to hold the input value */
  q15_t inA1, inA2, inB1, inB2;
  shortV *VectInA;
  shortV *VectInB; 
#ifdef RISCV_MATH_MATRIX_CHECK
  /* Check for matrix mismatch condition */
  if((pSrcA->numCols != pSrcB->numRows) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcB->numCols != pDst->numCols))
  {
    /* Set status as RISCV_MATH_SIZE_MISMATCH */
    status = RISCV_MATH_SIZE_MISMATCH;
  }
  else
#endif
  {
    /* Matrix transpose */
    do
    {
      /* Apply loop unrolling and exchange the columns with row elements */
      col = numColsB >> 2;

      /* The pointer px is set to starting address of the column being processed */
      px = pSrcBT + i;

      /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.        
       ** a second loop below computes the remaining 1 to 3 samples. */
      while(col > 0u)
      {
        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;
 
        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;
 
        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;
 
        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Read one element from the row */
        in = *pInB++;

        /* Store one element in the destination */
        *px = in;
        
		/* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Decrement the column loop counter */
        col--;
      }

      /* If the columns of pSrcB is not a multiple of 4, compute any remaining output samples here.        
       ** No loop unrolling is used. */
      col = numColsB % 0x4u;

      while(col > 0u)
      {
        /* Read and store the input element in the destination */
        *px = *pInB++;

        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB;

        /* Decrement the column loop counter */
        col--;
      }

      i++;

      /* Decrement the row loop counter */
      row--;

    } while(row > 0u);

    /* Reset the variables for the usage in the following multiplication process */
    row = numRowsA;
    i = 0u;
    px = pDst->pData;

    /* The following loop performs the dot-product of each row in pSrcA with each column in pSrcB */
    /* row loop */
    do
    {
      /* For every row wise process, the column loop counter is to be initiated */
      col = numColsB;

      /* For every row wise process, the pIn2 pointer is set        
       ** to the starting address of the transposed pSrcB data */
      pInB = pSrcBT;

      /* column loop */
      do
      {
        /* Set the variable sum, that acts as accumulator, to zero */
        sum = 0;

        /* Apply loop unrolling and compute 2 MACs simultaneously. */
        colCnt = numColsA >> 2;

        /* Initiate the pointer pIn1 to point to the starting address of the column being processed */
        pInA = pSrcA->pData + i;

        /* matrix multiplication */
        while(colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */

#if defined (USE_DSP_RISCV)
        /*  inA1 = *pInA++;
          inB1 = *pInB++;
          sum =  mac(inA1,inB1,sum);
          inA2 = *pInA++;
          inB2 = *pInB++;
          sum =  mac(inA2,inB2,sum);
          inA1 = *pInA++;
          inB1 = *pInB++;
          sum =  mac(inA1,inB1,sum);
          inA2 = *pInA++;
          inB2 = *pInB++;
          sum =  mac(inA2,inB2,sum);
*/
          VectInA = (shortV*)pInA;
          VectInB = (shortV*)pInB;


          sum = sumdotpv2(*VectInA,*VectInB,sum);
          VectInA = (shortV*)(pInA+2);
          VectInB = (shortV*)(pInB+2);

          pInA+=4;
          pInB+=4;
          sum = sumdotpv2(*VectInA,*VectInB,sum);
#else
          inA1 = *pInA++;
          inB1 = *pInB++;
          inA2 = *pInA++;
          sum += inA1 * inB1;
          inB2 = *pInB++;

          inA1 = *pInA++;
          inB1 = *pInB++;
          sum += inA2 * inB2;
          inA2 = *pInA++;
          inB2 = *pInB++;

          sum += inA1 * inB1;
          sum += inA2 * inB2;
#endif
          /* Decrement the loop counter */
          colCnt--;
        }

        /* process odd column samples */
        colCnt = numColsA % 0x4u;

        while(colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */
#if defined (USE_DSP_RISCV)
          sum =  mac((*pInA++),(*pInB++),sum);
#else
          sum += (q31_t) (*pInA++) * (*pInB++);
#endif
          colCnt--;
        }

        /* Saturate and store the result in the destination buffer */
        *px = (q15_t) (sum >> 15);
        px++;

        /* Decrement the column loop counter */
        col--;

      } while(col > 0u);

      i = i + numColsA;

      /* Decrement the row loop counter */
      row--;

    } while(row > 0u);

    /* set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;
  }

  /* Return to application */
  return (status);
}

/**        
 * @} end of MatrixMult group        
 */
