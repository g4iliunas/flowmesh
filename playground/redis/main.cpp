#include <hiredis/adapters/libuv.h>
#include <hiredis/async.h>
#include <hiredis/hiredis.h>
#include <hiredis/read.h>

#include <iostream>
#include <string>
#include <string_view>

#include <unordered_map>

#include <uv.h>

using HashedToken = std::string;
using ProviderElements = std::unordered_map<std::string, std::string>;
// {hashed_token: {a: b, c: d}, ...}
using ProviderHashMap = std::unordered_map<HashedToken, ProviderElements>;

struct db_query {
    std::string hashed_token;
    ProviderHashMap &hashmap;
};

static void on_connect(const redisAsyncContext *c, int status)
{
    std::cout << "connected" << '\n';
}

static void on_disconnect(const redisAsyncContext *c, int status)
{
    std::cout << "disconnected" << '\n';
    // in flowmesh manager server we temporarily suspend the activity of the
    // database (signins, signouts..)
}

int main(void)
{
    uv_loop_t loop;
    uv_loop_init(&loop);

    redisAsyncContext *c = redisAsyncConnect("127.0.0.1", 6379);
    redisLibuvAttach(c, &loop);
    redisAsyncSetConnectCallback(c, on_connect);
    redisAsyncSetDisconnectCallback(c, on_disconnect);

    ProviderHashMap providers;

    std::string_view hashed_token =
        "b524b9cde2c29f25afa6c12b838ec0544f7c472128f5181c10ed4ea54929f318";
    std::string query = "HGETALL providers:";
    query += hashed_token;

    db_query q = {std::string(hashed_token), providers};

    redisAsyncCommand(
        c,
        [](redisAsyncContext *c, void *r, void *privdata) {
            redisReply *reply = reinterpret_cast<redisReply *>(r);
            if (!reply) {
                std::cout << "failed to fetch the result from db" << '\n';
                return;
            }

            std::cout << "elements: " << reply->elements << std::endl;

            db_query *q = reinterpret_cast<db_query *>(privdata);
            ProviderElements elements;

            for (int i = 0; i < reply->elements; i += 2) {
                // since this is a redis hashmap, all keys and values are
                // strings
                elements[reply->element[i]->str] = reply->element[i + 1]->str;
            }

            q->hashmap.insert({q->hashed_token, elements});

            // display the hashmap
            for (auto &it : q->hashmap) {
                std::cout << it.first << " ---\n";
                for (auto &it1 : it.second)
                    std::cout << it1.first << ": " << it1.second << '\n';
            }

            redisAsyncDisconnect(c);
        },
        reinterpret_cast<void *>(&q), query.c_str());

    uv_run(&loop, UV_RUN_DEFAULT);

    redisAsyncDisconnect(c);
    redisAsyncFree(c);
    return 0;
}