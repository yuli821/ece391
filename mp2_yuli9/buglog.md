9/27  
1. Bug: The split screen position is a lot higher than what I set in the line Compare field.  
Context: I set the register in line compare field to 182.  
Solution: Reading Piazza post, the line number stored in register needs to time 2. Because each line of pixels is drawn twice on two separate scan lines. VGA counts scan lines for comparison.

9/28  
1. Bug: The status bar doesn't show, and the top screen is blinking with vertical black line.  
Context: I called two functions in game_loop, fill_status_bar and show_status_bar, fill_status_bar copy the output of text_to_image to the buffer, and show_status_bar copy the bar to the memory (address 0).
Possible Cause: I didn't divide the status bar image to four planes, is that a requirement? Looks like I have to divide the planes.  
Solution: Solved it with dividing it into four planes.  

2. Bug: Status bar doesn't show any content. 
Possible Reason: Speculate the problem is in copy_status_bar when I copy the build buffer to address 0 in video memory.  
Debug: Tried to initialize the buffer with only one color without calling text_to_img, the status bar still doesn't show anything, which means the problem is inside the buffer or the copy_status bar function.  
Solution: Problem solved. The problem is not with the copy_status_bar, the initialization of the build_text is not sufficient. I only initialize the last elemnt in the build buffer, and the initialization doesn't cover the entire buffer. After I initialize the buffer inside the function with a for loop, the color showed up.  

9/29
1. Bug: There's a thin dash line betweent the logcal view screen and the status bar.  
Possible Cause: The line number I stored in the register is lower than what it should be.  
Solution: Subtract the line number I originally stored in line compare field by 1, the dash line disappear. The line number in the Line Compare field is the one line right above the split screen starts.

10/5  
1. Bug: The screen are completely black.
Possible Cause: Octree doesn't execute successfully. No palette is set. Writing to virtual memory is not successful.
Debug process: Using gdb to print some values inside the palette, everything looks correct. Rewriting the logic in compare function which is used by the qsort, no effect.
Solution: The qsort sort the palette from low to high. I previously took the first 128 elements from the low side. After the reverse the order, the color appears.
2. Bug: The colors are random, with high saturation.
Possible Cause: The average process is wrong, or the index is wrong.
Solution: Add 64 to the index when add it to the p->img, everything works.

10/7  
1. Bug: The tux is connected, but nothing from the input.c file is working.
Possible Cause: Fail to init tux controller.  
Debug process: Print the return value of tux_init, print random value inside the kernel using printk to test whether the driver is executed.
Solution: Fail to load module, everything works after I load the module again.

10/8
1. Bug: After adding the synchronization thread in adventure.c, the scrolling rate using the tux is lagging, and moving multiple photos every time I press the C/B/A button.
Solution: Adding button_press = 0 at the end of tux_thread, there's still some extra moving, but it's better. Adding a previous command, check if the current command equals the previous command and whether it's moving the photo or moving the room. One press on the C/B/A only causes one room change. Following the instructor's advice on Piazza, add a sched_yield() at the end of the thread.