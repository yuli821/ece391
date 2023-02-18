### P1 Solution

1. The 10-bits Line Compare field specifices the scan line at which a horizontal division can occur, providing for split-screen operation. If no horizontal division is required, its field should be set to 3FFh. The 6th bit of Maximum Scan Line Register(09h) stores the bit 9 of Line Compare field. The 4th bit of Overflow Register(07h) stores the bit 8 of Line Compare field. The Line Compare Register(18h) stores the bit 7-0 of Line Compare field. When the line counter reaches the value in the Line Compare Register, the current scan line start address is reset to 0.   
Constraints:   
-Either the top and bottom window are panned by the same amount, or the panning is only applied to the top window. If the Pixel Panning Mode field(5th bit of Attribute Mode Control Register(10h)) is set to 1, then the Pixel Shift Count(bit 3-0 in Horizontal Pixel Panning Register(13h)) and Byte Panning fields(bit 6-5 in Preset Row Scan Register(08h)) are reset to 0 for the remainder of the display cycle.  
-The Preset Row Scan only works for the top window, the value for the buttom is fixed to 0.   
-When the line counter reaches the value in the Line Compare Register, the current scan line start address is reset to 0. Because of this, the bottom window's display memory have to start at offset 0. The bottom screen locates first in memory and is followed by the top in memory. Since the status bar is non-scrolling, it needs to be located in the bottom screen, therefore, the content of status bar needs to start at offset 0. 

2. First, set the DAC Address Write Mode Register(3C8h) to the palette entry's index value.   
Second, writes the 6-bits value of red intensity to the DAC Data Register(3C9h).    
Third, writes the 6-bits value of green intensity to the DAC Data Register.           
Fourth, writes the 6-bits value of blue intensity to the DAC Data Register.  
After three consecutive writes, the 18-bit RGB value in an order of red, green, and blue is loaded into the palette RAM.
