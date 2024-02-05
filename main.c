#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <utmp.h>
#include <unistd.h>
#include <sys/resource.h>
#include <math.h>

// Function to print essential system information
void print_system_info() {
    /* this function uses 2 libraries(sysinfo, utsname) to get the required system information.
        first the function initializes them. After that it simply prints out the values using the
        the right function call.
    */
    struct sysinfo si;
    struct utsname un;

    if (sysinfo(&si) != 0) {// just to see if it fails or not. if i dont put this, nothing is printed out.
        printf("error fetching sysinfo :-(\n");
        return;
    }
    if (uname(&un) != 0) {
        printf("error fetching utsname :-(\n");
        return;
    }

    printf("### System Information ###\n");
    printf("System Name = %s\n", un.sysname);
    printf("Machine Name = %s\n", un.nodename);
    printf("Version = %s\n", un.version);
    printf("Release = %s\n", un.release);
    printf("Architecture = %s\n", un.machine);
    printf("System running since last reboot: %ld days %ld:%ld:%ld (%ld:%ld:%ld)\n", si.uptime / (24 * 3600),
           (si.uptime % (24 * 3600)) / 3600, (si.uptime % 3600) / 60, si.uptime % 60,
           si.uptime / (3600), (si.uptime % 3600) / 60, si.uptime % 60); //PLEASE CHECK IF THE LAST PART WORKS OR NOT
    printf("---------------------------------------\n");
}

// Function to print information about user sessions
void print_users_info(int num_samples, int delay){
    /* this function uses 1 libraries(utmp) to get the required users information.
        first it uses setutent and getutent to populate data structure with information. After that it simply prints out the values using the
        the right function call. it also prints out the number of cores of a computer by counting the number of times
        "processor" was repeated in the proc/cpuinfo file. lastly if prints the current cpu usage. more information on how
        to do this can be found throughout the code and in the README file.
    */
    struct utmp *user;
    char info[UT_NAMESIZE +1];// this const. is given in man info for library and is 32

    printf("### Sessions/users ###\n");
    setutent();//thanx to my TA ik about this
    user = getutent(); //getutent() populates the data structure with information
                      //source: https://man7.org/linux/man-pages/man3/getutent.3.html
    while(user != NULL){
        if(user->ut_type == USER_PROCESS){// another const. with value 7
            strncpy(info, user->ut_user, UT_NAMESIZE);
            info[UT_NAMESIZE] = '\0';
            const char *ip_addr = user->ut_host;
            printf("%s\t%s (%s)\n", info,  user->ut_line, ip_addr);
        }
        user = getutent();
    }
    printf("---------------------------------------\n");
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");//with the help of https://www.cyberciti.biz/faq/check-how-many-cpus-are-there-in-linux-system/
    int num = 0;
    char line[255];
    while (fgets(line, sizeof(line), cpuinfo) != NULL) {
        if (strncmp(line, "processor", 9) == 0) {// just sibling wont work, coz it counts multithreading as well, so dont risk it!
            num++;
        }
    }
    fclose(cpuinfo);
    printf("Number of cores: %d\n", num);
    // to do the cpu usage, i will use info provided on the website https://www.linuxhowtos.org/System/procstat.htm

    FILE *statFile = fopen("/proc/stat", "r");
    if (statFile == NULL) {
        perror("Error opening /proc/stat");
        return;
    }

    // my design decision for cpu usage is to match the tdelay.
    // for more info on how to read this file see: https://www.baeldung.com/linux/total-process-cpu-usage#:~:text=2.2.-,The%20CPU%20Usage,been%20running%20in%20user%20mode.
    long preUser, preNice, preSys, preIdle, preIowait, preIrq, preSoftirq, preSteal, preGuest, preGNice;
    long user1, nice, sys, idle, iowait, irq, softirq, steal, guest, guestNice;

    // read the initial values.
    fscanf(statFile, "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &preUser, &preNice, &preSys, &preIdle, &preIowait,
           &preIrq, &preSoftirq, &preSteal, &preGuest, &preGNice);
    for (int i = 0; i < num_samples; ++i) {
        sleep(delay);

        // move the file pointer to the beginning of the file. scource: https://stackoverflow.com/questions/57530723/how-can-i-read-a-file-content-multiple-times-in-c#:~:text=You%20can%20use%20fseek(file,the%20end%20of%20the%20file.
        fseek(statFile, 0, SEEK_SET);

        // new values
        fscanf(statFile, "cpu %ld %ld %ld %ld %ld %ld %ld %ld %ld %ld", &user1, &nice, &sys, &idle, &iowait, &irq, &softirq, &steal, &guest, &guestNice);

        long preTotal = preUser + preNice + preSys + preIdle + preIowait + preIrq + preSoftirq + preSteal + preGuest + preGNice;
        long total = user1 + nice + sys + idle + iowait + irq + softirq + steal + guest + guestNice;

        long totalDiff = total - preTotal;
        long idleDiff = idle - preIdle;
        double cpuUsage = ((double)(totalDiff - idleDiff) / totalDiff) * 100.0;
        printf("total cpu use = %.2f%%\n", cpuUsage);

        preUser = user1;
        preNice = nice;
        preSys = sys;
        preIdle = idle;
        preIowait = iowait;
        preIrq = irq;
        preSoftirq = softirq;
        preSteal = steal;
        preGuest = guest;
        preGNice = guestNice;
    }
        fclose(statFile);
    printf("---------------------------------------\n");
}

