import 'stdlib/io.rex' as io;

const main = func () {
    let obj = object {
        'stmt': 'FUCK CCF.',
        'online': 'true'
    };
    io.print("Hello, reXscript!", obj.stmt, obj.online);
    return 0;
};