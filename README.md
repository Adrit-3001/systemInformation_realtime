# systemInformation_realtime
this repository has a C file that contains a code that can be run on on linux terminal to get various system informations.
System Monitor Program 
This program is a simple system monitor that provides information about system 
resources, user sessions, and memory usage. Please not that I have provided 
comments throughout the code to help better understand. These comments include 
useful resources that I used for the assignment as well as some personal comments 
that help me understand what I did and how I did it later if I decided to look at this 
assignment once again. 
Overview of Functions 
1. print_system_info() 
This is a simple function that retrieves and prints essential system information 
using the sysinfo and utsname libraries. The displayed information includes the 
system name, machine name, version, release, architecture, and uptime since the 
last reboot. 
2. print_users_info(int num_samples, int delay) 
This function takes in 2 arguments and prints information about user sessions. It 
utilizes the utmp library to access user-related data, such as username, terminal 
line, and IP address. Next it gets the cpu usage information by utilizing the 
information given in the proc/stat file. After various calculations, it calculates the 
cpu usage and prints that information, it also updates the previous value to give 
real time data. It can have various delay between usage, depending on the value 
you provide. I am printing the cpu usage after memory information, just so it is 
easy to read and not too complicated. 
3. print_mem_info(int num_samples, int delay, int graphics) 
This function provides memory usage information, both physical and virtual. It 
uses the getrusage function to obtain memory-related statistics for the information 
about memory being used by our program and the sysinfo library to calculate 
physical and virtual memory usage. If the graphics command is not executed, the 
program simply gets the corresponding values and prints them. The graphics
parameter enables a visual representation of memory changes if set to 1. To 
calculate graphical representation, I have approached this problem by using a 
temporary memory info that stores the previous memory usage. The function 
calculates the change in memory and prints the symbol accordingly. The graphical 
representation works like this (assuming the change is never more than 200mb): if 
the change is 0.03, it will print 3 characters and 12 characters if change is 0.12. The 
representation caps out at 0.20, any change more than this will not be shown 
accurately because of my assumption. Please give me some marks for graphics T_T. I know I didn’t do cpu part but 
please T_T.
4. clearScreen() 
This function clears the console screen using escape codes. 
5. main(int argc, char *argv[]) 
The main function parses command-line arguments and determines the program's 
behavior. It supports options such as the number of samples, delay between 
samples, sequential mode, system information display, user information display, 
and graphics mode. The program then executes the corresponding functions based 
on the provided options. It also gives default values to the constants. If there is no 
sequential mode indicated, then it calls the function according to the command line 
arguments. If sequential mode is indicated, it goes through a loop of 10 iterations 
starting from 0 to 9 and calls the functions to print the various information and has 
a default delay of 1 second between every iteration. 
Important note: the program assumes there will not be several command line 
arguments in the same line, for example: ./a.out --sequential --samples=5 --
tdelay=2 –graphics. It can perform all the commands that were shown in the demo 
video. Also, all the useful resources (e.g.- websites links) can be found throughout 
the code explaining where I got the information from to tackle a certain task. 
How to Run the Program: 
Please note that I have used the math.h library, so you will need to compile the 
program using gcc main.c -lm. To run it, simply execute ./a.out [options]. 
