
# mp1.S - missile-command version
# Good luck, and have fun!

.data
	
    # Constants for accessing the fields of a struct missile, 
    # struct missile is defined in rtc.h and mp1.h

	NEXT 	  = 0
	X	  = 4
	Y	  = 8
	VX	  = 12
	VY	  = 16
	DEST_X	  = 20
	DEST_Y	  = 24
	EXPLODED  = 28
	C	  = 32

    # Character to draw for an explosion - '@'

	EXPLOSION = 64


# Data shared between this file and rtc.c helper functions

# This '.globl' directive makes these symbols visible externally
.globl mp1_missile_list, base_alive, mp1_score

mp1_missile_list:	.long 0x0	# Head of list pointer
base_alive:		.long 0x0	# Array of 3 bytes, plus a padding byte
mp1_score:		.long 0x0	# Player's current score


# Data private to this  file

base_pic:     	    .string "/^^^\\"	# Picture of a live base
dead_base_pic: 	    .string "xxxxx"	# Picture of a dead base
crosshairs_x:		.long 0x0	# X-position of the crosshairs
crosshairs_y:		.long 0x0	# Y-position of the crosshairs

	

.text

# void mp1_poke(void);
# You can use this function to write to video memory.
#
# Interface: Register-based arguments (not C-style)
#    Inputs: %cl  - The byte you wish to write
#            %eax - Offset from the start of video memory that you wish
#                   to write to
#   Outputs: Text-mode video screen is written to at location %eax with
#            the byte in %cl
# Registers: Clobbers EDX

mp1_poke:
		movl    vmem_base_addr(,1),%edx
		movb    %cl,(%edx,%eax,1)
		ret

#   ----------------- Exported functions ---------------------

# void mp1_rtc_tasklet(unsigned long garbage);
# Performs three tasks:
#	(1) updates the list of missiles (implement this in update_missiles,
#           below, and call it from here).
#	(2) Redraw the bases - they may have been overwritten with missiles
#	(3) Redraw the crosshairs - it may have been overwritten with missiles
# Inputs   : none
# Outputs  : none
# Registers: Standard C calling convention

.globl mp1_rtc_tasklet
mp1_rtc_tasklet:
		pushl %ebp
    	movl %esp, %ebp            		#save ebp
	                              	 	#variable, callee-saved regs
		pushl %ecx        				#caller-saved regs
		#pushl mp1_missile_list      	#head of the linked-list, argument of update missiles
		call update_missiles
		#addl $4, %esp
		popl %ecx

		pushl %ecx                     #caller saved reg, temp reg
		pushl %edx
		call draw_bases
		popl %edx
		popl %ecx

		call draw_crosshairs

		leave
		ret

# int mp1_ioctl(unsigned long arg, unsigned int cmd)
# The dispatch function for the MP1 ioctls - should use the cmd argument
# and a jumptable to execute one of the specific ioctls implemented below.
# Inputs   : unsigned long arg - parameter to the mp1_ioctl_....
#          : unsigned int cmd  - specifies which mp1_ioctl_... function 
#          :                     to execute
# Outputs  : Returns an integer - depends on which ioctl() function is called
# Registers: Standard C calling convention

.globl mp1_ioctl
mp1_ioctl:
		cmpl $0, 8(%esp)             # if cmd < 0 or cmd > 4, not valid, return -1
		jl INVALID
		cmpl $4, 8(%esp)
		jg INVALID
		movl 8(%esp),%eax
		jmp *jump_table(,%eax,4)
INVALID: 
		movl $-1, %eax
		ret


# ----------------- Functions private to this file -------------------

# ebx-curr pointer; edi-prev pointer; esi-temp; ecx-temp
update_missiles:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %ebx                  #callee saved reg, store current pointer
		pushl %edi                  #edi-prev pointer
		pushl %esi                  #esi-temp reg
		movl mp1_missile_list, %ebx #function interface, store the head of linked list to ebx
		movl %ebx, %edi             # prev = head

