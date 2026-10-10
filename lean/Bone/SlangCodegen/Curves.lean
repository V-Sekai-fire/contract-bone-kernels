import Drape.SlangCodegen.Dsl

/-!
# `Bone.SlangCodegen.Curves` — the engine's key curves as Slang helper functions

Each helper restates one engine function, statement for statement, in `float`:

- `bk_ease`         `Math::ease` (a key's transition)
- `bk_cubic`        `Math::cubic_interpolate_in_time` (Barry–Goldman)
- `bk_makima_tan`, `bk_makima`
                    `Animation::makima_tangent`, `Animation::makima_interpolate_in_time`
                    (with `akima_extrapolate` inlined)
- `bk_qmul`, `bk_qinv`, `bk_qlog`, `bk_qexp`
                    `Quaternion::operator*`, `inverse`, `log`, `exp`
- `bk_slerp`        `Quaternion::slerp`
- `bk_nearest_log`  `get_nearest_log` (animation.cpp)
- `bk_blend_log`    `Quaternion::blend_log`
- `bk_sq_cubic`     `Quaternion::spherical_cubic_interpolate_in_time`
- `bk_sq_makima`    `Animation::spherical_makima_interpolate_in_time`

`CMP_EPSILON` is `1e-5`. The engine first passes each quaternion key through
`Basis(q).get_rotation_quaternion()`, which only picks a sign (keys are unit);
the flips that follow re-align every neighbour with `from`, so skipping it
changes at most the sign of the result, which is the same rotation.
-/

namespace Bone.SlangCodegen.Curves

open LeanSlang
open Drape.SlangCodegen.Dsl

def f3T : SlangType := .vec .float 3
def f4T : SlangType := .vec .float 4
def mem (e : E) (f : String) : E := .member e f
def retE (e : E) : St := .ret (some e)
def eps : E := .litFloatExact 1e-5
def fi (n : Int) : E := .litInt n

def fn (ret : SlangType) (name : String) (ps : List (String × SlangType)) (body : List St) : SlangFunctionDecl :=
  { retType := ret, name := name, params := ps.map (fun (n, t) => arg n t), body := body }

private def fs (ns : List String) : List (String × SlangType) := ns.map (·, fT)

def f2T : SlangType := .vec .float 2
/-- A `precise` local: no contraction into fused multiply-add, which df32 needs. -/
def pl (ty : SlangType) (n : String) (e : E) : St := .declarePrecise ty n (some e)
def f2 (a b : E) : E := call "float2" [a, b]

/-! ### df32: a time as a float pair `hi + lo`

Key times, the clip time and the clip length are carried as df32 pairs, and
every difference of times is taken in df32 before it is rounded to one float.
The engine keeps key times in double; with keys a millisecond apart the cubic
and makima curves extrapolate by hundreds, which turns float32 rounding of the
times into visible error. -/

/-- Error-free sum (Knuth): `a + b = s + e` exactly. -/
def twoSum : SlangFunctionDecl :=
  fn f2T "bk_two_sum" (fs ["a", "b"])
    [ pl fT "s" (v "a" + v "b"), pl fT "bb" (v "s" - v "a")
    , pl fT "e" ((v "a" - (v "s" - v "bb")) + (v "b" - v "bb"))
    , retE (f2 (v "s") (v "e")) ]

/-- Dekker split of a float into two 12-bit halves. -/
def split : SlangFunctionDecl :=
  fn f2T "bk_split" (fs ["a"])
    [ pl fT "c" (fl 4097.0 * v "a"), pl fT "hi" (v "c" - (v "c" - v "a")), pl fT "lo" (v "a" - v "hi")
    , retE (f2 (v "hi") (v "lo")) ]

/-- Error-free product (Dekker): `a · b = p + e` exactly. -/
def twoProd : SlangFunctionDecl :=
  fn f2T "bk_two_prod" (fs ["a", "b"])
    [ pl fT "pr" (v "a" * v "b"), pl f2T "sa" (call "bk_split" [v "a"]), pl f2T "sb" (call "bk_split" [v "b"])
    , pl fT "e" (((mem (v "sa") "x" * mem (v "sb") "x" - v "pr") + mem (v "sa") "x" * mem (v "sb") "y" + mem (v "sa") "y" * mem (v "sb") "x")
        + mem (v "sa") "y" * mem (v "sb") "y")
    , retE (f2 (v "pr") (v "e")) ]

/-- df32 + df32. -/
def dfAdd : SlangFunctionDecl :=
  fn f2T "bk_df_add" [("a", f2T), ("b", f2T)]
    [ pl f2T "s" (call "bk_two_sum" [mem (v "a") "x", mem (v "b") "x"])
    , pl fT "e" (mem (v "s") "y" + mem (v "a") "y" + mem (v "b") "y")
    , pl fT "h" (mem (v "s") "x" + v "e"), pl fT "l" (v "e" - (v "h" - mem (v "s") "x"))
    , retE (f2 (v "h") (v "l")) ]

/-- df32 − df32. -/
def dfSub : SlangFunctionDecl :=
  fn f2T "bk_df_sub" [("a", f2T), ("b", f2T)]
    [ retE (call "bk_df_add" [v "a", .un "-" (v "b")]) ]

/-- df32 × float. -/
def dfMulF : SlangFunctionDecl :=
  fn f2T "bk_df_mulf" [("a", f2T), ("b", fT)]
    [ pl f2T "pr" (call "bk_two_prod" [mem (v "a") "x", v "b"])
    , pl fT "e" (mem (v "pr") "y" + mem (v "a") "y" * v "b")
    , pl fT "h" (mem (v "pr") "x" + v "e"), pl fT "l" (v "e" - (v "h" - mem (v "pr") "x"))
    , retE (f2 (v "h") (v "l")) ]

/-- A df32 rounded to one float. -/
def dfv (a : E) : E := mem a "x" + mem a "y"
/-- `a − b` in df32, rounded to one float. -/
def dfd (a b : E) : E := dfv (call "bk_df_sub" [a, b])


/-- `Math::ease(x, c)`. -/
def ease : SlangFunctionDecl :=
  fn fT "bk_ease" (fs ["x0", "c"])
    [ let_ fT "x" (fmin (fmax (v "x0") (fl 0.0)) (fl 1.0))
    , if_ (gt (v "c") (fl 0.0))
        [ if_ (lt (v "c") (fl 1.0))
            [ retE (fl 1.0 - call "pow" [fl 1.0 - v "x", fl 1.0 / v "c"]) ]
            [ retE (call "pow" [v "x", v "c"]) ] ]
        [ if_ (lt (v "c") (fl 0.0))
            [ if_ (lt (v "x") (fl 0.5))
                [ retE (call "pow" [v "x" * fl 2.0, .un "-" (v "c")] * fl 0.5) ]
                [ retE ((fl 1.0 - call "pow" [fl 1.0 - (v "x" - fl 0.5) * fl 2.0, .un "-" (v "c")]) * fl 0.5 + fl 0.5) ] ] ]
    , retE (fl 0.0) ]

private def lerpE (a b w : E) : E := a + w * (b - a)

/-- `Math::cubic_interpolate_in_time(from, to, pre, post, w, toT, preT, postT)`; the
times are df32 offsets from the `from` key, and every time difference is df32. -/
def cubic : SlangFunctionDecl :=
  fn fT "bk_cubic" (fs ["from", "to", "pre", "post", "w"] ++ [("toT", f2T), ("preT", f2T), ("postT", f2T)])
    [ pl f2T "t" (call "bk_df_mulf" [v "toT", v "w"])
    , let_ fT "fTo" (dfv (v "toT")), let_ fT "fPre" (dfv (v "preT")), let_ fT "fPost" (dfv (v "postT"))
    , let_ fT "dPostTo" (dfd (v "postT") (v "toT")), let_ fT "dToPre" (dfd (v "toT") (v "preT"))
    , let_ fT "tPre" (dfd (v "t") (v "preT")), let_ fT "tTo" (dfd (v "t") (v "toT")), let_ fT "tt" (dfv (v "t"))
    , let_ fT "a1" (lerpE (v "pre") (v "from") (sel (eq (v "fPre") (fl 0.0)) (fl 0.0) (v "tPre" / .un "-" (v "fPre"))))
    , let_ fT "a2" (lerpE (v "from") (v "to") (sel (eq (v "fTo") (fl 0.0)) (fl 0.5) (v "tt" / v "fTo")))
    , let_ fT "a3" (lerpE (v "to") (v "post") (sel (eq (v "dPostTo") (fl 0.0)) (fl 1.0) (v "tTo" / v "dPostTo")))
    , let_ fT "b1" (lerpE (v "a1") (v "a2") (sel (eq (v "dToPre") (fl 0.0)) (fl 0.0) (v "tPre" / v "dToPre")))
    , let_ fT "b2" (lerpE (v "a2") (v "a3") (sel (eq (v "fPost") (fl 0.0)) (fl 1.0) (v "tt" / v "fPost")))
    , retE (lerpE (v "b1") (v "b2") (sel (eq (v "fTo") (fl 0.0)) (fl 0.5) (v "tt" / v "fTo"))) ]

/-- `Animation::makima_tangent(p0, p1, p2, p3)`. -/
def makimaTan : SlangFunctionDecl :=
  fn fT "bk_makima_tan" (fs ["p0", "p1", "p2", "p3"])
    [ let_ fT "w1" (fabs (v "p3" - v "p2") + fabs (v "p3" + v "p2") * fl 0.5)
    , let_ fT "w2" (fabs (v "p1" - v "p0") + fabs (v "p1" + v "p0") * fl 0.5)
    , if_ (eq (v "w1" + v "w2") (fl 0.0)) [ retE ((v "p1" + v "p2") * fl 0.5) ]
    , retE ((v "w1" * v "p1" + v "w2" * v "p2") / (v "w1" + v "w2")) ]

/-- `Animation::makima_interpolate_in_time(from, to, pre, post, pp, qq, w, toT, preT, postT, ppT, qqT)`;
times are df32 offsets from the `from` key, differences in df32. -/
def makima : SlangFunctionDecl :=
  let ex (e n : E) : E := fl 2.0 * e - n
  fn fT "bk_makima" (fs ["from", "to", "pre", "post", "pp", "qq", "w"] ++
      [("toT", f2T), ("preT", f2T), ("postT", f2T), ("ppT", f2T), ("qqT", f2T)])
    [ let_ fT "fTo" (dfv (v "toT")), let_ fT "fPre" (dfv (v "preT"))
    , let_ fT "dPost" (dfd (v "postT") (v "toT")), let_ fT "dPp" (dfd (v "preT") (v "ppT")), let_ fT "dQq" (dfd (v "qqT") (v "postT"))
    , let_ bT "cTo" (eq (v "fTo") (fl 0.0))
    , let_ bT "cPre" (eq (v "fPre") (fl 0.0))
    , let_ bT "cPost" (eq (v "dPost") (fl 0.0))
    , let_ bT "cPp" (eq (v "dPp") (fl 0.0))
    , let_ bT "cQq" (eq (v "dQq") (fl 0.0))
    , let_ fT "mFrom" (sel (v "cTo") (fl 0.0) ((v "to" - v "from") / v "fTo"))
    , let_ fT "mPre" (sel (v "cPre") (fl 0.0) ((v "from" - v "pre") / .un "-" (v "fPre")))
    , let_ fT "mPost" (sel (v "cPost") (fl 0.0) ((v "post" - v "to") / v "dPost"))
    , if_ (v "cPre") [ setv "mPre" (sel (v "cPost") (v "mFrom") (ex (v "mFrom") (v "mPost"))) ]
    , if_ (v "cPost") [ setv "mPost" (sel (v "cPre") (v "mFrom") (ex (v "mFrom") (v "mPre"))) ]
    , let_ fT "mPp" (sel (v "cPp") (ex (v "mPre") (v "mFrom")) ((v "pre" - v "pp") / v "dPp"))
    , let_ fT "mQq" (sel (v "cQq") (ex (v "mPost") (v "mFrom")) ((v "qq" - v "post") / v "dQq"))
    , let_ fT "tFrom" (call "bk_makima_tan" [v "mPp", v "mPre", v "mFrom", v "mPost"])
    , let_ fT "tTo" (call "bk_makima_tan" [v "mPre", v "mFrom", v "mPost", v "mQq"])
    , let_ fT "s2" (v "w" * v "w")
    , let_ fT "s3" (v "s2" * v "w")
    , retE ((fl 2.0 * v "s3" - fl 3.0 * v "s2" + fl 1.0) * v "from" +
            (v "s3" - fl 2.0 * v "s2" + v "w") * v "fTo" * v "tFrom" +
            (fl (-2.0) * v "s3" + fl 3.0 * v "s2") * v "to" +
            (v "s3" - v "s2") * v "fTo" * v "tTo") ]

private def q (n : String) (c : String) : E := mem (v n) c

/-- `Quaternion::operator*`. -/
def qmul : SlangFunctionDecl :=
  fn f4T "bk_qmul" [("a", f4T), ("b", f4T)]
    [ retE (call "float4"
        [ q "a" "w" * q "b" "x" + q "a" "x" * q "b" "w" + q "a" "y" * q "b" "z" - q "a" "z" * q "b" "y"
        , q "a" "w" * q "b" "y" + q "a" "y" * q "b" "w" + q "a" "z" * q "b" "x" - q "a" "x" * q "b" "z"
        , q "a" "w" * q "b" "z" + q "a" "z" * q "b" "w" + q "a" "x" * q "b" "y" - q "a" "y" * q "b" "x"
        , q "a" "w" * q "b" "w" - q "a" "x" * q "b" "x" - q "a" "y" * q "b" "y" - q "a" "z" * q "b" "z" ]) ]

/-- `Quaternion::inverse` (unit quaternions: the conjugate). -/
def qinv : SlangFunctionDecl :=
  fn f4T "bk_qinv" [("a", f4T)]
    [ retE (call "float4" [.un "-" (q "a" "x"), .un "-" (q "a" "y"), .un "-" (q "a" "z"), q "a" "w"]) ]

/-- `Quaternion::log`, as the vector part. -/
def qlog : SlangFunctionDecl :=
  fn f3T "bk_qlog" [("a", f4T)]
    [ if_ (gt (fabs (q "a" "w")) (fl 1.0 - eps)) [ retE (mem (v "a") "xyz" * fl 2.0) ]
    , let_ fT "r" (fl 1.0 / call "sqrt" [fl 1.0 - q "a" "w" * q "a" "w"])
    , retE (mem (v "a") "xyz" * v "r" * (fl 2.0 * call "acos" [q "a" "w"])) ]

/-- `Quaternion::exp` of a pure quaternion given as its vector part. -/
def qexp : SlangFunctionDecl :=
  fn f4T "bk_qexp" [("e", f3T)]
    [ let_ fT "th" (call "length" [v "e"])
    , if_ (lt (v "th") eps) [ retE (call "float4" [fl 0.0, fl 0.0, fl 0.0, fl 1.0]) ]
    , let_ f3T "ax" (v "e" / v "th")
    , retE (call "float4" [v "ax" * call "sin" [v "th" * fl 0.5], call "cos" [v "th" * fl 0.5]]) ]

/-- `Quaternion::slerp`. -/
def slerp : SlangFunctionDecl :=
  fn f4T "bk_slerp" [("a", f4T), ("b0", f4T), ("w", fT)]
    [ let_ f4T "b" (v "b0")
    , let_ fT "c" (call "dot" [v "a", v "b"])
    , if_ (lt (v "c") (fl 0.0)) [ setv "c" (.un "-" (v "c")), setv "b" (.un "-" (v "b")) ]
    , let_ fT "s0" (fl 1.0 - v "w")
    , let_ fT "s1" (v "w")
    , if_ (gt (fl 1.0 - v "c") eps)
        [ let_ fT "om" (call "acos" [v "c"])
        , let_ fT "si" (call "sin" [v "om"])
        , setv "s0" (call "sin" [(fl 1.0 - v "w") * v "om"] / v "si")
        , setv "s1" (call "sin" [v "w" * v "om"] / v "si") ]
    , let_ f4T "r" (v "a" * v "s0" + v "b" * v "s1")
    , retE (v "r" / fmax (call "length" [v "r"]) (.litFloatExact 1e-20)) ]

/-- `get_nearest_log(q, neighbor)`. -/
def nearestLog : SlangFunctionDecl :=
  fn f3T "bk_nearest_log" [("a", f4T), ("nb", f3T)]
    [ let_ fT "phase" (fl 2.0 * .litFloatExact 6.283185307179586)
    , let_ f3T "ax" (mem (v "a") "xyz")
    , let_ fT "ang" (fl 2.0 * call "atan2" [call "length" [v "ax"], q "a" "w"])
    , let_ bT "axZero" (and_ (and_ (lt (fabs (q "ax" "x")) eps) (lt (fabs (q "ax" "y")) eps)) (lt (fabs (q "ax" "z")) eps))
    , let_ fT "nl" (call "length" [v "nb"])
    , if_ (v "axZero")
        [ setv "ax" (sel (eq (v "nl") (fl 0.0)) (v "nb") (v "nb" / v "nl")) ]
        [ setv "ax" (v "ax" / call "length" [v "ax"]) ]
    , let_ fT "ph" (call "round" [(call "dot" [v "nb", v "ax"] - v "ang") / v "phase"])
    , retE (v "ax" * (v "ang" + v "ph" * v "phase")) ]

/-- `Quaternion::blend_log(from_q, from_ln, to_ln, to_in_from_ln, w)`. -/
def blendLog : SlangFunctionDecl :=
  fn f4T "bk_blend_log" [("fq", f4T), ("fromLn", f3T), ("toLn0", f3T), ("tif", f3T), ("w", fT)]
    [ let_ f3T "toLn" (v "toLn0")
    , let_ fT "th" (call "length" [v "tif"])
    , let_ f3T "cr" (call "cross" [v "tif", v "toLn"])
    , setv "toLn" (v "toLn" + v "tif" + v "cr" * fl 0.5)
    , if_ (ge (v "th") eps)
        [ let_ fT "c" (fl 1.0 / (v "th" * v "th"))
        , if_ (ge (fabs (v "th" - .litFloatExact 3.141592653589793)) (fmax (eps * fabs (v "th")) eps))
            [ setv "c" (v "c" - (fl 1.0 + call "cos" [v "th"]) / (fl 2.0 * v "th" * call "sin" [v "th"])) ]
        , setv "toLn" (v "toLn" + call "cross" [v "tif", v "cr"] * v "c") ]
    , let_ f3T "bl" (v "fromLn" + (v "toLn" - v "fromLn") * v "w")
    , retE (call "bk_qmul" [v "fq", call "bk_qexp" [v "bl"]]) ]

private def flipTo (dst ref : String) (prevFlipped : Option String) : List St :=
  let d := call "dot" [v ref, v dst]
  let flag := "f_" ++ dst
  [ let_ bT flag (match prevFlipped with
      | none => lt d (fl 0.0)
      | some p => sel (v p) (le d (fl 0.0)) (lt d (fl 0.0)))
  , if_ (v flag) [ setv dst (.un "-" (v dst)) ] ]

private def lnIn (n base k : String) : St :=
  let_ f3T n (call "bk_qlog" [call "bk_qmul" [call "bk_qinv" [v base], v k]])

private def comps : List String := ["x", "y", "z"]

/-- `Quaternion::spherical_cubic_interpolate_in_time` (from `a` to `b`, `pa` before `a`, `pb` after `b`). -/
def sqCubic : SlangFunctionDecl :=
  let side (out frm tt pre post : String) : St :=
    let_ f3T out (call "float3" (comps.map fun c =>
      call "bk_cubic" [mem (v frm) c, mem (v tt) c, mem (v pre) c, mem (v post) c, v "w", v "toT", v "preT", v "postT"]))
  fn f4T "bk_sq_cubic" [("a", f4T), ("b0", f4T), ("pa0", f4T), ("pb0", f4T), ("w", fT), ("toT", f2T), ("preT", f2T), ("postT", f2T)]
    ([ let_ f4T "pa" (v "pa0"), let_ f4T "b" (v "b0"), let_ f4T "pb" (v "pb0") ] ++
     flipTo "pa" "a" none ++ flipTo "b" "a" none ++ flipTo "pb" "b" (some "f_b") ++
     [ let_ f3T "z0" (call "float3" [fl 0.0, fl 0.0, fl 0.0])
     , lnIn "aTo" "a" "b", lnIn "aPre" "a" "pa", lnIn "aPost" "a" "pb"
     , side "fromLn" "z0" "aTo" "aPre" "aPost"
     , lnIn "bFrom" "b" "a", lnIn "bPre" "b" "pa", lnIn "bPost" "b" "pb"
     , side "toLn" "bFrom" "z0" "bPre" "bPost"
     , retE (call "bk_blend_log" [v "a", v "fromLn", v "toLn", v "aTo", v "w"]) ])

/-- `Animation::spherical_makima_interpolate_in_time` (`pp` before `pa`, `qq` after `pb`). -/
def sqMakima : SlangFunctionDecl :=
  let side (out frm tt pre post pp qq : String) : St :=
    let_ f3T out (call "float3" (comps.map fun c =>
      call "bk_makima" [mem (v frm) c, mem (v tt) c, mem (v pre) c, mem (v post) c, mem (v pp) c, mem (v qq) c,
        v "w", v "toT", v "preT", v "postT", v "ppT", v "qqT"]))
  fn f4T "bk_sq_makima" [("a", f4T), ("b0", f4T), ("pa0", f4T), ("pb0", f4T), ("pp0", f4T), ("qq0", f4T),
      ("w", fT), ("toT", f2T), ("preT", f2T), ("postT", f2T), ("ppT", f2T), ("qqT", f2T)]
    ([ let_ f4T "pa" (v "pa0"), let_ f4T "b" (v "b0"), let_ f4T "pb" (v "pb0"),
       let_ f4T "pp" (v "pp0"), let_ f4T "qq" (v "qq0") ] ++
     flipTo "pa" "a" none ++ flipTo "pp" "pa" (some "f_pa") ++
     flipTo "b" "a" none ++ flipTo "pb" "b" (some "f_b") ++ flipTo "qq" "pb" (some "f_pb") ++
     [ let_ f3T "z0" (call "float3" [fl 0.0, fl 0.0, fl 0.0])
     , lnIn "aTo" "a" "b", lnIn "aPre" "a" "pa", lnIn "aPost" "a" "pb"
     , let_ f3T "aPp" (call "bk_nearest_log" [call "bk_qmul" [call "bk_qinv" [v "a"], v "pp"], v "aPre"])
     , let_ f3T "aQq" (call "bk_nearest_log" [call "bk_qmul" [call "bk_qinv" [v "a"], v "qq"], v "aPost"])
     , side "fromLn" "z0" "aTo" "aPre" "aPost" "aPp" "aQq"
     , lnIn "bFrom" "b" "a", lnIn "bPre" "b" "pa", lnIn "bPost" "b" "pb"
     , let_ f3T "bPp" (call "bk_nearest_log" [call "bk_qmul" [call "bk_qinv" [v "b"], v "pp"], v "bPre"])
     , let_ f3T "bQq" (call "bk_nearest_log" [call "bk_qmul" [call "bk_qinv" [v "b"], v "qq"], v "bPost"])
     , side "toLn" "bFrom" "z0" "bPre" "bPost" "bPp" "bQq"
     , retE (call "bk_blend_log" [v "a", v "fromLn", v "toLn", v "aTo", v "w"]) ])

private def tau : E := .litFloatExact 6.283185307179586

/-- `Math::fposmod(x, y)`. -/
def fposmod : SlangFunctionDecl :=
  fn fT "bk_fposmod" (fs ["x", "y"])
    [ let_ fT "r" (call "fmod" [v "x", v "y"])
    , if_ (or_ (and_ (lt (v "r") (fl 0.0)) (gt (v "y") (fl 0.0))) (and_ (gt (v "r") (fl 0.0)) (lt (v "y") (fl 0.0))))
        [ setv "r" (v "r" + v "y") ]
    , retE (v "r") ]

/-- `Math::angle_difference(from, to)`. -/
def angleDiff : SlangFunctionDecl :=
  fn fT "bk_angle_diff" (fs ["from", "to"])
    [ let_ fT "d" (call "fmod" [v "to" - v "from", tau])
    , retE (call "fmod" [fl 2.0 * v "d", tau] - v "d") ]

/-- `Math::cubic_interpolate_angle_in_time`. -/
def cubicAngle : SlangFunctionDecl :=
  let unwrap (out x ref : String) : St :=
    let_ fT out (v ref + call "fmod" [fl 2.0 * call "fmod" [v x - v ref, tau], tau] - call "fmod" [v x - v ref, tau])
  fn fT "bk_cubic_angle" (fs ["from", "to", "pre", "post", "w"] ++ [("toT", f2T), ("preT", f2T), ("postT", f2T)])
    [ let_ fT "fr" (call "fmod" [v "from", tau])
    , unwrap "pr" "pre" "fr"
    , unwrap "tr" "to" "fr"
    , unwrap "po" "post" "tr"
    , retE (call "bk_cubic" [v "fr", v "tr", v "pr", v "po", v "w", v "toT", v "preT", v "postT"]) ]

/-- `Animation::makima_interpolate_angle_in_time`. -/
def makimaAngle : SlangFunctionDecl :=
  let unwrap (out x ref : String) : St :=
    let_ fT out (v ref + call "fmod" [fl 2.0 * call "fmod" [v x - v ref, tau], tau] - call "fmod" [v x - v ref, tau])
  fn fT "bk_makima_angle" (fs ["from", "to", "pre", "post", "pp", "qq", "w"] ++
      [("toT", f2T), ("preT", f2T), ("postT", f2T), ("ppT", f2T), ("qqT", f2T)])
    [ let_ fT "fr" (call "fmod" [v "from", tau])
    , unwrap "pr" "pre" "fr"
    , unwrap "ppr" "pp" "pr"
    , unwrap "tr" "to" "fr"
    , unwrap "po" "post" "tr"
    , unwrap "qqr" "qq" "po"
    , retE (call "bk_makima" [v "fr", v "tr", v "pr", v "po", v "ppr", v "qqr", v "w", v "toT", v "preT", v "postT", v "ppT", v "qqT"]) ]

/-- Every helper, callees before callers. -/
def all : List SlangFunctionDecl :=
  [ twoSum, split, twoProd, dfAdd, dfSub, dfMulF, ease, cubic, makimaTan, makima, qmul, qinv, qlog, qexp, slerp, nearestLog, blendLog, sqCubic, sqMakima,
    fposmod, angleDiff, cubicAngle, makimaAngle ]

end Bone.SlangCodegen.Curves
