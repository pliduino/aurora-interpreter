# Aurora Interpreter

Interpreter made in C.

A lexer turns your file into tokens which in turn get parsed into a 128 bit instruction to be used by the interpreter.

## Features

Currently supports assigning, printing and creating integer, float and char values, it also supports primitive functions without arguments.

```
  fn test_func(){
    var i32 test_int;
    test_int = 3;
    print(test_int);
  }

  test_func();

  var f32 test_float;
  test_float = 5.0;
  print(test_float);
```

## Options
```
  -c --compile   -> Pre-compiles code into bytecode
  -x --execute   -> Executes pre-compiled code
  -t --transpile -> Transpiles your code to C (Does not support function for now)
```

## Goals
- [ ] Use LLVM instead of transpiler
- [ ] Add flow control
- [ ] Add loops
- [ ] Add AST to lexer so parser can compound commands