LOOP_UPDATE:
		cmpl $0, %ebx               #if current == 0
		je DONE_UPDATE

		#check if it's currently exploding
		cmpl $0, EXPLODED(%ebx)     #compage missile.exploded with 0
		jne EXPLODING               #if missile.exploded != 0, missile is exploding

		#update the position
		movl VX(%ebx), %esi         #store the velocity in x in esi
		addl %esi, X(%ebx)          #store x + vx into x, update x location

		movl VY(%ebx), %esi         #update y
		addl %esi, Y(%ebx)

		#check if moving out of the screen
		movl X(%ebx), %esi          #store the x location to esi
		sarl $16, %esi              #right shift 16 bits of x value, get the screen location, reserve the signed bit
		cmpl $0, %esi               #if x < 0 || x > 79, out of screen
		jl REMOVE_MISSILE
		cmpl $79, %esi
		jg REMOVE_MISSILE

		movl Y(%ebx), %esi          #repeat the check for y
		sarl $16, %esi
		cmpl $0, %esi
		jl REMOVE_MISSILE
		cmpl $24, %esi
		jg REMOVE_MISSILE

		#check whether reaches destination
		movl X(%ebx), %esi
		sarl $16, %esi
		cmpl DEST_X(%ebx), %esi     #if x != dest_x, draw the missile in the new location
		jne UPDATE_POSITION         #redraw the missile in the new location
		movl Y(%ebx), %esi
		sarl $16, %esi
		cmpl DEST_Y(%ebx), %esi     #if x == dest_x && y == dest_y, exploding
		je EXPLODING

UPDATE_POSITION:
		#movl X(%ebx), %esi          #store current x location to esi
		#subl VX(%ebx), %esi         #subtract velocity from current x location
		#sarl $16, %esi              #right shift 16 bits of x value, get the previous screen location, reserve the signed bit
		#movl X(%ebx), %ecx
		#sarl $16, %ecx              #current screen location in ecx

		#cmpl %esi, %ecx
		#jne CLEAR_MISSILE

		#movl Y(%ebx), %esi          #store current y location to esi
		#subl VY(%ebx), %esi         #subtract velocity from current y location
		#sarl $16, %esi              #right shift 16 bits of y value, get the previous screen location, reserve the signed bit
		#movl Y(%ebx), %ecx
		#sarl $16, %ecx              #current screen location in ecx
		
		#cmpl %esi, %ecx
		#je DRAW_SCREEN

#CLEAR_MISSILE:
		movb $32, %cl               #draw the missile with space
		movl Y(%ebx), %esi
		subl VY(%ebx), %esi
		sarl $16, %esi
		imull $160, %esi, %eax      #store 160 * y in eax
		movl X(%ebx), %esi         #get the previous x position
		subl VX(%ebx), %esi
		sarl $16, %esi
		shll $1, %esi               #store 2 * x in esi
		addl %esi, %eax             #add esi and eax, store the sum in eax
		pushl %edx                  #caller saved reg
		call mp1_poke
		popl %edx

#DRAW_SCREEN:
		movb C(%ebx), %cl
		movl Y(%ebx), %esi
		sarl $16, %esi				#get the current y position
		imull $160, %esi, %eax      #store 160 * y in eax
		movl X(%ebx), %esi
		sarl $16, %esi				#get the current x position
		shll $1, %esi               #store 2 * x in esi
		addl %esi, %eax             #add esi and eax, store the sum in eax
		pushl %edx                  #caller saved reg
		call mp1_poke
		popl %edx

LOOP_NEXT:
		cmpl $0, %ebx
		je DONE_UPDATE
		movl %ebx, %edi             #prev <- current
		movl NEXT(%ebx), %ebx       #move to the next element in the linked list, current <- current->next
		jmp LOOP_UPDATE

EXPLODING:
		pushl %ecx					#caller saved reg
		pushl %edx
		pushl %ebx                  #argument, pointer to the missile
		call missile_explode        #return value in eax
		addl $4, %esp
		popl %edx
		popl %ecx

		cmpl $0, %eax              #missile_explode, check if return 0
		je WITHOUT_NOTIFY
		pushl %eax
		pushl %edx
		pushl %ecx
		call mp1_notify_user
		popl %ecx
		popl %edx
		popl %eax

