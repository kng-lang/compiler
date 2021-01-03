<p align="center">
  <img src="./res/logo_small.png" alt="KNG logo" width="250">
</p>

# KNG - Write beautiful & fast system code



```
user : interface {
    name : string
    age : u32
}

app.main := (args: string[]) exit_code : u32 {

    name := args[0]
    age := u32(args[0])

    person : user = (name, age)

    io.println "hello {person.name}, nice to meet you!"

    exit_code = 0
}

```


# Building

kng uses CMake as its build system. CMakeLists.txt in the root directory of the repository will build both kng.compiler & kng.runtime. To build either kng.compiler or kng.runtime, use the CMakeLists.txt in the respective directory.

### Unix Makefile

To build a makefile, specify "Unix Makefiles" as the generator

```
cmake -B ./build -G "Unix Makefiles"
```

### Visual Studio 16 2019

To build a Visual Studio solution, specify a visual studio version as the generator

```
cmake -B ./build -G "Visual Studio 16 2019"
```