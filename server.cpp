#include<bits/stdc++.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;
typedef long long ll;
const long long k_max_msg = 4096;

enum {
    STATE_REQ =0,
    STATE_RES =1,
    STATE_END=2,
};

struct conn {
    int fd =-1;
    int state = 0;
    size_t rbuf_size = 0;
    uint8_t rbuf[4+k_max_msg];
    size_t wbuf_size =0;
    size_t wbuf_sent =0;
    uint8_t wbuf[4+k_max_msg];
};

void msg(string message) {
    cout <<message <<'\n';
}

int32_t read_full(int fd, char *buff, ll n ) {
    while( n > 0) {
        ll rv = read(fd, buff,n);
        if(rv <= 0) return -1;
        assert((ll) rv <= n);
        n -= rv;
        buff+=rv;
    }
    return 0;
}

int32_t write_all(int fd, char *buff, ll n) {
    while(n >0) {
        ll rv =write(fd, buff, n);
        if(rv <=0) return -1;
        assert(rv <=n);
        n -= rv;
        buff+=rv;
    }
    return 0;
}


ll one_request(int client_socket_fd) {
    char buff[4+k_max_msg+1];
    errno =0;
    ll err = read_full(client_socket_fd, buff, 4);
    if(err) {
        if(errno ==0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }
    ll len  =0;
    memcpy(&len, buff,4);
    if(len > k_max_msg) {
        msg("too long");
        return -1;
    }
    err = read_full(client_socket_fd, &buff[4], len);
    if(err) {
        msg("read() error");
        return -1;
    }
    buff[4+len] ='\0';
    cout << "client says" << &buff[4] <<"\n";

    const char reply[] = "world";
    char write_buffer[4+ sizeof(reply)];
    len = strlen(reply);

    memcpy(write_buffer, &len, 4);
    
    memcpy(&write_buffer[4], reply,len);
    return write_all(client_socket_fd,write_buffer, 4+ len);
}

void do_something(int client_socket_fd) {
    char buff[64] = {};
    int n = read(client_socket_fd, buff, sizeof(buff) -1);
    if(n < 0) {
        cerr << "read has been failed\n";
    }
    cout << "client says" << buff <<"\n";
    string wbuff = "world";
    write(client_socket_fd, wbuff.c_str(), wbuff.length());
}

int main() {
    int fd = socket(AF_INET, SOCK_STREAM,0);
    int val =1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,&val, sizeof(val));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port= ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);

    int rv = bind(fd, (struct sockaddr*)&addr, sizeof(addr));

    if(rv) {
        cout << "bind()\n";
    }
    rv = listen(fd, SOMAXCONN);
    if(rv) {
        cout <<"listen()\n";
    }

    while(true) {
        sockaddr_in client_addr = {};
        socklen_t sock_len = sizeof(client_addr);
        int client_socket_fd = accept(fd, (struct sockaddr*)&client_addr,&sock_len);
        if(client_socket_fd <0) {
            continue;
        }
        while(true) {
            int32_t err = one_request(client_socket_fd);
            if(err) break;
        }
        close(client_socket_fd);
    }
}
