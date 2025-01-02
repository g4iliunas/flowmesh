#!/usr/bin/env python3

import redis

if __name__ == "__main__":
    r = redis.Redis(host='localhost', port=6379, db=0)


    print("Proxy provider stat entries:")
    for entry in r.keys("provider_hash:*"):
        print(entry.decode(), ":", r.hgetall(entry))

    print("Proxy provider entries:")
    for entry in r.keys("provider:*"):
        print(entry.decode(), ":", r.hgetall(entry))