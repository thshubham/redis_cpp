#include<bits/stdc++.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
using namespace std;
typedef long long ll;
const ll k_max_msg = 4096;

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

static ll query(int fd, char *text) {
    ll len = strlen(text);
    if(len > k_max_msg) {
        return -1;
    }
    char writeBuffer[4+k_max_msg];
    memcpy(writeBuffer, &len,4);
    memcpy(&writeBuffer[4], text,len);
    if(int err = write_all(fd, writeBuffer, 4+len)) {
        return err;
    }

    char readBuffer[4 + k_max_msg+1];
    errno = 0;
    int err = read_full(fd, readBuffer,4);

    if(err) {
        if(errno ==0) {
            msg("EOF");
        } else {
            msg("read() error");
        }
        return err;
    }

    memcpy(&len, readBuffer,4);

    if(len > k_max_msg) {
        msg("too long");
        return -1;
    }

    err = read_full(fd, &readBuffer[4], len);

    if(err) {
        msg(" read() error");
        return err;
    }

    readBuffer[4+len] = '\0';
    cout << "server says " << &readBuffer[4] <<"\n";
    return 0;
}

int  main() {
    int fd = socket(AF_INET,SOCK_STREAM,0);
    if(fd < 0) {
        cout<<"socket()";
        return 1 ;
    }
    sockaddr_in addr ={};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(INADDR_LOOPBACK);

    int rv = connect(fd,(const struct sockaddr *)& addr, sizeof(addr));
    if(rv < 0) {
        cout<<"connect()";
        return 1 ;
    }

    // string msg = "hello";
    // write(fd, msg.c_str(),msg.length());
    // char buff[64]= {};
    // int n = read(fd,buff, sizeof(buff)-1);

    // if(n < 0) {
    //     cout<<"read()";
    //     return 1;
    // }

    // cout << "server says" << buff<<"\n";
    ll err = query(fd, "hello1");
    if(err) {
        goto L_DONE;
    }
    err = query(fd, "hello2");
    if(err) {
        goto L_DONE;
    }
    err = query(fd, "hello3");
    if(err) {
        goto L_DONE;
    }
    
    L_DONE:
        close(fd);
        return 0;
}