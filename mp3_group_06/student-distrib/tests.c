#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "rtc.h"
#include "filesys.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

/* IDT Test - exception
 * 
 * Check if zero-division exception succefully raised
 * Inputs: None
 * Outputs: None
 * Side Effects: Raise exception or not
 * Coverage: exception handler
 * Files: idt.h/c, exceptions.h/S
 */
// void testExZero() {
// 	int a;
// 	a = 3/0;
// }

/* Paing Test1 
 * 
 * Test if we can successfully dereference a valid pointer, and compare the derefenreced content
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
int testPaging1() {
	TEST_HEADER;
	int a = 391;                       //random number for testing
	int*b = &a;
	int result = PASS;
	if (*b != a) {
		assertion_failure();
		result = FAIL;
	}
	return result;
}

/* Paing Test Null
 * 
 * Test if we can raise exception by dereference a NULL pointer
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
void testPagingNULL() {
	TEST_HEADER;
	int* a = NULL;
	int b;
	b = *a;
}

/* Paing Test set-up memory
 * 
 * Test if we can access memory that is set to presend
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
void testPagingUsedMem() {
	TEST_HEADER;
	int* addr = (int*)0xb8000;                    //video memory address, dereference should not cause exception
	int b;
	b = *addr;
}

void testIDT() {
 	__asm__("INT $0x15\n\t");
}

/* Paing Test video memory
 * 
 * Test if we can access memory that is kernel page (4mb)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
 void testPagingUsedMem1() {
	TEST_HEADER;
	int* addr = (int*)0x400000;                   //kernel address, should not cause exception
	int b;
	b = *addr;
}


/* Paing Test unused memory1
 * 
 * Test if we can access memory that is not set to presend (at 3MB)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
void testPagingUnusedMem1() {
	TEST_HEADER;
	int* addr = (int*)0x300000;                 //unmapped memory, page fault
	int b;
	b = *addr;
}

/* Paing Test unused memory2
 * 
 * Test if we can access memory that is not set to presend (at 0x0)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
void testPagingUnusedMem2() {
	TEST_HEADER;
	int* addr = (int*)0x0;                      //unmapped memory, page fault
	int b;
	b = *addr;
}
/* Paing Test unused memory3
 * 
 * Test if we can access memory that is outside of page initialization (9mb)
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
 void testPagingUnusedMem3() {
	TEST_HEADER;
	int* addr = (int*)0x900000;                 //unmapped memory, page fault
	int b;
	b = *addr;
}

/* Paing Test value in PDE
 * 
 * Test if the value in PDE is correct
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
 int valueInPDE() {
	TEST_HEADER;
	int result = PASS;
	uint32_t a = pde[1].addr;
	if (a != 0x400) {                          //address offset for kernel page
		assertion_failure();
		result = FAIL;
	}
	return result;
}

/* Paing Test value in PTE
 * 
 * Test if the value in PTE is correct
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Paging initialize and enable
 * Files: paging.h/S
 */
 int valueInPTE() {
	TEST_HEADER;
	int result = PASS;
	uint32_t a = pte[0xb8].present;                //check the present plage at 0xb8
	if (a != 1) {
		assertion_failure();
		result = FAIL;
	}
	return result;
}

/* Checkpoint 2 tests */

/* RTC Test 1
 * Test if rtc frequency initially set as 2Hz
 * Inputs: None
 * Outputs: None
 * Side Effects: Print "1" in the screen at frequency equal to 2Hz
 * Coverage: RTC Open(init + set frequency), RTC Read
 * Files: rtc.h/S
 */
void testRTC1() {
	TEST_HEADER;
	uint8_t fn[3] = "RTC"; //random string, unused
	rtc_open(fn);

	int buf[32];
	buf[0] = 0; //random number, unused

	while(1) {
		if (rtc_read(0, buf, 4) == 0){
			printf("1");
		}
	}
}

/* RTC Test 2
 * Test if can set rtc frequency
 * Inputs: freq - the frequency we want to set to
 * Outputs: None
 * Side Effects: If freq is power of 2, print "1" in the screen at frequency equal to freq
 * 				 If freq is not the power of 2, print "fail to set the frequency"
 * Coverage: RTC Open(init + set frequency), RTC Read, RTC Write
 * Files: rtc.h/S
 */
