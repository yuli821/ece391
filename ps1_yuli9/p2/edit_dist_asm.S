/*
 * tab:2
 *
 * edit_dist_asm.S - inplementation of computing edit distance between two (linked) lists
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
 * Author:        Xiang Li
 * Version:       1
 * Creation Date: Sat Aug 20 2022
 * History:
 *    XL    1    Sat Aug 20 2022
 *        First written.
 */

.data
  UNUSED  = 0
  VAL   = 4
  NEXT = 8

.text
.global edit_dist_asm

# Edit Dist ASM (Assembly)
# Declaration is in edit_dist.h
#
# Registers:
#   EAX - Return Value
#   EBX - root1
#   BSI - root2
#   you may choose how to store your local/temp variables
#

edit_dist_asm:
  pushl %ebp
  movl %esp, %ebp

  # --- STUDENT CODE STARTS HERE ---
  subl $12, %esp   #leave space for local variables
  pushl %ebx       #caller stack frame, initialize ebx esi to root1, root2
  pushl %esi
  pushl %edi
  movl 8(%ebp), %ebx
  movl 12(%ebp), %esi
  
  cmpl $0, %ebx   
  jne ROOT1NOTZERO   #if root1 != 0
  cmpl $0, %esi
  je  RETURN0      #if root1 == 0 && root2 == 0 return 0
  jmp RETURN1      #if root1 == 0 && root2 != 0 
                   #return 1+edit_dist_c(root1, root2->next)
ROOT1NOTZERO:
  cmpl $0, %esi
  je RETURN2       #return 1+edit_dist_c(root1->next, root2)
  
  movl 4(%ebx), %edi   #edi <- M[ebx+4], store root1->val to ecx
  cmpl %edi, 4(%esi)   #compare root1->val and root2->val
  je RETURN3           #return edit_dist_c(root->next,root2->next)

  pushl %esi           #calculate insert
  pushl 8(%ebx)
  call edit_dist_asm
  addl $8, %esp
  movl %eax, -4(%ebp)

  pushl 8(%esi)     #calculate delete
  pushl %ebx
  call edit_dist_asm
  addl $8, %esp
  movl %eax, -8(%ebp)

  pushl 8(%esi)     #calculate replace
  pushl 8(%ebx)
  call edit_dist_asm
  addl $8, %esp
  movl %eax, -12(%ebp)

  movl -4(%ebp), %edi        #store insert to ecx
  cmpl -8(%ebp), %edi        #if insert > delete
  jg ELSEIF
  cmpl -12(%ebp), %edi       #if insert > replace
  jg ELSEIF
  movl %edi, %eax
  addl $1, %eax
  jmp DONE

ELSEIF:
  movl -8(%ebp), %edi        #store delete to ecx
  cmpl -12(%ebp), %edi       #if delete > replace
  jg ELSE
  movl %edi, %eax
  addl $1, %eax
  jmp DONE

ELSE:
  movl -12(%ebp), %eax
  addl $1, %eax
  jmp DONE

RETURN1:
  pushl NEXT(%esi)
  pushl %ebx
  call edit_dist_asm
  addl $8, %esp
  addl $1, %eax
  jmp DONE

RETURN2:
  pushl %esi
  pushl NEXT(%ebx)
  call edit_dist_asm
  addl $8, %esp
  addl $1, %eax
  jmp DONE
 
RETURN3:
  pushl NEXT(%esi)
  pushl NEXT(%ebx)
  call edit_dist_asm
  addl $8, %esp
  jmp DONE

RETURN0:
  andl $0, %eax

DONE:
  popl %edi
  popl %esi
  popl %ebx
  movl %ebp, %esp
  
  # --- STUDENT CODE ENDS HERE ---

  leave
  ret

