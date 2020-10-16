# fiberhttp

C++ http server with fiber. This library build using :
- boost::fiber
- libuv
- fiberio [https://github.com/hampus/fiberio](https://github.com/hampus/fiberio)
- cpp-httplib [https://github.com/yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)


# Requirement
- git, cmake, 
- vcpkg and install package boost::fibers and libuv

# Compile
```bash
$ git clone --recursive https://github.com/apinprastya/fiberhttp
$ cd fiberhttp
$ mkdir build
$ cmake DCMAKE_TOOLCHAIN_FILE=[your_vcpkg_folder]/scripts/buildsystems/vcpkg.cmake ..
$ make
```

# Example

```cpp
#include <iostream>
#include <fiberhttp.h>

int main()
{
    fiberhttp::FiberHttpServer server;
    server.get("/", [](const fiberhttp::Request &req, fiberhttp::Response &res) {
        res.set_content("{\"fiberhttp\":true}", "application/json");
    });
    server.run("0.0.0.0", 8990);
    return 0;
}
```