void testRTC2(int freq) {
	TEST_HEADER;
	uint8_t fn[3] = "RTC"; //random string, unused
	rtc_open(fn);
	int result;
	int buf[32];
	buf[0] = freq;

	result = rtc_write(0, buf, 4);
	if (result != 0) {
		printf("fail to set the frequency");
	}else {
		while(1) {
			if (rtc_read(0, buf, 4) == 0){
				printf("1");
			}
		}
	}
	rtc_close(0);
}

/* RTC Test 3
 * Test if rtc frequency can set as 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024Hz
 * Inputs: None
 * Outputs: None
 * Side Effects: Print "1" in the screen accelerate from 2Hz to 1024Hz
 * Coverage: RTC Open(init + set frequency), RTC Read, RTC Write
 * Files: rtc.h/S
 */
void testRTC3() {
	TEST_HEADER;
	uint8_t fn[3] = "RTC";//random string, unused
	rtc_open(fn);

	int result;
	int buf[32];

	int i;
	int p;
	p = 1;
	int count = 10; // in total 10 different frequencies
	for (i = 0; i < 10; i++){
		p *= 2;
		buf[0] = p;
		result = rtc_write(0, buf, 4);
		if (result != 0) {
			printf("fail to set the frequency");
		}else {
			while(count != 0) {
				if (rtc_read(0, buf, 4) == 0){
					printf("1");
					count--;
				}
			}
			count = 10;
		}
	}
	rtc_close(0);
}

/* File system test: read_dentry_by_index
 * 
 * Test if we can read dentry by index successfully
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Print file name of in the dentry if success
 * Coverage: File system initialization
 * Files: filesys.h/c
 */
int testReadDentryByIndex() {
	TEST_HEADER;
	dentry_t dt;
	int index = 0;
	int result = PASS;
	if (read_dentry_by_index(index, &dt) != 0) {
		assertion_failure();
		result = FAIL;
	}
	int i;
	for (i = 0 ; i < 32; i++) {                              //32, max file name length
		printf("%c", dt.filename[i]);
	}
	printf("\n");
	return result;
}

/* File system test: read_dentry_by_name
 * 
 * Test if we can find the dentry by its file name
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: print out the file name if success
 * Coverage: File system initialization
 * Files: filesys.h/c
 */
int testReadDentryByName() {
	TEST_HEADER;
	dentry_t dt;
	int result = PASS;
	if (read_dentry_by_name((uint8_t*)"frame0.txt", &dt) != 0) {
		assertion_failure();
		result = FAIL;
	}
	int i;
	for (i = 0 ; i < 32; i++) {                            //32, max file name length
		printf("%c", dt.filename[i]);
	}
	printf("\n");
	return result;
}

/* File system test: dir_read
 * 
 * Test if we can read every files inside the directory in a while loop
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: print all the filename and its file size
 * Coverage: directory read and file system initialization
 * Files: filesys.h/c
 */
// int testDirRead() {
// 	TEST_HEADER;
// 	int result = PASS;
// 	int bytes;
// 	uint8_t buf[33];                                      //need a null-terminated character at the end
// 	buf[32] = '\0';
// 	if (dir_open((uint8_t*)".", 2) != 0) {                //file descriptor is hardcode to 2
// 		result = FAIL;
// 		return result;
// 	}else {
// 		while(1) {
// 			bytes = dir_read(2, &buf, 32);
// 			if (bytes == -1) {
// 				break;
// 			}
// 			printf("File name: %s, File size: %d\n",buf,bytes);
// 		}
// 	}
// 	return result;
// }

/* File system test: f_read
 * 
 * Test if we can read a small txt file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Display the file content
 * Coverage: File read
 * Files: filesys.h/c
 */
// int testFileReadSmall() {
// 	TEST_HEADER;
// 	int result = PASS;
// 	int bytes_read;
// 	int i = 0;
// 	uint8_t buf[200];                                     //buffer size for data, larger than the file size
// 	uint8_t filename[32] = "frame1.txt";
// 	if(f_open((uint8_t*)filename, 2) != 0){               //file descriptor is hardcode to 2 at this point
// 		result = FAIL;
// 		return result;
// 	} else {
// 		bytes_read = f_read(2, &buf, 200);                //read 200 byte, exceed the file size, it still works
// 		if (bytes_read == -1) {
// 			result = FAIL;
// 			return result;
// 		}
// 		while(i != bytes_read) {
// 			printf("%c", buf[i]);
// 			i++;
// 		}
// 		printf("\n");
// 		printf("file name: %s\n", filename);
// 	}
// 	return result;
// }

