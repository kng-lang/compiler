<p align="center">
  <img src="./res/logo_small.png" alt="KNG logo" width="250">
</p>

# KNG - Write beautiful & fast system code

KNG is a compiled, systems programming language designed around ease of use & flexibility for the programmer

```
user : interface {
    name : string
    age : u32
}

app.main : (args: string[]) exit_code : u32 {

    say_hello := (person : user)
        io.println "hello {person.name}!"

    james : user = ("james", "20")

    say_hello(james);

    exit_code = 0
}

```

# Contributing
Everyone is welcome to contribute! As KNG is in early stages of development, pull requests are the current way of contributing. Later on in KNGs lifecycle we may adopt a more 'professional' approach but for now PRs are welcome :)


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