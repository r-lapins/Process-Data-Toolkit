find_package(GTest REQUIRED)

function(add_pdt_test name source)
    add_executable(${name} ${source})

    if(source MATCHES "^csv/")
        target_link_libraries(${name} PRIVATE pdt_csv)
    elseif(source MATCHES "^wav/")
        target_link_libraries(${name} PRIVATE pdt_dsp pdt_io_wav)
    elseif(source MATCHES "^rtlsdr/")
        target_link_libraries(${name} PRIVATE pdt_io_rtlsdr)
    else()
        message(FATAL_ERROR "Unknown test source group for ${source}")
    endif()

    string(REGEX REPLACE "^test_" "" ctest_name "${name}")
    add_test(NAME ${ctest_name} COMMAND ${name})
endfunction()

add_pdt_test(test_stats_by_sensor csv/test_stats_by_sensor.cpp)
add_pdt_test(test_anomaly_zscore csv/test_anomaly_zscore.cpp)
add_pdt_test(test_anomaly_iqr csv/test_anomaly_iqr.cpp)
add_pdt_test(test_anomaly_mad csv/test_anomaly_mad.cpp)
add_pdt_test(test_dataset csv/test_dataset.cpp)
add_pdt_test(test_filter csv/test_filter.cpp)
add_pdt_test(test_report csv/test_report.cpp)
add_pdt_test(test_stats csv/test_stats.cpp)
add_pdt_test(test_time csv/test_time.cpp)
add_pdt_test(test_csv csv/test_csv.cpp)

add_pdt_test(test_peak_detection wav/test_peak_detection.cpp)
add_pdt_test(test_csv_export wav/test_csv_export.cpp)
add_pdt_test(test_window wav/test_window.cpp)
add_pdt_test(test_dft wav/test_dft.cpp)
add_pdt_test(test_fft wav/test_fft.cpp)

add_pdt_test(test_make_fft_backend wav/test_make_fft_backend.cpp)
target_link_libraries(test_make_fft_backend PRIVATE GTest::gtest_main)

add_pdt_test(test_wav_analysis_service wav/test_wav_analysis_service.cpp)
target_link_libraries(test_wav_analysis_service PRIVATE GTest::gtest_main)

add_pdt_test(test_fft_size_policy wav/test_fft_size_policy.cpp)
target_link_libraries(test_fft_size_policy PRIVATE GTest::gtest_main)

if(PDT_BUILD_RTLSDR)
    add_pdt_test(test_iq_ring_buffer rtlsdr/test_iq_ring_buffer.cpp)
    target_link_libraries(test_iq_ring_buffer PRIVATE GTest::gtest_main)

    add_pdt_test(test_iq_fft_backend rtlsdr/test_iq_fft_backend.cpp)
    target_link_libraries(test_iq_fft_backend PRIVATE GTest::gtest_main pdt_dsp)
endif()

add_executable(test_csv_cli_smoke test_csv_cli_smoke.cpp)
target_link_libraries(test_csv_cli_smoke PRIVATE pdt_csv_cli_args)
add_test(NAME csv_cli_smoke COMMAND test_csv_cli_smoke)

add_executable(test_wav_cli_smoke test_wav_cli_smoke.cpp)
target_link_libraries(test_wav_cli_smoke PRIVATE pdt_wav_cli_args)
add_test(NAME wav_cli_smoke COMMAND test_wav_cli_smoke)

if(PDT_ENABLE_CUDA)
    add_executable(test_cuda_smoke test_cuda_smoke.cpp)
    target_link_libraries(test_cuda_smoke PRIVATE CUDA::cudart CUDA::cufft)
    add_test(NAME cuda_smoke COMMAND test_cuda_smoke)
endif()