/* File system test: f_read
 * 
 * Test if we can read a large txt file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Display the file content to the screen
 * Coverage: File read
 * Files: filesys.h/c
 */
// int testFileReadLarge() {
// 	TEST_HEADER;
// 	int result = PASS;
// 	int bytes_read;
// 	int i = 0;                                           //iteration constant
// 	uint8_t buf[40000];                                  //large buffer for reading large file
// 	uint8_t filename[] = "verylargetextwithverylongname.tx";
// 	if(f_open((uint8_t*)filename, 2) != 0){
// 		result = FAIL;
// 		return result;
// 	} else {
// 		bytes_read = f_read(2, &buf, 40000);            //read 40000 bytes, might exceed the file size, which should still works
// 		if (bytes_read == -1) {
// 			result = FAIL;
// 			return result;
// 		}
// 		while(i != bytes_read) {
// 			printf("%c", buf[i]);
// 			i++;
// 		}
// 		printf("\n");
// 		printf("file name: %s\n", filename);
// 	}
// 	return result;
// }

/* File system test: f_read
 * 
 * Test if we can read into a executable file
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: Display the content in the executable to the screen
 * Coverage: File read
 * Files: filesys.h/c
 */
// int testFileReadExecutable() {
// 	TEST_HEADER;
// 	int result = PASS;
// 	int bytes_read;
// 	int i = 0;                                       //iteration constant
// 	uint8_t buf[40000];                              //large buffer for executable
// 	uint8_t filename[] = "testprint";
// 	if(f_open((uint8_t*)filename, 2) != 0){
// 		result = FAIL;
// 		return result;
// 	} else {
// 		bytes_read = f_read(2, &buf, 40000);
// 		if (bytes_read == -1) {
// 			result = FAIL;
// 			return result;
// 		}
// 		while(i != bytes_read) {
// 			if (buf[i] != '\0') {
// 				putc(buf[i]);
// 			}
// 			//putc(buf[i]);
// 			i++;
// 		}
// 		printf("\n");
// 		printf("file name: %s\n", filename);
// 	}
// 	return result;
// }

// /* Terminal Test
//  * Test read and write with terminal
//  * Inputs: None
//  * Outputs: None
//  * Files: terminal.h/S, keyboard.h/c, lib.h/c
//  */
int testTerminal()
{
	TEST_HEADER;
	uint8_t fn[9] = "Terminal";
	terminal_open(fn);

	int result;
	int8_t buf[BUF_SIZE];
	int bytes_read;
	bytes_read = 0;
	result = PASS;

	while (1)
	{
		terminal_write(0, (int8_t*)"Terminal reading: 128 bytes\n", 28);
		bytes_read = terminal_read(0, buf, BUF_SIZE);
		terminal_write(0, buf, bytes_read);
		terminal_write(0, (int8_t*)"Terminal reading: 5 bytes\n", 26);
		bytes_read = terminal_read(0, buf, 5);
		terminal_write(0, buf, bytes_read);
		terminal_write(0, (int8_t*)"Terminal reading: 150 bytes\n", 28);
		bytes_read = terminal_read(0, buf, 150);
		terminal_write(0, buf, bytes_read);
	}
	
	terminal_close(0);
	return result;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	// launch your tests here
    // while (1) {
    //         test_interrupts();
    //     }
    //TEST_OUTPUT("testPaging1", testPaging1());

	//testPagingNULL();
	//testPagingUsedMem();
	//testPagingUsedMem1();
	//testPagingUnusedMem1();
	//testPagingUnusedMem2();
	//testPagingUnusedMem3();


	//testExZero();
	//TEST_OUTPUT("valueInPDE", valueInPDE());
	//TEST_OUTPUT("valueInPTE", valueInPTE());
	/*checkpoint 2 tests*/
	//testRTC1();
	//testRTC2(5);
	//testRTC2(16);
	//testRTC3();
	//TEST_OUTPUT("testReadDentryByIndex", testReadDentryByIndex());
	//TEST_OUTPUT("testReadDentryByName", testReadDentryByName());
	//TEST_OUTPUT("testDirRead", testDirRead());
	//TEST_OUTPUT("testFileReadSmall", testFileReadSmall());
	//TEST_OUTPUT("testFileReadLarge", testFileReadLarge());
	//TEST_OUTPUT("testFileReadExecutable", testFileReadExecutable());
	//TEST_OUTPUT("testTerminal",testTerminal());
}

