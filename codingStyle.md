# Indentation
4 spaces

# Parentheses
Put them directly after the statement see:
```c
switch (suffix) {
case A:
    statement;
case B:
    statement;
default:
    statement;
}
```

## Functions
Only the parentheses of functions are different
```c
int foo(int x, char y)
{
    body of the function...
}
```

## If-Statement
Use parentheses if there are multiple statements.
```c
if (condition) {
    statements;
}
```
Don't use them if you only have one statement.
```c
if (condition)
    statement;
```
except you don't have only one statement in each if-body.
```c
if (condition) {
    statement;
    statement;
    statement;
    ...
} else if (condition) { <- use them here
    statement;
} else {
    statement;
    statement;
}
```

# Spaces
use spaces after these words:

- if
- switch
- case
- for
- do
- while

But don't use spaces after function names. Don't add spaces around statements inside parentheses

# Naming
local variables can be short but global variables must be clear
