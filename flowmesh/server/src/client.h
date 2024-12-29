#ifndef CLIENT_H
#define CLIENT_H

#include <uv.h>

class Client {
  public:
    Client();
    ~Client();
    uv_tcp_t *get_client() { return &this->client; }

  private:
    uv_tcp_t client;
};

#endif // CLIENT_H
