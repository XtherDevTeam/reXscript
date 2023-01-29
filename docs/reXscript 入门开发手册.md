# reXscript 入门开发手册

## 目录

1. reXscript介绍
2. reXscript解释器的安装及使用
3. 基础语法与变量类型
4. 基础语句和流程控制
5. 多线程的使用



## reXscript 介绍

### reXscript的简介

reXscript是一种具有很好的扩展性的动态类型、解释性函数式语言。

- **reXscript是一门动态类型语言: **这意味着你可以在运行时更改变量的类型，降低了编写代码时的思考难度，这也意味着reXscript没有设计 `泛型 (Generic)` `模板 (Template)` 的必要。
- **reXscript是一门纯解释型的语言: **这意味着reXscript不会生成 `Bytecode` 的为了提升运行效率的中间表示形式，同时这也意味着reXscript的运行效率不能做到像`C++`等编译型语言的高效。

### reXscript的特点

- **易于上手: **语法上为 `C-like` 风格，熟悉 `JavaScript` 等语言的人员可以很快掌握 `reXscript` 的基础语法。
- **跨平台: **解释器可以在所有使用 `POSIX` 标准以及拥有 `Clang`, `gcc` 等现代 `C++` 编译器的计算机上编译，实现 `一次编写，多处运行`。
- **可拓展: **用户可以轻松地剪裁 `reXscript` 解释器，使其运行在某些特定的平台上，同时，也可以轻松地为 `reXscript` 的内置类型增加新的方法，向全局作用域添加自己的函数。



## reXscript解释器的安装及使用

此处参考 README.md



## 基础语法与变量类型

先从一个简单的 reXscript 程序开始讲起。新建一个名为 `hello.rex` 的文件并向其写入如下 reXscript 代码:

```javascript
let rexModInit = func() {
  let a = 114514, b = 114.514, c = "this is a string", d = false;
  print(a, " ", b, " ", c, " ", d, "\n");
};
```

在上面的代码中我们定义了一个 rexModInit 变量，其值为一个函数。

rexModInit是每个 `.rex` 文件的入口，在脚本被加载的时候，reXscript 解释器就会调用这个函数来运行这个 rex 文件.

一般地，函数可以被传入参数，如上面代码的 `print` 函数，它可以被传入多个参数。

在 `rexModInit` 中，我们定义了三个变量，分别为 整型变量 `a`, 浮点型变量 `b`, 字符串型变量 `c`, 布尔型变量 `d`.

`114514`, `114.514`, `"this is a string"`, `false` 等为字面量 (Literals)，在 `reXscript` 会转换成相应的类型。

上面代码的运行结果应为 `114514 114.514 this is a string 0`

特殊地，reXscript还有一种特殊的类型: `字节型 (bytes)`，这是一种特殊的内置类型，他没有字面量，只可以通过字符串型变量内置的 `encode(charset)` 函数调用 `charsets` API 来获取字节型的值。

E.g.

```javascript
let rexModInit = func () {
  let a = "こんにちは、reXscript！".encode('utf-8');
  print(a);
  return 0;
};
```

该 reXscript 文件中使用字符串将其编码为 `utf-8` 字符编码 初始化了一个变量 `a`, 然后打印了该变量，对应的输出应为 `\xe3\x81\x93\xe3\x82\x93\xe3\x81\xab\xe3\x81\xa1\xe3\x81\xaf\xe3\x80\x81reXscript\xef\xbc\x81`。

此外，reXscript还有 `动态数组 (vec)`, `对象 (object)` 等内置类型。

E.g.

```javascript
let rexModInit = func () {
  let a = ["this", "is", "a", "list", 114514];
  let b = {
    member1: "it works",
    "member2": "it also works",
    "member 3": "it works too!"
  };
  print(a, "\n");
  print(b, "\n");
  return 0;
};
```

上面代码定义了变量 `a` 和 变量 `b`，变量 `a` 是一个数组，包含了五个元素：字符串 `"this"`、`"is"`、`"a"`、`"list"` 和整数型 `114514`。

变量 `b` 是一个对象，包含了三个成员：

- "member1"：值为字符串 "it works"
- "member2"：值为字符串 "it also works"
- "member 3"：值为字符串 "it works too!"

最后，程序会输出

```javascript
["this","is","a","list",114514]
{"member 3": "it works too!","member1": "it works","member2": "it also works"}
```

你可以使用 下标表达式 `[]` 来访问对象，如 `b["member 3"]`；也可以用 成员表达式 `.` 来访问对象如 `b.member1`。注意：成员表达式仅可用于符合 标识符 定义的成员，如 `member1` `member2`。

在定义的时候，一个键值对的左端可以为字符串和标识符，两者是等效的。

