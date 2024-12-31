# Redis database layout
## Provider
- `hashmap: provider_hash:<hashed_token> bytes_sent <int> bytes_received <int>`
- `hashmap: provider:<proxy_username> proxy_password <str> hashed_token <str>`
## Consumer
- `hashmap: consumer:<username> hashed_password <str> balance <int>`
- `set: consumer_proxies:<username> <provider_hashed_token1> <provider_hashed_token2> ...`