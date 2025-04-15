#include <bits/stdc++.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <poll.h>
using namespace std;
typedef long long ll;
const long long k_max_msg = 4096;

enum
{
    STATE_REQ = 0,
    STATE_RES = 1,
    STATE_END = 2,
};

struct Conn
{
    int fd = -1;
    int state = 0;
    size_t rbuf_size = 0;
    uint8_t rbuf[4 + k_max_msg];
    size_t wbuf_size = 0;
    size_t wbuf_sent = 0;
    uint8_t wbuf[4 + k_max_msg];
};

void msg(string message)
{
    cout << message << '\n';
}

int32_t read_full(int fd, char *buff, ll n)
{
    while (n > 0)
    {
        ll rv = read(fd, buff, n);
        if (rv <= 0)
            return -1;
        assert((ll)rv <= n);
        n -= rv;
        buff += rv;
    }
    return 0;
}

int32_t write_all(int fd, char *buff, ll n)
{
    while (n > 0)
    {
        ll rv = write(fd, buff, n);
        if (rv <= 0)
            return -1;
        assert(rv <= n);
        n -= rv;
        buff += rv;
    }
    return 0;
}

ll one_request(int client_socket_fd)
{
    char buff[4 + k_max_msg + 1];
    errno = 0;
    ll err = read_full(client_socket_fd, buff, 4);
    if (err)
    {
        if (errno == 0)
        {
            msg("EOF");
        }
        else
        {
            msg("read() error");
        }
        return err;
    }
    ll len = 0;
    memcpy(&len, buff, 4);
    if (len > k_max_msg)
    {
        msg("too long");
        return -1;
    }
    err = read_full(client_socket_fd, &buff[4], len);
    if (err)
    {
        msg("read() error");
        return -1;
    }
    buff[4 + len] = '\0';
    cout << "client says" << &buff[4] << "\n";

    const char reply[] = "world";
    char write_buffer[4 + sizeof(reply)];
    len = strlen(reply);

    memcpy(write_buffer, &len, 4);

    memcpy(&write_buffer[4], reply, len);
    return write_all(client_socket_fd, write_buffer, 4 + len);
}

void do_something(int client_socket_fd)
{
    char buff[64] = {};
    int n = read(client_socket_fd, buff, sizeof(buff) - 1);
    if (n < 0)
    {
        cerr << "read has been failed\n";
    }
    cout << "client says" << buff << "\n";
    string wbuff = "world";
    write(client_socket_fd, wbuff.c_str(), wbuff.length());
}

void die(string s)
{
    cout << s << endl;
    return;
}

void fd_set_nb(int fd)
{
    errno = 0;
    int flag = fcntl(fd, F_GETFL, 0);

    if (errno)
    {
        die("fctnl erro");
        return;
    }

    flag |= O_NONBLOCK;

    errno = 0;
    (void)fcntl(fd, F_SETFL, flag);
    if (errno)
    {
        die("fcntl error");
    }
}

int main()
{
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        die("Socket()");
        return;
    }

    int val = 1;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    addr.sin_port = ntohs(1234);
    addr.sin_addr.s_addr = ntohl(0);

    int rv = bind(fd, (struct sockaddr *)&addr, sizeof(addr));

    if (rv)
    {
        cout << "bind()\n";
    }
    rv = listen(fd, SOMAXCONN);
    if (rv)
    {
        cout << "listen()\n";
    }

    std::vector<Conn *> fd2conn;

    fd_set_nb(fd);

    std::vector<struct pollfd> poll_args;

    while (true)
    {
        poll_args.clear();

        struct pollfd pfd = {fd, POLLIN, 0};
        poll_args.push_back(pfd);

        for (Conn *conn : fd2conn)
        {
            if (!conn)
            {
                continue;
            }
            struct pollfd pfd = {};
            pfd.fd = conn->fd;
            pfd.events = conn->state == STATE_REQ ? POLLIN : POLLOUT;
            pfd.events = pfd.events | POLLERR;
            poll_args.push_back(pfd);
        }

        int rv = poll(poll_args.data(), (nfds_t)poll_args.size(), 1000);
        if (rv < 0)
        {
            die("poold");
        }
        for (size_t i = 1; i < poll_args.size(); i++)
        {
            if (poll_args[i].revents)
            {
                Conn *conn = fd2conn[poll_args[i].fd];
                connection_io(conn);
                if (conn->state == STATE_END)
                {
                    // client closed normally, or something bad happened.
                    // destroy this connection
                    fd2conn[conn->fd] = NULL;
                    (void)close(conn->fd);
                    free(conn);
                }
            }
        }

        if(poll_args[0].revents) {
            (void) accept_new_conn(fd2conn,fd);
        }
    }
}
