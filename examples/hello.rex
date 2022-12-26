let add = func(a, b) {
    return a + b;
};

let lambda_test = func() {
    let a = 114, b = 514;
    let lam = lambda (a, b) -> (c) {
        outer.a += c;
        outer.b += c;
        return outer.a + outer.b;
    };
    print(lam(1919), " ", lam(810), "\n");
    return 0;
};

let str_test = func () {
    let str = "    Root Cui AK IOI       ";
    print("Original string: ", *str, "\n");
    str = str.trim();
    print("After str.trim(): ", *str, "\n");
    print("str.split(): ", "\n");
    let split_result = str.split(" ");
    for (let i = 0;i < split_result.length();++i) {
        print(*split_result[i], "\n");
    }
    return 0;
};

let thread_test = func() {
    let lam = lambda () -> () {
        for (let i = 0;i < 5;++i) {
            print("Konnichiha, sekai! Thread is ", *thread_id, "\n");
        }
        return 114514;
    };
    let th_id = threading.start(lam);
    let res = threading.wait(th_id);
    print("This is thread ", *thread_id, ", thread result: ", *res, "\n");
    return 0;
};

let charsets_test = func() {
    let string = "This is a string that contains some unicode character like 你 好 世 界 ？ ！ 。";
    print("Original: ", string, "\n");
    print("Encode as ansi and decode with ansi: ", string.encode("ansi").decode("ansi"), "\n");
    print("Encode as utf-8 and decode with utf-8: ", string.encode("utf-8").decode("utf-8"), "\n");
    print("Yahoo!");
    return 0;
};

let rexstd_test = func() {
    let std = nativeImport("libstd.dylib");

    std.fs.mkdirs("1/a");

    let file = std.fs.open("1/a/test.txt", "w+");
    file.write("Hello, world! 你好，世界！".encode("utf-8"));
    file.close();

    file = std.fs.open("1/a/test.txt", "r");
    let content = file.read(file.length).decode("utf-8");
    print(*content, "\n");
    print("Is EOF: ", file.eof(), "\n");
    file.close();

    print("Stating file: ", std.fs.stat("1/a/test.txt"), " ", std.fs.stat("1/a"), "\n");
    print("Unlinking file: ", std.fs.unlink("1/a/test.txt"), "\n");
    print("Listdir: ", std.fs.listDir("."), "\n");
    print("realpath: ", std.fs.realpath("."), "\n");
    return 0;
};

let rexffi_unittest = func() {
    let lib = nativeImport("libtest.dylib");
    print(lib.hello(114, 514), "\n");
    return 0;
};

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
    print("\n");
    return 0;
};

let rexModInit = func() {
    let s = {a: 1, b: 2};
    print(*s, "\n", s.a, "\n", s.b, "\n", s["a"], "\n", s["b"], "\n");
    print([1,2,3] == [1,2,3], " ", [1,2,3] == [1,2,4], "\n");
    print(" ".join("RootCui", "AK", "IOI!"), "\n");
    print(*([].append("RootCui", "AK", "IOI!")), "\n");
    print(lambda_test());
    thread_test();
    charsets_test();
    rexstd_test();
    rexffi_unittest();
    iter_test();
    str_test();
    return 0;
};