WITHOUT_NOTIFY:
		subl $1, EXPLODED(%ebx)         #decrement exploded

		movb $32, %cl               #draw the missile with space
		movl Y(%ebx), %esi
		sarl $16, %esi
		imull $160, %esi, %eax      #store 160 times y in eax
		movl X(%ebx), %esi
		sarl $16, %esi
		shll $1, %esi               #store 2 times x in esi
		addl %esi, %eax             #add esi and eax, store the sum in eax
		pushl %edx                  #caller saved reg
		call mp1_poke
		popl %edx

		cmpl $0, EXPLODED(%ebx)               #check if exploded equals 0
		jle REMOVE_MISSILE

		movb $64, %cl         #draw explosion
		movl Y(%ebx), %esi
		sarl $16, %esi				#get the current y position
		imull $160, %esi, %eax      #store 160 times y in eax
		movl X(%ebx), %esi
		sarl $16, %esi				#get the current x position
		shll $1, %esi               #store 2 times x in esi
		addl %esi, %eax             #add esi and eax, store the sum in eax
		pushl %edx                  #caller saved reg
		call mp1_poke
		popl %edx
		jmp LOOP_NEXT

REMOVE_MISSILE:
		#call the missile_remove function to remove current node, missile_remove(curr, prev)
		pushl %edi#prev
		pushl %ebx                  #push prev and curr as function arguments
		call mp1_missile_remove
		addl $8, %esp
		jmp LOOP_NEXT

DONE_UPDATE:
		popl %esi
		popl %edi
		popl %ebx
		movl %ebp, %esp
		popl %ebp
		ret

# input: current pointer, previous pointer
mp1_missile_remove:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %edi                  #ebx-pointer to the node that needs to be removed, edi-previous node
		pushl %esi                  #temp
		movl 8(%ebp), %ebx          #function interface, store the address to ebx
		movl 12(%ebp), %edi         #store prev pointer to edi

		movb $32, %cl               #draw the missile with space
		movl Y(%ebx), %esi
		subl VY(%ebx), %esi
		sarl $16, %esi
		imull $160, %esi, %eax      #store 160 * y in eax
		movl X(%ebx), %esi         #get the previous x position
		subl VX(%ebx), %esi
		sarl $16, %esi
		shll $1, %esi               #store 2 * x in esi
		addl %esi, %eax             #add esi and eax, store the sum in eax
		pushl %edx                  #caller saved reg
		call mp1_poke
		popl %edx

		cmpl mp1_missile_list, %ebx # compare the current pointer with the head pointer, if equal, remove from head
		jne REMOVE_MIDDLE           # if not equal, remove from the middle of the linked list
		movl NEXT(%ebx), %esi       # temp <- curr->next, which will be the new head
		movl %esi, mp1_missile_list # head <- curr->next
		jmp FREE

REMOVE_MIDDLE:
		movl NEXT(%ebx), %esi       #esi <- curr->next,
		movl %esi, NEXT(%edi)       #edi->next <- curr->next

FREE:
		#free the node which is pointed by ebx, call mp1_free
		pushl %eax
		pushl %ecx
		pushl %edx                  #caller-saved register
		pushl %ebx                  #pointer that needs to be deleted
		call mp1_free               #call the free function
		addl $4, %esp               #delete the argument, which is the pointer
		popl %edx
		popl %ecx
		popl %eax

		movl %esi, %ebx             #replace the next poitner to the current pointer

		popl %esi
		popl %edi
		movl %ebp, %esp
		popl %ebp
		ret

# draw the three bases based on base_alive array
# ecx: inner loop counter, edi: outer loop counter
# esi: temp
draw_bases:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %edi					#callee saved reg
		pushl %esi
		pushl %ebx

		movl $4, %ebx               #loop counter
		cmpb $0, base_alive     #compare base_alive[0] with 0
		je DRAW_DEAD_LOOP_ONE
		jmp DRAW_ALIVE_LOOP_ONE
COMPARE_TWO:
		movl $4, %ebx
		movl $1, %esi
		cmpb $0, base_alive(%esi)
		je DRAW_DEAD_LOOP_TWO
		jmp DRAW_ALIVE_LOOP_TWO
COMPARE_THREE:
		movl $4, %ebx
		movl $2, %esi
		cmpb $0, base_alive(%esi)
		je DRAW_DEAD_LOOP_THREE
		jmp DRAW_ALIVE_LOOP_THREE
