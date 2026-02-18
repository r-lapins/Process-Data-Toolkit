add_executable(test_stats test_stats.cpp)
target_link_libraries(test_stats PRIVATE pdt)
add_test(NAME stats COMMAND test_stats)

add_executable(test_csv test_csv.cpp)
target_link_libraries(test_csv PRIVATE pdt)
add_test(NAME csv COMMAND test_csv)
