add_executable(test_stats test_stats.cpp)
target_link_libraries(test_stats PRIVATE pdt)

add_test(NAME stats COMMAND test_stats)
