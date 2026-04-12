#include <cuda_runtime.h>
#include <iostream>

int main()
{
    int count = 0;
    const auto err = cudaGetDeviceCount(&count);

    std::cout << "err=" << static_cast<int>(err)
              << " msg=" << cudaGetErrorString(err)
              << " count=" << count << "\n";

    return 0;
}
