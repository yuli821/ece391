/*
 * tab:2
 *
 * mystery.c
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Yan Miao
 * Version:       1
 * Creation Date: Sun Aug 29 2021
 * Author:        Xiang Li
 * Version:       2
 * Modified Date: Sun Aug 21 2022
 * History:
 *    YM    1    Sun Aug 29 2021
 *    XL    2    Sun Aug 21 2022
 */

#include "mystery.h"

uint32_t mystery_c(uint32_t x, uint32_t y) {
  //------- YOUR CODE HERE -------
  int temp1, temp2, temp3;
  temp1 = 0;                         // MOVL $0, %EDI
  temp2 = 1;                         // MOVL $1, %ESI
  temp3 = 1;                         // MOVL $1, %EDX

  if (y > 24) {                      // compare 12(%EBP) with 24, jump to DONE1 if greater
    return 0;                        // DONE1 returns 0
  }

  if (x > 42 || x == 42) {          // compare 8(%EBP) with 42, jump to DONE1 if greater or eqaul
    return 0;                       // DONE1 returns 0
  }

  while (x > temp2 || x == temp2) { // LOOP1, loop until temp2 is greater than the first argument
    temp1 += temp2;                 // ADDL %ESI, %EDI
    temp2 ++;                       // INCL $1, $ESI
  }

  temp2 = 1;                        // set temp2 back to 1, MOVL $1, %ESI

  while (y > temp2 || y == temp2) { // LOOP2, loop until temp2 is greater than the second argument
    temp3 *= temp2;                 // IMULL %ESI, %EDI
    temp2 ++;                       // INCL %ESI
  }

  return temp1 | temp3;             // ORL %EDI, %EDX
                                    // MOVL %EDX, %EAX
  //------------------------------
}
