let counter = 0;
let lock = mutex();
func foo() {
  with (_ : lock) {
    print("Thread ", thread_id, ": Count ", counter, ": Hello\n");
  	++counter;
  }
  return null;
}

func rexModInit() {
  let a = threading.start(foo), b = threading.start(foo);
  threading.wait(a);
  threading.wait(b);
  lock.finalize(); // destroy lock object
  return null;
}