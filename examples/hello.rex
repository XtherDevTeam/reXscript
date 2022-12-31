let add = func (a, b) {
    return a + b;
};

let lambda_test = func () {
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
    forEach (i in split_result) {
        print(*i, "\n");
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
    let std = require("../../rexStdlib/dist/libstd.dylib");

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
        print(*i, " ");
    }
    print("\n");
    print("Vec object iterator test:\n");
    let vec = ["this is a string", false, 0, -114.514, "Root Cui AK IOI"];
    forEach (i in vec) {
        print(*i, "\n");
    }
    return 0;
};

let sqlite_test = func() {
    let sqlite = require("../../rexStdlib/dist/libstd.dylib").sqlite;
    let db = sqlite.open(":memory:");
    db.executeScript(
        "
        create table oj_contest_ranking
        (
            uid                integer primary key,
            scores             string default '[]',
            final_score        integer default 0
        );
        insert into oj_contest_ranking (uid, scores, final_score) values (114514, 'reXscript NB', 1919810);
        "
    );
    let rows = db.execute("select * from oj_contest_ranking");
    db.close();
    forEach (i in rows) {
        print(i["scores"].decode("ansi"), "\n");
    }
    return 0;
};

let socket_test = func() {
    let net = require("../../rexStdlib/dist/libstd.dylib").net;
    let socket = net.socket();
    let ip = net.resolve("www.baidu.com");
    print(ip, "\n");
    socket.connect(net.resolve("www.baidu.com"), 80);
    socket.send("GET / HTTP/1.1\r\nHost: www.baidu.com\r\nConnection: close\r\n\r\n".encode("ansi"));
    let buf = "".encode("ansi");
    while (true) {
        let temp = socket.recv(1024);
        print("RCVD: ", temp.length(), "\n");
        if (temp.length() > 0) {
            buf.concat(temp);
        } else {
            break;
        }
    }
    print(buf.decode("utf-8"));
    socket.close();
    return 0;
};

let args_test = func() {
    forEach (i in rexArgs) {
        print(*i, "\n");
    }
};

let main_test = func() {
    let s = {a: 1, b: 2};
    print(*s, "\n", s.a, "\n", s.b, "\n", s["a"], "\n", s["b"], "\n");
    print([1,2,3] == [1,2,3], " ", [1,2,3] == [1,2,4], "\n");
    print(" ".join("RootCui", "AK", "IOI!"), "\n");
    print(*([].append("RootCui", "AK", "IOI!")), "\n");
    print(lambda_test());
    thread_test();
    charsets_test();
    rexstd_test();
    iter_test();
    str_test();
    sqlite_test();
    args_test();
    socket_test();
    return 0;
};

let rexModInit = func() {
    main_test();
    return 0;
};