# reXscript 入门开发手册

## 目录

1. reXscript介绍
2. reXscript解释器的安装及使用
3. 基础语法与变量类型



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

