let tests =
  Test_promise.suites
  @ Test_ffi.suites
  @ [Test_io.suite];

let () =
  Framework.run("repromise", tests);
