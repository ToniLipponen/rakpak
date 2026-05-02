Work in progress definitions for concepts the build system uses.

# Project
*Project* is a collection of *build targets*, imported *packges* and all things needed for building.

# Package
A *package* is a project that *provides* modules using the *"provides"* section in the projects config.
*Packages* can *provide* *modules* from other *packages* they themselves *import*.

## Rules
- A *package* **must** *provide* **atleast** one *module*.
- A *package* **must** have a name

# Provides
*Provides* are a collection of *modules* a *package* exports/makes visible for use in other *projects*.

# Build target
A *build target* is a unit of code inside a *project*. They can be libraries or executables.

They are internal to a *project*, and cannot be referenced by other *projects*.

A *build target* can reference other *build targets* within the same *project* as *dependencies*.

*Build targets* can declare properties visible outside its *project*. These public properties get merged into a *modules* *usage requirements* when exported by a *project* with its *"provides"* section.

# Module
A *module* is a unit of reusable code exported by *packages* using the *"provides"* section.

A *module* consist of one or multiple *build targets*.

*Modules* have *usage requirements*, things a *build target* must satisfy in order to use a *module*.

A *module* does **not** have to be compiled code (build artifacts), it could be just be include paths and/or defines (in case of header only libraries).

# Usage requirements
*Usage requirements* are a collection of include path, link/build flags and defines a *build target* must include in their build process. This happens automatically when declaring a *module* as a *dependency*.

# Important distinctions
- Modules are **not** the same as build targets. 
    - Build targets are projects internal implementation details. While modules are a collection of build targets exported by a package.
