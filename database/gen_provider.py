#!/usr/bin/env python3

import redis
import sys
import hashlib
import time
import string
import secrets

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: ./gen_provider.py <proxy_username>")
        exit(1)

    proxy_username = sys.argv[1]

    # generate a token
    token = hashlib.sha256((proxy_username + str(time.time())).encode()).hexdigest() 
    print("Generated token:", token)

    # not hashing it would be a security flaw
    hashed_token = hashlib.sha256(token.encode()).hexdigest()

    # open a connection to redis db
    r = redis.Redis(host='localhost', port=6379, db=0)

    # add an entry (we dont include the username or balance in here, because other tables will hold this entry)
    r.hset(f"provider_hash:{hashed_token}", mapping={"bytes_sent": 0, "bytes_received": 0})

    # print out the result
    print(f"provider_hash:{hashed_token}:", r.hgetall(f"providers:{hashed_token}"))

    ### add proxy user entry
    # generate a password
    proxy_password = ''.join(secrets.choice(string.ascii_letters + string.digits) for i in range(10))  
    print("Generated password:", proxy_password)

    r.hset(f"provider:{proxy_username}", mapping={"proxy_password": proxy_password, "hashed_token": hashed_token})

    # print out the result
    print(f"provider:{proxy_username}:", r.hgetall(f"provider:{proxy_username}"))
