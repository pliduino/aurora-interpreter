# c-interpreter

Interpreter made in C. 

A lexer turns your file into tokens which in turn get parsed into a 128 bit instruction to be used by the interpreter.

## Features

Currently supports assigning, printing and creating 32 bits integer and float values.

```
  var i32 test_int;
  test_int = 3;
  print(test_int);

  var f32 test_float;
  test_float = 5.0;
  print(test_float);
```
