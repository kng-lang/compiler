// KNGs build system is entirely native, building is done using KNG itself within a build.k file

@import "build"

app.build : () {
    builder := Builder()
    builder.add_src "helloworld.k"
    builder.optimise Builder.OPTIMISE_FULL
    builder.output "helloworld.exe"
    builder.compile_and_run
}