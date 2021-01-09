<p align="center">
  <img src="./res/logo_small.png" alt="KNG logo" width="250">
</p>

# KNG - Write beautiful & fast system code [![Documentation Status](https://readthedocs.org/projects/kng/badge/?version=latest)](https://kng.readthedocs.io/en/latest/?badge=latest)


KNG is a compiled, systems programming language designed around ease of use & flexibility for the programmer

```
user : interface {
    name : string;
    age : u32;
}

app.main : (args: string[]) exit_code : u32 {

    say_hello := (person : user)
        io.println "hello {person.name}!";

    james : user = ("james", "20");

    say_hello(james);

    exit_code = 0;
}

```

# Contributing
Everyone is welcome to contribute! As KNG is in early stages of development, pull requests are the current way of contributing. Later on in KNGs lifecycle we may adopt a more 'professional' approach but for now PRs are welcome :)


# Building

see BUILDING.md