// Function to print memory usage information
void print_mem_info(int num_samples, int delay, int graphics){
    /* this function uses 2 libraries(sysinfo, sys/resource) to get the required memory information.
        first it prints the total memory usage by our program in kb. then it prints various memory info using the sysinfo
        library. more information can be found throughout the code and in the README file.
    */
    struct rusage memUsage;
    struct rusage* addr = &memUsage;

    if (getrusage(RUSAGE_SELF, addr) != 0) {// using info about sys/resource and getrusage (using man command)
        printf("error fetching usageInfo :-(\n");
        return;
    }
    printf("Nbr of samples: %d -- every %d secs\n", num_samples, delay);
    printf("Memory usage: %ld kilobytes\n", memUsage.ru_maxrss);
    printf("---------------------------------------\n");
    printf("### Memory ### (Phys.Used/Tot -- Virtual Used/Tot)\n");

    double preMemo;//
    struct sysinfo si;
    if (sysinfo(&si) != 0) {
        printf("error fetching sysinfo :-(\n");
        return;
    }
    preMemo = (si.totalram - si.freeram) / (1024.0 * 1024.0 * 1024.0);
//    printf("^^^^^^^%.2f\n", preMemo);
    for (int i = 0; i < num_samples; ++i) {
        struct sysinfo si;// should be inside the loop so that i get new values everytime, i made a mistake earlier
        if (sysinfo(&si) != 0) {
            printf("error fetching sysinfo :-(\n");
            return;
        }
        double physTotal = si.totalram / (1024.0 * 1024.0 * 1024.0);
        double physUsed = (si.totalram - si.freeram) / (1024.0 * 1024.0 * 1024.0);
        double virTotal = (si.totalram + si.totalswap) / (1024.0 * 1024.0 * 1024.0);
        double virUsed = (si.totalram - si.freeram + si.totalswap - si.freeswap) / (1024.0 * 1024.0 * 1024.0); // Wrong calculation
        if(!graphics){
            printf("%.2f GB / %.2f GB -- %.2f GB / %.2f GB\n", physUsed, physTotal, virUsed, virTotal);
        } else{
            if(i != 0){
//                printf("^^^^^^^%.2f\n", preMemo);
                double change = (physUsed - preMemo);
                printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB   |", physUsed, physTotal, virUsed, virTotal);

                // i will be assuming that the change is not more than 200 mb, if it is, then there will be no * or @ in the end
                for (int j = 0; j < 20; ++j) {
                    if (change > 0 && j < round(change*100)) {
                        if(j + 1 == round(change*100)){
                            putchar('*');
                        }else putchar('#');
                    } else if(change < 0 && j < round(-change*100)) {
                        if(j + 1 == round(-change*100)){
                            putchar('@');
                        }else putchar(':');
                    }
                }

                printf(" %.2f (%.2f)\n", change, physUsed);
            }
            else{
                printf("%.2f GB / %.2f GB  -- %.2f GB / %.2f GB   |", physUsed, physTotal, virUsed, virTotal);
                printf("o 0.00 (%.2f)\n", physUsed);
            }

        }
        preMemo = physUsed;
        sleep(delay);
    }

    printf("---------------------------------------\n");
}

// Function to clear the console screen
void clearScreen() {
    // using the info provided on the site: https://stackoverflow.com/questions/37774983/clearing-the-screen-by-printing-a-character
    printf("\033[2J"); // clear
    printf("\033[H");  // left
}

// Main function
int main(int argc, char *argv[]) {
    /* main funtion to initialize everything as well as check for various command line arguments.
     */
    int samples = 10;
    int tdelay = 1;
    int mode = 0;// for user and system
    int seq = 0;
    int graphics = 0;
    clearScreen();
    if (argc == 3) {
        samples = atoi(argv[1]);
        tdelay = atoi(argv[2]);
    } else if (argc > 1) {
        char *arg = argv[1];
        if (strncmp(arg, "--samples=", 10) == 0) {
            samples = atoi(arg + 10);
        } else if (strncmp(arg, "--tdelay=", 9) == 0) {
            tdelay = atoi(arg + 9);
        } else if (strcmp(argv[1], "--sequential") == 0) {
            seq = 1;
        } else if (strcmp(argv[1], "--system") == 0) {
            mode = 1;
        } else if (strcmp(argv[1], "--user") == 0) {
            mode = 2;
        } else if ((strcmp(argv[1], "--graphics") == 0) || (strcmp(argv[1], "-g") == 0)) {
            graphics = 1;
        }
    }
    if(!seq){
        if (mode == 0) {
            print_mem_info(samples, tdelay, graphics);
        }
        if (mode == 0 || mode == 2) {
            print_users_info(1, tdelay);
        }
        if (mode == 0 || mode == 1) {
            print_system_info();
        }
    }
    else{
        for (int count = 0; count < samples; count++) {
            printf(">>> iteration %d\n", count);

            if (mode == 0) {
                print_mem_info(1, tdelay, graphics); // Use 1 sample for sequential mode
            }
            if (mode == 0 || mode == 2) {
                print_users_info(1, tdelay);
            }
            if (mode == 0 || mode == 1) {
                print_system_info();
            }

            sleep(tdelay);
        }
    }


    return 0;
}