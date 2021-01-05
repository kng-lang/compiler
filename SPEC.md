



# spec

# WARNING
KNG is in very early stages of development and only a few outlined features are implemented. KNG currently supports:
- function declerations & definitions (no paramaters or return types, only top level)
- constant declerations & assignments
- variable declerations & assignments
- s32 literal type (no other types currently supported)

# general syntax
statements can end with either a ; or \n

## types
These are the basic types


u8
u16
u32
s8
s16
s32
s64
f32
f64
char
interface


# variables

we can declare a variable as a type, and optionally initialise it

```
x : u8      
x : u8 = 1
x := 1
```

## constants
constants are variables that can only be assigned to one
```
x : u8 1
x : 1     // infers type of s32
```

# scopes


# functions
functions are first class objects

## constant fn's
to declare a fn that will always be bound to a variable, we declare it as a constant
```
x : () {
    io.println "fn bound to x!"
}
```
## lambdas
if we want the ability to rebind x to another fn, we need to declare x as a variable and assign a lambda to it (a lambda is just a fn that isn't bound to a variable)
```
x := (){
    io.println "fn that can be re-bound!"
}
```

# global behaviour
Unline C, global variables can be initialised with any aribtary expression
```
x : s32 = 123
y := x * 3
```

we can also call functions at the global scope

```
x : fn () 321
y := x()
```

# custom data types
say we want to declare a c-style struct which behaves as a vector

```
vec : interface = {
    x : f64 = 0
    y : f64 = 0

    vec := ()
    vec := (x : f64, y : f64) this.x = x; this.y = y
    add := (other : vec) vec return vec(this.x + other.x, this.y + other.y)
}

```

this assigns the vec variable to an instance of this structure, note. vec is just a variable, it can be changed (the structure signature is saved seperately in the compiler completely independently of the vec variable).

we can then create a copy of this using the constructor. note vec is now bound to a type and so we can declare it as a type specifier.

```
instance_1 : interface = vec()
instance_2 : vec       = vec()

```

we can also declare interface members outside of the actual body somewhere else

```

vec : interface = {

}

vec.x : f64 = 0
vec.y : f64 = 0

```

note this is different from assigning to an interface value

```
vec.x = 0
```

## oop behaviour

Custom data types in kng takes inspiration from go interfaces, however, instead of go interfaces & structs kng only has interfaces.

say we want to implement animal oop...

```
animal := {
    speak : fn 
}


dog := {
    speak : fn () = "woof!"
}

make_animal_speak := (animal : ^animal) animal.speak()

make_animal_speak (dog())

```

when we call make_animal_speak, at compile time, the interface is checked to see if it implements all the members of the animal interface. if it does, then dog.speak is called using the vtable of dog.
note, this only works with pointers

# pointers

we declare pointers using the ^ character
```
arr : ^u8
arr : ^u8 = {'a', 'b', 'c'}
```
we can then take the address of a value using the & character

```
arr : ^vec = &vec() 
```

unlike c, we can use the . accessor for both literals and pointers
```
v1 : vec  = vec()
v2 : ^vec = &vec()
v1.x
v2.x
```

# building
like app.main, the app.build function tells kng how to build the app. the app.build fn can either be in a seperate file or inside the current app file

```
@import "builder"

app.build := (){
    b := builder()
    b.name = "my_app"
    b.add_source({"a.k", "b.k"})
    b.add_lib({"my_lib.lib"})
    b.optimise(b.optimiser.full)
    
    b.build()      // builds the app to an exe but doesnt run it
    b.run()        // builds the app to an exe and runs it
    b.jit()        // jit's the app and runs it
}
```

# garbage collection
we can enable garbage collection in the build options. if the compiler sees a structure will be used outside of the current scope, it will heap allocate it and the gc runtime (on another thread) will deallocate it when ready

```
fn_1 := (v : ^vec) io.println v.x

main := () {
    v := &vec() // this is turned into a heap allocation by the compiler (v := new vec)
    fn_1(v)
}
```
in this example, the compiler can see that v in main is used outside of main (passed to another function) and so it heap allocates it

# reflection

## typeof

## sizeof

## details


# includes

# compiler directives

## run
run runs an expression at compile time and inserts the compiled value (this is not the same as text replacement!)

```
do_maths := () 1+2+3
num := @run do_maths()
```

## align
the align directive tells the compiler how to align the members of a struct

union creates a c style union

```
@align "union"
data : interface = {
    x : u8
    y : u32
    z : u8
}

// creates a struct of 4 bytes as u32 is 4 bytes

```

packed packes the data types together and ignores word alignment

```
@align "packed"
data : interface = {
    x : u8
    y : u32
    z : u8
}

// creates a struct of 8 bytes as u32 is 4 bytes, x and z can be packed into a word of 4 bytes
```

aligned aligns the members on word boundaries

```
@align "aligned"
data : interface = {
    x : u8
    y : u32
    z : u8
}

// creates a struct of 8 bytes as u32 is 4 bytes, x and z can be packed into a word of 4 bytes
```