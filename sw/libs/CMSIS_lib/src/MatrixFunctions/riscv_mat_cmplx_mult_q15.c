/* ----------------------------------------------------------------------
 * Project:      CMSIS DSP Library
 * Title:        arm_cmplx_mat_mult_q15.c
 * Description:  Q15 complex matrix multiplication
 *
 * $Date:        27. January 2017
 * $Revision:    V.1.5.1
 *
 * Target Processor: Cortex-M cores
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2017 ARM Limited or its affiliates. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.

 Modifications 2017  Mostafa Saleh       (Ported to RISC-V PULPino)
 */

#include "riscv_math.h"

/**
 * @ingroup groupMatrix
 */

/**
 * @addtogroup CmplxMatrixMult
 * @{
 */


/**
 * @brief Q15 Complex matrix multiplication
 * @param[in]       *pSrcA points to the first input complex matrix structure
 * @param[in]       *pSrcB points to the second input complex matrix structure
 * @param[out]      *pDst points to output complex matrix structure
 * @param[in]		*pScratch points to the array for storing intermediate results
 * @return     		The function returns either
 * <code>ARM_MATH_SIZE_MISMATCH</code> or <code>ARM_MATH_SUCCESS</code> based on the outcome of size checking.
 *
 * \par Conditions for optimum performance
 *  Input, output and state buffers should be aligned by 32-bit
 *
 * \par Restrictions
 *  If the silicon does not support unaligned memory access enable the macro UNALIGNED_SUPPORT_DISABLE
 *	In this case input, output, scratch buffers should be aligned by 32-bit
 *
 * @details
 * <b>Scaling and Overflow Behavior:</b>
 *
 * \par
 * The function is implemented using a 64-bit internal accumulator. The inputs to the
 * multiplications are in 1.15 format and multiplications yield a 2.30 result.
 * The 2.30 intermediate
 * results are accumulated in a 64-bit accumulator in 34.30 format. This approach
 * provides 33 guard bits and there is no risk of overflow. The 34.30 result is then
 * truncated to 34.15 format by discarding the low 15 bits and then saturated to
 * 1.15 format.
 *
 * \par
 * Refer to <code>riscv_mat_mult_fast_q15()</code> for a faster but less precise version of this function.
 *
 */




