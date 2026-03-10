#include<iostream>
#include<unistd.h>
#include<sys/wait.h>
using namespace std;
int main(){
    cout<<"1. I am the Parent Program starting up.\n";
    // fork() asks the OS to clone the entire program right at this line.
    // From this point on, Two identical programs are running at the same time.
    pid_t process_id=fork();
    if(process_id<0){
        cerr<<"Fork Failed!\n";
        return 1;
    }
    else if(process_id==0){
        //fork() returns 0 to the newly created child process.
        cout<<"2. Hello from the child process\n";
        sleep(2); //simulate doing some work
        cout<<"3. Child process is finished.\n";
    }
    else{
        // fork() returns a positive number (the Child ID) to the original Parent Process.
        cout<<"2. Hello from the parent! I just birthed a child with ID: " << process_id<< "\n";
        // The parent waits for the child to finish so it doesn't leave a "zombie" process.
        wait(NULL);
        cout<<"4.Parent sees the child is done. Shutting down.\n";
    }
    return 0;
}