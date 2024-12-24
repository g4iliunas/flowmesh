#!/usr/bin/env python3

import redis

if __name__ == "__main__":
    r = redis.Redis(host='localhost', port=6379, db=0)

    for entry in r.keys("providers:*"):
        print(entry.decode(), ":", r.hgetall(entry))