#draw 1st alive base
DRAW_ALIVE_LOOP_ONE:
		cmpl $-1, %ebx
		je COMPARE_TWO
		movb base_pic(%ebx), %cl      #store dead_base_pic[ecx] into %cl, which is dead_base_pic plus offset
		movl $3840, %eax            #3840 = 24 * 2 * 80, the base always locates at row 24
		movl $18, %edi
		addl %ebx, %edi             #edi <- 18+loop counter
		shll $1, %edi
		addl %edi, %eax             #3840 + 2 * (18+loop counter)
		pushl %edx
		call mp1_poke               #draw the character
		popl %edx
		decl %ebx
		jmp DRAW_ALIVE_LOOP_ONE
	
#draw 2nd alive base
DRAW_ALIVE_LOOP_TWO:
		cmpl $-1, %ebx
		je COMPARE_THREE
		movb base_pic(%ebx), %cl      #store dead_base_pic[ecx] into %cl, which is dead_base_pic plus offset
		movl $3840, %eax            #3840 = 24 * 2 * 80, the base always locates at row 24
		movl $38, %edi
		addl %ebx, %edi             #edi <- 38+loop counter
		shll $1, %edi
		addl %edi, %eax             #3840 + 2 * (38+loop counter)
		pushl %edx
		call mp1_poke               #draw the character
		popl %edx
		decl %ebx
		jmp DRAW_ALIVE_LOOP_TWO

#draw 3rd alive base
DRAW_ALIVE_LOOP_THREE:
		cmpl $-1, %ebx
		je LOOP_END
		movb base_pic(%ebx), %cl      #store dead_base_pic[ecx] into %cl, which is dead_base_pic plus offset
		movl $3840, %eax            #3840 = 24 * 2 * 80, the base always locates at row 24
		movl $58, %edi
		addl %ebx, %edi             #edi <- 58+loop counter
		shll $1, %edi
		addl %edi, %eax             #3840 + 2 * (58+loop counter)
		pushl %edx
		call mp1_poke               #draw the character
		popl %edx
		decl %ebx
		jmp DRAW_ALIVE_LOOP_THREE

#draw 1st dead base
DRAW_DEAD_LOOP_ONE:
		cmpl $-1, %ebx
		je COMPARE_TWO
		movb dead_base_pic(%ebx), %cl      #store dead_base_pic[ecx] into %cl, which is dead_base_pic plus offset
		movl $3840, %eax            #3840 = 24 * 2 * 80, the base always locates at row 24
		movl $18, %edi
		addl %ebx, %edi             #edi <- 18+loop counter
		shll $1, %edi
		addl %edi, %eax             #3840 + 2 * (18+loop counter)
		pushl %edx
		call mp1_poke               #draw the character
		popl %edx
		decl %ebx
		jmp DRAW_DEAD_LOOP_ONE

#draw 2nd dead base
DRAW_DEAD_LOOP_TWO:
		cmpl $-1, %ebx
		je COMPARE_THREE
		movb dead_base_pic(%ebx), %cl      #store dead_base_pic[ecx] into %cl, which is dead_base_pic plus offset
		movl $3840, %eax            #3840 = 24 * 2 * 80, the base always locates at row 24
		movl $38, %edi
		addl %ebx, %edi             #edi <- 38+loop counter
		shll $1, %edi
		addl %edi, %eax             #3840 + 2 * (38+loop counter)
		pushl %edx
		call mp1_poke               #draw the character
		popl %edx
		decl %ebx
		jmp DRAW_DEAD_LOOP_TWO

#draw 3rd dead base
DRAW_DEAD_LOOP_THREE:
		cmpl $-1, %ebx
		je LOOP_END
		movb dead_base_pic(%ebx), %cl      #store dead_base_pic[ecx] into %cl, which is dead_base_pic plus offset
		movl $3840, %eax            #3840 = 24 * 2 * 80, the base always locates at row 24
		movl $58, %edi
		addl %ebx, %edi             #edi <- 58+loop counter
		shll $1, %edi
		addl %edi, %eax             #3840 + 2 * (58+loop counter)
		pushl %edx
		call mp1_poke               #draw the character
		popl %edx
		decl %ebx
		jmp DRAW_DEAD_LOOP_THREE