动态数组也可以使用 `forEach` 进行遍历

```javascript
forEach (i in a) {
  print(i, " ");
}
```

以上程序将会输出 `this is a list 114514`。

以下是一些内置类型的内置方法：

String 类型：

| 方法名       | 作用                                                         | 调用格式                   |
| ------------ | ------------------------------------------------------------ | -------------------------- |
| substr       | 截取字符串的某一部分                                         | str.substr(start[, count]) |
| startsWith   | 判断字符串是否以指定字串为开头                               | str.startsWith(b)          |
| endsWith     | 判断字符串是否以指定字串为结尾                               | str.startsWith(b)          |
| charAt       | 获取某一位置上的字符编码                                     | str.charAt(idx)            |
| fromChar     | 从字符编码初始化字符串                                       | str.fromChar(ch)           |
| length       | 获取字符串长度                                               | str.length()               |
| rexEqual     | 判断字符串是否相等的内置算符 `==` 重载                       | str == b                   |
| rexNotEqual  | 判断字符串是否不相等的内置算符 `!=` 重载                     | str != b                   |
| rexAdd       | 拼接两个字符串的内置算符 `+` 重载                            | str + b                    |
| rexAddAssign | 与字串 `b` 进行拼接并将结果保存到 `str` 的内置算符 `+=` 重载 | str += b                   |
| join         | 使用 `str` 对参数中的字符串进行拼接                          | str.join(a, b, ...)        |
| encode       | 将字符串编码为指定格式并返回编码后的 `bytes` 对象            | str.encode(charset)        |
| trim         | 去除字符串两端的空白字符                                     | str.trim()                 |

Vec (动态数组) 类型：

| 方法名      | 作用                                               | 调用格式              |
| ----------- | -------------------------------------------------- | --------------------- |
| append      | 加入元素并返回数组自身的引用                       | vec.append(a, b, ...) |
| pop         | 弹出尾部元素并返回数组自身的引用                   | vec.pop()             |
| remove      | 删除在数组中首次出现的指定元素并返回数组自身的引用 | vec.remove(ele)       |
| removeAll   | 删除数组中出现的全部指定元素并返回数组自身的引用   | vec.removeAll(ele)    |
| length      | 返回数组长度                                       | vec.length()          |
| rexEqual    | 判断两个 `vec` 是否相同的重载                      | vec == b              |
| rexNotEqual | 判断两个 `vec` 是否不相同的重载                    | vec != b              |
| rexIter     | 返回一个迭代器用于遍历数组                         | vec.rexIter()         |

`Bytes` 类型：

| 方法名      | 作用                                         | 调用格式              |
| ----------- | -------------------------------------------- | --------------------- |
| length      | 返回字节型的长度                             | bytes.length()        |
| decode      | 解码字节型为字符串，接收一个参数代表字符编码 | bytes.decode(charset) |
| concat      | 拼接两个字节型并返回自身引用                 | bytes.concat(b)       |
| rexEqual    | 判断两个字节型是否相等的重载                 | bytes == b            |
| rexNotEqual | 判断两个字节型是否不相等的重载               | bytes != b            |

`linkedList` 类型：

| 方法名    | 作用                           | 调用格式                  |
| --------- | ------------------------------ | ------------------------- |
| append    | 加入一个元素到链表尾部         | linkedList.append(ele)    |
| pop       | 弹出链表尾部的元素             | linkedList.pop()          |
| remove    | 删除在链表中首次出现的指定元素 | linkedList.remove(ele)    |
| removeAll | 删除在链表中出现的所有指定元素 | linkedList.removeAll(ele) |
| rexIter   | 返回一个迭代器用于遍历链表     | linkedList.rexIter()      |

## 基础语句和流程控制

`reXscript` 的模组由顶层结构和一个入口函数组成。

**顶层结构**: 即文件所不被大括号包含，在模组作用域范围的语句集合。

**入口函数**: 顶层结构中的 `rexModInit` 函数。

### let

`let` 语句是 reXscript 中一个重要的语句，他的作用是声明和定义变量。当然，也可以复写一个变量的值。

E.g.

```js
let i = 114514; // 定义一个值为 114514 的变量 i
print(i); // 114514
let i = "Overwriting"; // 复写 i 的值为 "Overwriting"
print(i); // Overwriting
let a = 1919, b = 0.81; // 同时定义多个变量
print(a, " ", b); // 1919 0.81
```

### if

`if` 语句是根据条件真假执行相应代码的语句，当条件为真时，执行 `if` 的代码块，反之，执行 `else` 的代码块（`else` 的代码块可以被省略）

`let` 会在当前代码的作用域进行定义和复写变量。

