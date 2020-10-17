# FiberHTTP

C++ http server with fiber. This library built using :
- boost::fiber
- libuv
- fiberio [https://github.com/hampus/fiberio](https://github.com/hampus/fiberio)
- cpp-httplib [https://github.com/yhirose/cpp-httplib](https://github.com/yhirose/cpp-httplib)

This library test compiled running on Linux and Windows.
Please note that this library framework not ready for any production.

This library was created to be used by Sultan POS 2.


# Requirement
- git, cmake, C++ compiler
- vcpkg and install package boost::fibers and libuv

# Compile
```bash
$ git clone --recursive https://github.com/apinprastya/fiberhttp
$ cd fiberhttp
$ mkdir build
$ cmake -DCMAKE_TOOLCHAIN_FILE=[your_vcpkg_folder]/scripts/buildsystems/vcpkg.cmake ..
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

# Todo
- [ ] Keep alive
- [ ] Optimize parser
- [ ] Adding database access and ORM
- [ ] Gracefull exit

# License
MIT

# Contribute
Any contributions are welcome