LOOP_END:
		popl %ebx
		popl %esi
		popl %edi
		leave
		ret

draw_crosshairs:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %ebx                  #callee saved reg
		pushl %esi  				#ebx-x position; esi-y position

		movb $43, %cl               #%cl <- "+"
		movl crosshairs_x, %ebx
		movl crosshairs_y, %esi
		imull $160, %esi, %eax      # eax <- 2 * 80 * y
		shll $1, %ebx               # x * 2
		addl %ebx, %eax             # eax <- 2 * 80 * y + 2 * x
		pushl %edx
		call mp1_poke
		popl %edx

		popl %esi
		popl %ebx
		leave
		ret
		
#set base_alive, crosshair_x, crosshair_y
mp1_ioctl_startgame:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %ebx
		pushl %esi
		movb $1, base_alive(,1)     #base_alive[0] = 1
		movl $1, %esi
		movb $1, base_alive(%esi)   #base_alive[1] = 1
		movl $2, %esi
		movb $1, base_alive(%esi)   #base_alive[2] = 1

		movl $40, crosshairs_x  #crosshairs_x = 40
		movl $12, crosshairs_y  #crosshairs_y = 12

		popl %esi
		popl %ebx
		movl %ebp, %esp             #stack clean-up
		popl %ebp
		ret

mp1_ioctl_addmissile:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %ebx
		pushl %esi                  #callee-saved regs
		pushl %edi

		pushl %ecx					#caller saved
		pushl %edx
		pushl $36                   #number of bytes needed
		call mp1_malloc
		addl $4, %esp
		popl %edx
		popl %ecx

		cmpl $0, %eax               # if return a null pointer, error
		je FALSE
		movl %eax, %esi             # store the pointer of the allocated memory to esi

		pushl %ecx					#caller saved
		pushl %edx
		pushl $36                   # number of bytes needed
		pushl 8(%ebp)               # from pointer - the user space
		pushl %esi                  # to pointer - output from mp1_malloc
		call mp1_copy_from_user
		addl $12, %esp
		popl %edx
		popl %ecx

		cmpl $0, %eax               #if return is not 0, error
		jne FALSE_COPY

		movl mp1_missile_list, %edi
		movl %edi, NEXT(%esi)
		movl %esi, mp1_missile_list
		movl $0, %eax
		jmp DONE_ADD

FALSE_COPY:
		pushl %eax
		pushl %ecx
		pushl %edx                  #caller-saved register
		pushl %esi                  #pointer that needs to be deleted
		call mp1_free               #call the free function
		addl $4, %esp               #delete the argument, which is the pointer
		popl %edx
		popl %ecx
		popl %eax
FALSE:
		movl $-1, %eax
DONE_ADD:
		popl %edi
		popl %esi
		popl %ebx
		movl %ebp, %esp             #stack clean-up
		popl %ebp
		ret

mp1_ioctl_movexhairs:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %ebx
		pushl %edi                  # y offset, callee saved reg
		pushl %esi                  # x offset

		imull $160, crosshairs_y, %eax
		imull $2, crosshairs_x, %ebx
		addl %ebx, %eax
		movb $32, %cl
		pushl %edx
		call mp1_poke
		popl %edx

		xorl %edi, %edi
		xorl %esi, %esi
		movl 8(%ebp), %edi          # store the xhair_delta_packed into edi
		sarl $16, %edi              # logical right shift 16 bits to get the y offset
		movl 8(%ebp), %esi
		shll $16, %esi
		sarl $16, %esi              #get the lower 16 bits for the x offset

		addl crosshairs_x, %esi     # esi <- x + offset
		addl crosshairs_y, %edi     # edi <- y + offset
		
		cmpl $0, %esi               #check the range of x
		jl X_LESS
		cmpl $79, %esi
		jg X_GREATER
		movl %esi, crosshairs_x
CHECK_Y:
		cmpl $0, %edi               #check the range of y
		jl Y_LESS
		cmpl $24, %edi
		jg Y_GREATER
		movl %edi, crosshairs_y
		jmp REDRAW_CROSS
