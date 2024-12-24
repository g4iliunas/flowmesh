#!/usr/bin/env python3

import redis
import sys
import hashlib
import time

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: ./gen_provider.py <username>")

    username = sys.argv[1]

    # generate a token
    token = hashlib.sha256((username + str(time.time())).encode()).hexdigest() 

    # open a connection to redis db
    r = redis.Redis(host='localhost', port=6379, db=0)

    # add an entry (we dont include the username or balance in here, because other tables will hold this entry)
    r.hset(f"providers:{token}", mapping={"bytes_sent": 0, "bytes_received": 0})

    # print out the result
    print(f"providers:{token}", ":", r.hgetall(f"providers:{token}"))