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

let rexModInit = func() {
    let s = {a: 1, b: 2};
    print(*s, "\n", s.a, "\n", s.b, "\n", s["a"], "\n", s["b"], "\n");
    print([1,2,3] == [1,2,3], " ", [1,2,3] == [1,2,4], "\n");
    print(" ".join("RootCui", "AK", "IOI!"), "\n");
    print(*([].append("RootCui", "AK", "IOI!")), "\n");
    print(lambda_test());
    thread_test();
    return 0;
};