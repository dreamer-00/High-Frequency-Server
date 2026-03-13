#include<iostream>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<string.h>
using namespace std;
int main(){
    int fd[2];
    if(socketpair(AF_UNIX, SOCK_STREAM, 0, fd)<0){
        cerr<<"Tunnel Collapsed"<<"\n";
        return 1;
    }
    pid_t pid=fork();
    if(pid<0){
        cerr<<"Fork Failed!\n";
        return 1;
    }
    else if(pid==0){
        close(fd[0]);
        char buffer[100];
        cout<<"[Worker] waiting for data from the tunnel...\n";
        read(fd[1], buffer, sizeof(buffer));
        cout<<"[Worker] I received a message: "<<buffer<<"\n";
        close(fd[1]);
    }
    else{
        close(fd[1]);
        sleep(1);
        const char* message="Hello from the master process! Get to the work.";
        write(fd[0], message, strlen(message)+1);
        wait(NULL);
        close(fd[0]);
    }
    return 0;
}
