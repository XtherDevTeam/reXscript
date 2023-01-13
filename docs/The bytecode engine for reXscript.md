# The bytecode engine for reXscript

### 引言

**为什么要实现一个字节码引擎？**

在一次 `Peformance Test` 中，`reXscript` 的表现比 `Python 3` 的结果慢一倍，为了提升解释效率，我决定开发一套字节码引擎。

### 指令集

在这次的 `Bytecode Engine` 开发中，为了方便，我去掉了运算指令的类型。

同时，为了在字节码表示字符串，我引入了全局常量池 (`Global String Constants Pool`)，`String Literals` 在编译时会存入 `GCP`，直到被 `stringNew` 初始化为字符串对象。

**Instructions**

- `pushLocalCxt param[T=null]`

  初始化新的局部上下文。

- `popLocalCxt param[T=null]`

  弹出局部上下文。

- `jump param[T=int]`

  根据参数指定的偏移量跳转到指定的指令。

- `jumpIfTrue param[T=int]`

  当栈顶元素可以被转换成 `boolean` 的 `true` 值时，跳转到指定的偏移量。

- `jumpIfFalse param[T=int]`

  当栈顶元素可以被转换成 `boolean` 的 `false` 值时，跳转到指定的偏移量。

- `pushExceptionHandler param[T=int]`

  压入 Exception Handler，参数为指向 Handler 的 `pushLocalCxt` 命令的偏移量。

- `popExceptionHandler param[T=null]`

  弹出当前 `Exception Handler`，在 `Try-block` 结束后被生成。

- `invoke param[T=int]`

  根据命令的参数，将栈顶指定数量的元素弹出，调用此时栈顶的 `Callable-object`。

- `ret`

  将栈顶的元素作为返回值，退出函数执行。

- `find param[T=gcpString]`

  在 `localCxt`、`moduleCxt` 和 `globalCxt` 中寻找指定标识符的值。

- `findAttr param[T=gcpString]`

  在栈顶元素的 `member` 中寻找指定标识符的值，将其引用压入栈顶

  **Notes:** 对于方法的 `invoke` 可以 `duplicate` 栈顶元素，然后再 `findAttr` 再调用 `invokeMethod`。

- `index param[T=null]`

  弹出栈顶的一个元素，将其作为索引与当前栈顶的元素执行 `index` 操作。

- `invokeMethod param[T=int]`

  根据命令的参数，将栈顶指定数量的元素弹出，调用此时栈顶的 `Callable-object`，栈顶的后一个元素将为这个 `Callable-object` 的 `this` 指针。

- `opXXX param[T=null]`

  对应原 `interpreter` 的 `opXXX` 系列命令。取出指定数量的值进行计算后将值压入栈

- `assign param[T=gcpString]`

  取出栈顶的值，将其赋值给此时栈顶的值。

- `deepCopy param[T=null]`

  将栈顶的值取出进行深拷贝，压入栈。
  
- `intConst param[T=int]`

  压入整数常量
  
- `deciConst param[T=int]`

  压入浮点数常量
  
- `boolConst param[T=bool]`

  压入布尔值常量
  
- `nullConst param=[T=null]`

  压入 `null` 型常量
  
- `stringNew param[T=gcpString]`

  从常量池中创建 `string` 对象
  
- `arrayNew param[T=int]`

  从栈上倒序取出指定数量的元素组成 `Array object` 然后压入栈。

- `objectNew param[T=int]`

  在栈上取出指定数量的键值对组成 `object` 然后压入栈

- `funcNew param[T=int]`

  从栈顶获取 `gcpIndex` 作为 `gcpBytecodeArr`，再获取指定数量个 `gcpIndex` 作为 `gcpString` 为参数的标识符。

- `lambdaNew param[T=int]`

  从栈顶获取一个 `funcObject`，作为函数体，再获取指定数量个 `gcpIndex` 作为 `gcpString` 为 `outerArgs` 的标识符。

