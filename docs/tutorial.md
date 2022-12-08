# Variables

In reXscript, there are `null`, `int`, `deci`, `bool`, `str`, `vec`, `object`, `func`, `lambda`, `nativeFunc`, and `ref`.

To create or declarate a variable, you can use `letStmt`.

E.g. `let sth = 1`

In this example, you created a `int` value in stack named `sth`.

Like this, you can create a string value in stack by `let sth = "balahbalah"` or create a decimal value in stack by `let sth = 1.114514`.

Also, it's easy to create a boolean value by `true` or `false`.

According to the source code:
```c++
if (!stack.empty()) {
    stack.back().localCxt.back()[item.child[0].leaf.strVal] = managePtr(rhs);
} else if (moduleCxt) {
    moduleCxt->members[item.child[0].leaf.strVal] = managePtr(rhs);
} else {
    env->globalCxt->members[item.child[0].leaf.strVal] = managePtr(rhs);
}
```

We know, if the stack is empty, then it will try to create variables in the `moduleCxt`, if `moduleCxt` is null, then it will create in `globalCxt`. 

# Expressions

