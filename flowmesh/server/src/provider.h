#ifndef PROVIDER_H
#define PROVIDER_H

#include "client.h"

class Provider : Client<Provider> {
public:
    Provider();
    ~Provider();
};

#endif