type rejectable('a, 'e);
type never;

type promise('a) = rejectable('a, never);
type t('a) = promise('a);



let onUnhandledException = ref(exn => {
  prerr_endline("Unhandled exception in promise callback:");
  prerr_endline(Printexc.to_string(exn));
  exit(2);
});



[%%bs.raw {|
function WrappedRepromise(p) {
    this.wrapped = p;
};

function unwrap(value) {
    if (value instanceof WrappedRepromise)
        return value.wrapped;
    else
        return value;
}

function wrap(value) {
    if (value != null && typeof value.then === 'function')
        return new WrappedRepromise(value);
    else
        return value;
}

function new_(executor) {
    return new Promise(function (resolve, reject) {
        var wrappingResolve = function(value) {
            resolve(wrap(value));
        };
        executor(wrappingResolve, reject);
    });
};

function resolve(value) {
    return Promise.resolve(wrap(value));
};

function then(callback, promise) {
    var safeCallback = function (value) {
        try {
            return callback(value);
        }
        catch (exception) {
            onUnhandledException[0](exception);
        }
    };

    return promise.then(function (value) {
        return safeCallback(unwrap(value));
    });
};

function catch_(callback, promise) {
    var safeCallback = function (error) {
        try {
            return callback(error);
        }
        catch (exception) {
            onUnhandledException[0](exception);
        }
    };

    return promise.catch(safeCallback);
}
|}];



module Rejectable = {
  type t('a, 'e) = rejectable('a, 'e);

  external relax: promise('a) => rejectable('a, _) = "%identity";

  [@bs.val]
  external jsNew:
    (('a => unit) => ('e => unit) => unit) => rejectable('a, 'e) = "new_";

  let new_ = () => {
    let resolve = ref(ignore);
    let reject = ref(ignore);
    let p =
      jsNew((resolve', reject') => {
        resolve := resolve';
        reject := reject';
      });
    (p, resolve^, reject^);
  };

  [@bs.val]
  external resolve: 'a => rejectable('a, _) = "";

  [@bs.val]
  external then_:
    ('a => rejectable('b, 'e), rejectable('a, 'e)) => rejectable('b, 'e) =
      "then";

  let map = (callback, promise) =>
    promise |> then_(value => resolve(callback(value)));

  [@bs.scope "Promise"]
  [@bs.val]
  external reject: 'e => rejectable(_, 'e) = "";

  [@bs.val]
  external catch:
    ('e => rejectable('a, 'e2), rejectable('a, 'e)) => rejectable('a, 'e2) =
      "catch_";

  [@bs.val]
  external unwrap: 'a => 'a = "";

  [@bs.scope "Promise"]
  [@bs.val]
  external jsAll:
    array(rejectable('a, 'e)) => rejectable(array('a), 'e) = "all";

  let all = promises =>
    promises
    |> Array.of_list
    |> jsAll
    |> map (results =>
      results |> Array.map(unwrap) |> Array.to_list);

  [@bs.scope "Promise"]
  [@bs.val]
  external jsRace: array(rejectable('a, 'e)) => rejectable('a, 'e) = "race";

  let race = promises =>
    if (promises == []) {
      raise(Invalid_argument("Repromise.race([]) would be pending forever"));
    }
    else {
      jsRace(Array.of_list(promises));
    }
};



let new_ = () => {
  let (p, resolve, _) = Rejectable.new_();
  (p, resolve);
};

let resolve = Rejectable.resolve;
let then_ = Rejectable.then_;
let map = Rejectable.map;
let all = Rejectable.all;
let race = Rejectable.race;



module ReadyCallbacks = {
  let callbacksPending = () =>
    failwith("unnecessary on JS");

  type snapshot;

  let snapshot = () =>
    failwith("unnecessary on JS");

  let isEmpty = _snapshot =>
    failwith("unnecessary on JS");

  let call = _snapshot =>
    failwith("unnecessary on JS");
};
