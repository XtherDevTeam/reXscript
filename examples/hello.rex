let add = func(a, b) {
    return a + b;
};

let test = func(a, b) {
    let lam = lambda (a, b) -> () {
        return outer.a + outer.b;
    };
    return lam;
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
    print("This is thread ", thread_id, ", thread result: ", res, "\n");
    return 0;
};

let rexModInit = func() {
    let s = {a: 1, b: 2};
    print(*s, "\n", s.a, "\n", s.b, "\n", s["a"], "\n", s["b"], "\n");
    print([1,2,3] == [1,2,3], " ", [1,2,3] == [1,2,4], "\n");
    print(" ".join("RootCui", "AK", "IOI!"), "\n");
    print(*([].append("RootCui", "AK", "IOI!")), "\n");
    print(test(1,2)(), "\n");
    thread_test();
    return 0;
};