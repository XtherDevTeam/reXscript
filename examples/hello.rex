let add = func(a, b) {
    return a + b;
};

let test = func(a, b) {
    let lam = lambda (a, b) -> () {
        return outer.a + outer.b;
    };
    return lam;
};

let rexModInit = func() {
    let s = {a: 1, b: 2};
    print(*s, "\n", s.a, "\n", s.b, "\n", s["a"], "\n", s["b"], "\n");
    print([1,2,3] == [1,2,3], " ", [1,2,3] == [1,2,4], "\n");
    print(" ".join("RootCui", "AK", "IOI!"), "\n");
    print(*([].append("RootCui", "AK", "IOI!")), "\n");
    print(test(1,2)(), "\n");
    return 0;
};