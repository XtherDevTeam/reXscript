# The implementation of reXscript builtin hashMap

### Structure 结构

`hashMap()` 会返回一个 `Object` 包含以下几个成员: 

- `kvPairs` 是一个包含 `Key-Value Pairs` 以及 `key` 的哈希值 的 `Vec` 数据结构，用于全图遍历。
- `hashT` 是一张哈希表，`initialSize` 为 10。当 `kvPairs.length() > 2 * hashT.length()` 时会进行 `reallocate`。

以及一些方法: 

- `insert(k, v)` 会插入一对键值到 `hashMap`。
- `remove(k)` 会删除键为 `k` 的键值。
- `rexIndex(k)` 会返回键为 `k` 的 `value` 的引用。
- `keys()` 会返回一个包含了 `rexIter` 方法的数据结构，用于遍历整个 `hashMap` 结构的键。
- `rexIter()` 会返回一个用于遍历 `hashMap` 中 `kvPairs` 的迭代器。

### 原理

- 插入

当 hashMap 处理一次插入操作时，会对 `key` 进行哈希操作并将键值对存入 `kvPairs`，并对 `key` 的 `hash` 值进行取模后的值记为 `H'` ，放入 `hashT[H']` 对应的单元格中。

- 删除

只需删除单元格的 `LinkedList` 里面包含了指定 `kvPairs` 引用的元素即可。

- 查找

将所查找的 `k` 进行哈希取模过后的值记为 `k'` ，对 `hashT[k']` 链表进行遍历，对 `k` 进行比较后找到指定 `kvPair` 并返回引用。

- 重分配

在 `kvPairs.length() > 2 * hashT.length()` 的时候，`hashMap` 会自动重分配 `hashT`，调整容量为 `kvPairs.length()`，重新对所有键的 `hash` 进行取模，然后放入新的单元格。

### 特殊情况的处理

- 懒分配

一般情况下，分配 `hashT` 的时候并不会立即初始化所有的桶为链表，在执行插入操作的时候，当发现指定 `bucket` 没有被分配时，会自动初始化该 `bucket` 为 `linkedList` 并插入键值对到指定 `linkedList` 中。

