# Usage of generating FFI

FFI generator is a tool to automate the generation of C-style FFI to reXscript native modules.

By writing FFI description files, FFI generator can quickly generate a C-style library of reXscript native modules.

### Basic FFI description file structure:

```rexffi
ffi-lib: hello.h ; complete with your own library headers

ffi-types:
; just define your own structure and write down how to covert to rex::value
    i32:
        c-style: int
        type-to-rex: (value){(vint)($param)}
        rex-to-type: (i32)($param.getInt())
    :i32
:end-ffi-types

ffi-exports:
; write down the export functions below
    name: add
    params: i32,i32
    result: i32 ; if the function returns nothing, fill this place with `none`
:end-ffi-exports

ffi-helpers:
; write down your type casting helper here.
:end-ffi-helpers
```

After writing the FFI description file, you can generate the corresponding library header file with `rex --generate-ffi [file]`.

### To build the full native module

In the above you have finished writing the native module headers, next you need to clone the `http://www.xiaokang00010.top:4001/xiaokang00010/rexNativeModule` repository locally and make some modifications.

Go to the repository directory, delete `library.h`, and overwrite `library.cpp` with your generated file, and you are now ready to use the `cmake` tool to generate the reXscript native modules for your project.