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

Expressions in reXscript 
```c++
memberExpression,           // member expression like aaa.bbb.ccc,
                            // both 2 terms in expression are subscriptExpression
                            // primary means match a literal or memberExpression
uniqueExpression,           // like -[primary], --[primary], ++[primary], ![primary]
multiplicationExpression,   // multiplication expressions like a * b, a / b, a % b
additionExpression,         // addition expressions like a + b, a - b
binaryShiftExpression,      // like a << b, a >> b
logicEqualExpression,       // like a == b, a != b, a >= b, a <= b, a > b, a < b
binaryExpression,           // like a | b, a & b, a ^ b,
logicAndExpression,         // like a && b, a || b,
assignmentExpression,       // like a = b, a += b, a -= b, a *= b, a /= b, a %= b
```

E.g. `a + (b.a + c.b) * g / 1.2 << k & 1`

# Callable objects

Callable objects in reXscript are like `func`, `lambda`, `objects` that implmented `rexInvoke` func.

## Functions

Syntax: `func (funcArgs) {codeBlock}`

Functions is a callable object. The caller can pass values to the function (params), and the codes in the codeBlock will be executed. 

## Lambda

Syntax: `lambda (outerArgs) -> (funcArgs) {codeBlock}`

outerArgs will be copied and saved in the members of lambda object, to access outerArgs in lambda object, you can use `outer.varname`.

outerArgs will be persisetent saved in the lambda object.

E.g.

```rex
let a = 114, b = 514;
let lam = lambda(a, b) -> (c) {
    a += c;
    b += c;
    return a + b;
};
print(lam(1919), " ", lam(810), "\n");
```

The above codes will print `4466 6086` on the screen.

# Extended features

## Iterator

Iterators provide a way to traverse data structures

These abstractions should be included in iterators: `isEnd`, `next`.

Like:

```rex
let iter_test = func() {
    let data = {
        num: 100,
        rexIter: func() {
            let it = {
                cur: 0,
                container: this,
                next: func() {
                    let cur = this.cur;
                    if (this.cur > this.container.num) {
                        break;
                    }
                    ++this.cur;
                    return cur;
                }
            };
            return it;
        }
    };
    forEach (i in data) {
        print(*i, "\t");
    }
    return 0;
};
```

## References and deepCopy

Generally, when reXscript initialize an object from another object. It'll be constructed by reference instead of copy.

So some code might not work properly in reXscript like the following example.

```html
input> let lst = [1,2,2,3,3,3,4,4,4,4,5,5,5,5,5]
output> <rexValue type=null>
input> let b = lst
output> <rexValue type=null>
input> b
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
input> lst
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
input> lst.removeAll(4)
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
input> b
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
```

As you can see, reXscript constructed object `b` by reference and the changes on the original object `lst` can affect on `b`.

To avoid this, you can use deepCopy to construct an object like

```html
input> let lst = [1,2,2,3,3,3,4,4,4,4,5,5,5,5,5]
output> <rexValue type=null>
input> let b = *lst
output> <rexValue type=null>
input> lst
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
input> b
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
input> lst.removeAll(4)
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
input> lst
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
input> b
output> <rexValue type=ref val=<rexValue type=vec val=[<rexValue type=int val=1>,<rexValue type=int val=2>,<rexValue type=int val=2>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=3>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=4>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>,<rexValue type=int val=5>]>>
```

In this time, reXscript constucted `b` by copy, so the changes on `lst` couldn't affect on `b`.