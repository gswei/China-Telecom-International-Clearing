#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <time.h>

pid_t* childpid = NULL;
extern "C" {
void createMemory(const long memory, const int sp = 10);
void createCpu(const int num, const int sp);
pid_t r_wait(int *stat_loc) {
    int retval;

    while (((retval = wait(stat_loc)) == -1) && (errno == EINTR))
        ;
    return retval;
}

void sig_int(int signo) {

    if (getpid() == getpgid(0)) {
        for (int i = 0; *(childpid + i) != 0 || i < sizeof(childpid) / sizeof(pid_t); i++) {
            if (*(childpid + i) != 0) {
                kill(*(childpid + i), SIGKILL);
            }
        }
        while (wait(NULL) > 0) {
            ;
        }
        printf("用户中止了当前处理...\n");
        exit(1);
    }
}
}

int main(int argc, char *argv[]) {
    if (argc <= 1||argc!=3) {
        printf("系统压力测试工具\n");
        printf("testTools -c cpus -m memorysize \n");
        return 0;
    }

    int cpuNumber = 0, memSize = 0;
    int step=10;
    if(strncmp(argv[1],"-c",2)==0){
        cpuNumber = (int) strtol(argv[2], NULL, 0);
        printf("cpu个数:%d\n", cpuNumber);
    }else if(strncmp(argv[1],"-m",2)==0){
        memSize = (int) strtol(argv[2], NULL, 0);
        printf("分配内存大小:%dM\n", memSize);
    }else{
        printf("系统压力测试工具\n");
        printf("testTools -c cpus -m memorysize \n");
        return 0;
    }

    if (cpuNumber > 0) {
        createCpu(cpuNumber, step);
        exit(0);
    }
    if (memSize > 0) {
        createMemory(memSize, step);
        exit(0);
    }
    return 0;
}

//分配内存空间大小,单位为M
//这里不释放内存了,程序退出直接释放吧
void createMemory(const long memory, const int sp) {
    void * temp;
    int i = 0;
    long mSize=memory*1024*1024;
    while (true) {
        printf("已经分配%ldM\n", (i + 1) *memory);
        temp = malloc(mSize);
        memset(temp, 0, mSize);
        i++;
        sleep(2);
    }
}

void createCpu(const int num, const int sp) {
    signal(SIGINT, sig_int);
    childpid = new pid_t[num];
    for (int i = 0; i < num; i++) {
        childpid[i] = fork();
        if (childpid[i] == -1) {
            printf("fork failed for%s\n", strerror(errno));
            exit(1);
        } else if (childpid[i] == 0) {
            //执行cpu密集计算函数
            int rr=0;
            while (true) {
                srand((int) time(0));
                rr = rand();
            }
            exit(0);
        }
    }
    ////等待所有的子进程结束
    int status = 0;
    pid_t retPid;
    while ((retPid = r_wait(&status)) > 0) {
        ;
    }
}