riscv_status riscv_mat_cmplx_mult_q15(
  const riscv_matrix_instance_q15 * pSrcA,
  const riscv_matrix_instance_q15 * pSrcB,
  riscv_matrix_instance_q15 * pDst,
  q15_t * pScratch)
{
  /* accumulator */
  q15_t *pSrcBT = pScratch;                      /* input data matrix pointer for transpose */
  q15_t *pInA = pSrcA->pData;                    /* input data matrix pointer A of Q15 type */
  q15_t *pInB = pSrcB->pData;                    /* input data matrix pointer B of Q15 type */
  q15_t *px;                                     /* Temporary output data matrix pointer */
  uint16_t numRowsA = pSrcA->numRows;            /* number of rows of input matrix A    */
  uint16_t numColsB = pSrcB->numCols;            /* number of columns of input matrix B */
  uint16_t numColsA = pSrcA->numCols;            /* number of columns of input matrix A */
  uint16_t numRowsB = pSrcB->numRows;            /* number of rows of input matrix A    */
  uint16_t col, i = 0u, row = numRowsB, colCnt;  /* loop counters */
  riscv_status status;                             /* status of matrix multiplication */
  q63_t sumReal, sumImag;


  q15_t in;                                      /* Temporary variable to hold the input value */
  q15_t a, b, c, d;
#ifdef RISCV_MATH_MATRIX_CHECK
  /* Check for matrix mismatch condition */
  if ((pSrcA->numCols != pSrcB->numRows) ||
     (pSrcA->numRows != pDst->numRows) || (pSrcB->numCols != pDst->numCols))
  {
    /* Set status as RISCV_MATH_SIZE_MISMATCH */
    status = RISCV_MATH_SIZE_MISMATCH;
  }
  else
#endif
#if defined (USE_DSP_RISCV)
{
    shortV VectInA;
    shortV VectInB; 
    /* Matrix transpose */
    do
    {

      /* The pointer px is set to starting address of the column being processed */
      px = pSrcBT + i;


      col = numColsB;

      while (col > 0u)
      {
        /* Read two elements from the row */
        in = *pInB++;
        *px = in;
        in = *pInB++;
        px[1] = in;


        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB * 2;

        /* Decrement the column loop counter */
        col--;
      }

      i = i + 2u;

      /* Decrement the row loop counter */
      row--;

    } while (row > 0u);

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
        sumReal = 0;
        sumImag = 0;

        /* Apply loop unrolling and compute 2 MACs simultaneously. */
        colCnt = numColsA >> 1;

        /* Initiate the pointer pIn1 to point to the starting address of the column being processed */
        pInA = pSrcA->pData + i * 2;


        /* matrix multiplication */
        while (colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */



          /* read real and imag values from pSrcA buffer */
          a = *pInA;
          b = *(pInA + 1u);
          /* read real and imag values from pSrcB buffer */
          c = *pInB;
          d = *(pInB + 1u);
          VectInA[0] = a;
          VectInA[1] = -b;
          VectInB[0] = c;
          VectInB[1] = d;
          /* Multiply and Accumlates */
          sumReal += dotpv2(VectInA,VectInB);
          VectInA[1] = b;
          VectInB[0] = d;
          VectInB[1] = c;
          sumImag += dotpv2(VectInA,VectInB);
         /* sumReal += (q31_t) a *c;
          sumImag += (q31_t) a *d;
          sumReal -= (q31_t) b *d;
          sumImag += (q31_t) b *c;*/

          /* read next real and imag values from pSrcA buffer */
          a = *(pInA + 2u);
          b = *(pInA + 3u);
          /* read next real and imag values from pSrcB buffer */
          c = *(pInB + 2u);
          d = *(pInB + 3u);

          /* update pointer */
          pInA += 4u;

          /* Multiply and Accumlates */
          VectInA[0] = a;
          VectInA[1] = -b;
          VectInB[0] = c;
          VectInB[1] = d;
          /* Multiply and Accumlates */
          sumReal += dotpv2(VectInA,VectInB);
          VectInA[1] = b;
          VectInB[0] = d;
          VectInB[1] = c;
          sumImag += dotpv2(VectInA,VectInB);
          /* update pointer */
          pInB += 4u;


          /* Decrement the loop counter */
          colCnt--;
        }

        /* process odd column samples */
        if ((numColsA & 0x1u) > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */



          /* read real and imag values from pSrcA and pSrcB buffer */
          a = *pInA++;
          b = *pInA++;
          c = *pInB++;
          d = *pInB++;

          /* Multiply and Accumlates */
          VectInA[0] = a;
          VectInA[1] = -b;
          VectInB[0] = c;
          VectInB[1] = d;
          /* Multiply and Accumlates */
          sumReal += dotpv2(VectInA,VectInB);
          VectInA[1] = b;
          VectInB[0] = d;
          VectInB[1] = c;
          sumImag += dotpv2(VectInA,VectInB);

        }

        /* Saturate and store the result in the destination buffer */
        *px++ = (q15_t) (clip(sumReal >> 15, -32768,32767));
        *px++ = (q15_t) (clip(sumImag >> 15,-32768,32767));
        /* Decrement the column loop counter */
        col--;

      } while (col > 0u);

      i = i + numColsA;

      /* Decrement the row loop counter */
      row--;

    } while (row > 0u);

    /* set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;
}
#else

  {
    /* Matrix transpose */
    do
    {

      /* The pointer px is set to starting address of the column being processed */
      px = pSrcBT + i;


      col = numColsB;

      while (col > 0u)
      {
        /* Read two elements from the row */
        in = *pInB++;
        *px = in;
        in = *pInB++;
        px[1] = in;


        /* Update the pointer px to point to the next row of the transposed matrix */
        px += numRowsB * 2;

        /* Decrement the column loop counter */
        col--;
      }

      i = i + 2u;

      /* Decrement the row loop counter */
      row--;

    } while (row > 0u);

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
        sumReal = 0;
        sumImag = 0;

        /* Apply loop unrolling and compute 2 MACs simultaneously. */
        colCnt = numColsA >> 1;

        /* Initiate the pointer pIn1 to point to the starting address of the column being processed */
        pInA = pSrcA->pData + i * 2;


        /* matrix multiplication */
        while (colCnt > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */



          /* read real and imag values from pSrcA buffer */
          a = *pInA;
          b = *(pInA + 1u);
          /* read real and imag values from pSrcB buffer */
          c = *pInB;
          d = *(pInB + 1u);

          /* Multiply and Accumlates */
          sumReal += (q31_t) a *c;
          sumImag += (q31_t) a *d;
          sumReal -= (q31_t) b *d;
          sumImag += (q31_t) b *c;

          /* read next real and imag values from pSrcA buffer */
          a = *(pInA + 2u);
          b = *(pInA + 3u);
          /* read next real and imag values from pSrcB buffer */
          c = *(pInB + 2u);
          d = *(pInB + 3u);

          /* update pointer */
          pInA += 4u;

          /* Multiply and Accumlates */
          sumReal += (q31_t) a *c;
          sumImag += (q31_t) a *d;
          sumReal -= (q31_t) b *d;
          sumImag += (q31_t) b *c;
          /* update pointer */
          pInB += 4u;


          /* Decrement the loop counter */
          colCnt--;
        }

        /* process odd column samples */
        if ((numColsA & 0x1u) > 0u)
        {
          /* c(m,n) = a(1,1)*b(1,1) + a(1,2) * b(2,1) + .... + a(m,p)*b(p,n) */



          /* read real and imag values from pSrcA and pSrcB buffer */
          a = *pInA++;
          b = *pInA++;
          c = *pInB++;
          d = *pInB++;

          /* Multiply and Accumlates */
          sumReal += (q31_t) a *c;
          sumImag += (q31_t) a *d;
          sumReal -= (q31_t) b *d;
          sumImag += (q31_t) b *c;

        }

        /* Saturate and store the result in the destination buffer */

        *px++ = (q15_t) (__SSAT(sumReal >> 15, 16));
        *px++ = (q15_t) (__SSAT(sumImag >> 15, 16));

        /* Decrement the column loop counter */
        col--;

      } while (col > 0u);

      i = i + numColsA;

      /* Decrement the row loop counter */
      row--;

    } while (row > 0u);

    /* set status as RISCV_MATH_SUCCESS */
    status = RISCV_MATH_SUCCESS;
  }
#endif

  /* Return to application */
  return (status);
}

