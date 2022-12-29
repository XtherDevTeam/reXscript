let rexModInit = func() {
    let x = 1.0;
    for (let i = 0;i < 99999999;++i) {
        x = (i + i + 2 * i + 1 - 0.379) / (x);
    }
    print(x, "\n");
};