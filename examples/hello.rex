let add = func(a, b) {
    return a + b;
};

let lambda_test = func() {
    let a = 114, b = 514;
    let lam = lambda(a, b) -> (c) {
        outer.a += c;
        outer.b += c;
        return outer.a + outer.b;
    };
    print(lam(1919), " ", lam(810), "\n");
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

let rexModInit = func() {
    let s = {a: 1, b: 2};
    print(*s, "\n", s.a, "\n", s.b, "\n", s["a"], "\n", s["b"], "\n");
    print([1,2,3] == [1,2,3], " ", [1,2,3] == [1,2,4], "\n");
    print(" ".join("RootCui", "AK", "IOI!"), "\n");
    print(*([].append("RootCui", "AK", "IOI!")), "\n");
    print(lambda_test());
    thread_test();
    charsets_test();
    return 0;
};