每一个代码块都会创建一个局部作用域，局部作用域可以访问所有局部作用域的变量，但不同局部作用域的 `let` 语句不能修改另外的作用域的变量。

函数调用时产生的作用域有两个，一个是栈帧作用域（存放参数），另一个是代码块的局部作用域。

E.g.

```js
let RootCuiAKIOI = true;
if (RootCuiAKIOI) {
  print("cyy nb!");
} else {
  print("that's impossible");
}
```

### while

`while` 语句则是当条件为真时重复执行代码块直到条件为假。

E.g.

```js
let i = 0;
while (i < 10) {
  print("RootCui AK IOI\n");
  ++i;
}
```

### for

`for` 语句则像一个加强版的 `while` 语句，在 `while` 的功能基础上，可以在循环开始之前执行语句 `stmt1`，在代码块执行完毕后执行语句 `stmt2`，然后重复进行条件判断，直到条件为假时退出。

E.g. 与上面代码等效

```js
for (let i = 0; i < 10; ++i) {
  print("RootCui AK IOI!\n");
}
```

**P.S.** `for` 语句会额外创建一个局部上下文，用于保存在 `stmt1` 中可能出现的 `let` 语句所创建的变量。

### break

`break` 语句用于退出循环。

E.g.

```js
for (let i = 0;i < 10; ++i) {
  print("RootCui AK IOI\n");
  if (i > 5) {
    break;
  }
}
```

代码会重复执行6次 `for` 代码块然后退出循环。

### continue

`continue` 语句用于跳转到代码块结尾，重新开始新一轮循环。

E.g.

```js
for (let i = 0;i < 10; ++i) {
  continue;
  print("I won't be executed");
}
```

上面代码的 `print` 语句永远不会被执行，因为 `continue` 在执行完 `stmt2` 后跳转到了下一次循环。

### return

`return` 用于函数的返回。

E.g.

```js
func foo() {
  return 1 + 1 + 4 + 514;
}
print(foo()); // 520
```

函数在不执行 `return` 语句的情况下退出的话会返回 `null`。

### with

`with` 语句用于保证资源在使用完毕后得到释放，即使代码块内部执行了 `break` `continue` `return` 等语句都会先释放资源，再进行流程的跳转。

使用 `with` 语句的对象必须实现 `rexInit` `rexFree` 方法用于释放资源。

`rexInit` 在执行代码块之前被调用，`rexFree` 在代码块结束或运行时发生异常时被调用。

E.g.

```js
func foo() {
  with (i : std.fs.open("test.txt", "r")) {
    return std.json.loads(i.read(i.length));
  }
}
```

## 多线程的使用

`multi-threading` 是一个在现代语言非常常见的概念，它可以实现同一个进程同时运行多个任务，`reXscript` 也提供了线程库用于 `multi-threading`。

使用 `threading.start(callable, arg1, arg2, ...)` 用于开启一个线程，返回该线程的 `ID`。

获取当前线程的ID可以读取存储在解释器作用域的 `thread_id`。

使用 `threading.wait(threadID)` 来等待一个线程的结束，并获取返回值。

E.g.

```js
func foo() {
  let counter = 0;
  let result = threading.start(lambda (counter) -> (str) {
    while (outer.counter < 5) {
      print("Count ", outer.counter, ": ", str, "\n");
      ++outer.counter;
    }
    return 1919.810;
  }, "Hello, world!");
	print(threading.wait(result), "\n");
}
```

程序会运行五次 `lambda` 里的 `print` 后会返回 `1919.810` 回到主线程，输出线程结果。

### mutex

在程序运行的时候可能会出现两条线程访问同一资源的情况，这时候就可能造成写入数据损坏，可以用到 `mutex` 来对资源加锁。`mutex` 能够将资源限制为同一时间只有一条线程使用。

`mutex` 在已经加锁时，如果另一条线程试图加锁，则会阻塞该线程直到另一线程解锁。

E.g.

```js
let counter = 0;
let lock = mutex();
func foo() {
  with (_ : lock) {
    print("Thread ", thread_id, ": Count ", outer.counter, ": ", str, "\n");
  	++outer.counter;
  }
  return null;
}

func rexModInit() {
  let a = threading.start(foo), b = threading.start(foo);
  threading.wait(a);
  threading.wait(b);
  lock.finalize(); // destroy lock object
  return null;
}
```

以上代码演示了 `mutex` 的基本使用，各位可以尝试一下去掉 `with` 语句后程序会输出什么。

此外 `mutex` 还有 `lock` `tryLock` 等方法：`lock` 为 `rexInit` 函数的别名，用于加锁。`tryLock` 也是加锁，立即返回结果，`true` 为成功，`false` 为失败。

在使用完成后，需要调用 **finalize** 释放锁的内存，该操作是 **必须** 的。
