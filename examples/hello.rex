let add = func(a, b) {
    return a + b;
};

let rexModInit = func() {
    let sum = 0;
    for (let i = 0;i < 1000000;i += 1) {
        // print(format("Konnichiha, sekai! My location is ${str}.\nText: ${str} I: ${int bs=dec}\n", __path__, "Hello, world!", i));
        sum += i;
    }
    print(*sum, "\n");
    let s = {a: 1, b: 2};
    print(*s, "\n", s.a, "\n", s.b, "\n", s["a"], "\n", s["b"], "\n");
    print([1,2,3] == [1,2,3], " ", [1,2,3] == [1,2,4], "\n");
    print(" ".join("RootCui", "AK", "IOI!"), "\n");
    print(*([].append("RootCui", "AK", "IOI!")), "\n");
    return 0;
};