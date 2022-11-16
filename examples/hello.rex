import 'stdlib/io.rex' as io;

const main = func () {
    var obj = object {
        'stmt': 'FUCK CCF.',
        'online': 'true'
    };
    io.print("Hello, reXscript!", obj.stmt, obj.online);
    return 0;
};