#include<iostream>
#include<thread>
#include<chrono>
#include<atomic>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<fcntl.h>
constexpr int RING_SIZE=1024;
constexpr int PACKET_SIZE=512;
//Cache Line aligned packet slot
struct alignas(64) Slot{
    char data[PACKET_SIZE];
    int len;
};
//Ring Buffer
struct RingBuffer{
    Slot slots[RING_SIZE];
    std::atomic<int> write_idx{0};
    std::atomic<int> read_idx{0};
};
//Make Socket Non-blocking
void make_nonblocking(int fd) {
    int flags=fcntl(fd, F_GETFL,0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
RingBuffer ring;
//Network Thread: Receives packets and writes to ring buffer
void network_thread(){
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in addr{};
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(9000);
    bind(fd, (sockaddr*)&addr, sizeof(addr));
    make_nonblocking(fd);
    char buffer[PACKET_SIZE];
    while(true){
        sockaddr_in src{};
        socklen_t slen=sizeof(src);
        int n=recvfrom(fd,buffer,sizeof(buffer),0,(sockaddr*)&src,&slen);
        if(n>0){
            int w=ring.write_idx.load(std::memory_order_relaxed);
            int next_w=(w+1)%RING_SIZE;
            if(next_w!=ring.read_idx.load(std::memory_order_acquire)){
                memcpy(ring.slots[w].data, buffer, n);
                ring.slots[w].len=n;
                ring.write_idx.store(next_w, std::memory_order_release);
            }
        }
    }
}
void strategy_thread(){
    while(true){
        int r=ring.read_idx.load(std::memory_order_relaxed);
        if(r!=ring.write_idx.load(std::memory_order_acquire)){
            //Process packet in ring.slots[r]
            Slot& slot=ring.slots[r];
            auto start=std::chrono::high_resolution_clock::now();
            auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(start.time_since_epoch()).count();
            std::cout<<"Strategy got packet : "
                     <<slot.len<<" bytes at "
                     <<ns<<"ns\n";
                     int next_r=(r+1)%RING_SIZE;
                     ring.read_idx.store(next_r, std::memory_order_release);
        }
    }
}
int main(){
    std::thread net_thread(network_thread);
    std::thread strat_thread(strategy_thread);
    net_thread.join();
    strat_thread.join();
    return 0;
}
