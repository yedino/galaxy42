cmake_minimum_required(VERSION 2.8.3)

MESSAGE(STATUS "cmake in drafts")

add_subdirectory(stream_fast_sign)
add_subdirectory(bit_payments)
add_subdirectory(market_client)
add_subdirectory(ed25519)
add_subdirectory(market_server/server)
add_subdirectory(market_server/crypto)
add_subdirectory(market_server/hash_table)
add_subdirectory(market_server/market)
add_subdirectory(ring_signature)