X_LESS:
		movl $0, crosshairs_x
		jmp CHECK_Y
X_GREATER:
		movl $79, crosshairs_x
		jmp CHECK_Y
Y_LESS:
		movl $0, crosshairs_y
		jmp REDRAW_CROSS
Y_GREATER:
		movl $24, crosshairs_y
REDRAW_CROSS:
		movb $43, %cl
		movl crosshairs_x, %esi               #2 * x
		shll $1, %esi
		imull $160, crosshairs_y, %eax      #y * 2 * 80
		addl %esi, %eax             #eax <- 2 * 80 * y + 2 * x
		pushl %edx
		call mp1_poke               #draw the "+" at the current screen position
		popl %edx

		movl $0, %eax

		popl %esi
		popl %edi
		popl %ebx
		movl %ebp, %esp             #stack clean-up
		popl %ebp
		ret

mp1_ioctl_getstatus:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %ebx
		pushl %edi
		pushl %esi

		#pushl %ecx					#caller saved
		#pushl %edx
		#pushl $4                    #number of bytes needed
		#call mp1_malloc             #dynamically allocate 4-bytes memory to store the integer
		#addl $4, %esp
		#popl %edx
		#popl %ecx

		#cmpl $0, %eax               #check if null
		#je DONE_ERROR

		movl mp1_score, %esi        #copy the m[mp1_score] to esi
		andl $0x0000ffff, %esi   # clear the upper 16 bits of mp1_score

		cmpb $0, base_alive         #check if the first base died
		jne FIRST_ALIVE
CHECK_SECOND:
		movl $1, %edi
		cmpb $0, base_alive(%edi)  #check if the second base died
		jne SECOND_ALIVE
CHECK_THIRD:
		movl $2, %edi
		cmpb $0, base_alive(%edi)  #check if the third base died
		jne THIRD_ALIVE
		jmp DONE_CHECK

FIRST_ALIVE:
		addl $0x00010000, %esi   #set the bit 16 to 1
		jmp CHECK_SECOND
SECOND_ALIVE:
		addl $0x00020000, %esi   #set the bit 17 to 1
		jmp CHECK_THIRD
THIRD_ALIVE:
		addl $0x00040000, %esi   #set the bit 18 to 1
DONE_CHECK:
		pushl %esi               #push to the stack
		movl %esp, %edi
		#copy_to_user
		pushl %ecx					#caller saved
		pushl %edx
		pushl $4                    # number of bytes needed
		pushl %edi                  # from pointer - output from mp1_malloc
		pushl 8(%ebp)               # to pointer - userspace
		call mp1_copy_to_user
		addl $12, %esp
		popl %edx
		popl %ecx
		popl %esi
		
		cmpl $0, %eax               #check if copy is success
		je DONE_COPY                #error if not 0
DONE_ERROR:
		movl $-1, %eax
DONE_COPY:
		popl %esi
		popl %edi
		popl %ebx
		movl %ebp, %esp             #stack clean-up
		popl %ebp
		ret

mp1_ioctl_endgame:
		pushl %ebp
    	movl %esp, %ebp             #stack frame
		pushl %ebx
		pushl %esi
		movl mp1_missile_list, %ebx
LOOP_CLEAR:
		cmpl $0, %ebx               #check if curr pointer is null
		je DONE_CLEAR
		movl %ebx, %esi             #copy the pointer to a temp
		movl NEXT(%ebx), %ebx       #curr <- curr->next

		pushl %eax                  #free the curr node
		pushl %ecx
		pushl %edx                  #caller-saved register
		pushl %esi                  #pointer that needs to be deleted
		call mp1_free               #call the free function
		addl $4, %esp               #delete the argument, which is the pointer
		popl %edx
		popl %ecx
		popl %eax

		jmp LOOP_CLEAR

DONE_CLEAR:
		movl $0, %eax               #return success
		popl %esi
		popl %ebx
		movl %ebp, %esp             #stack clean-up
		popl %ebp
		ret

jump_table:
		.long mp1_ioctl_startgame, mp1_ioctl_addmissile, mp1_ioctl_movexhairs, mp1_ioctl_getstatus, mp1_ioctl_endgame

