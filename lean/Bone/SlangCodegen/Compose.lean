import Drape.SlangCodegen.Dsl
import Anny.SlangCodegen.Common

/-!
# `Bone.SlangCodegen.Compose` — a bone's local pose matrix (helper, not a kernel)

`localMatrix b` declares the 3×3 `M` and the 3-vector `lp` of bone `b`'s local
pose from the 10-float `localPose` record (position 0–2, rotation 3–6 as
`x y z w`, scale 7–9). `M` is the engine's `Basis::set_quaternion_scale`: the
rotation of the quaternion times `diag(scale)` (columns scaled). `bone_local`
writes it; `bone_jump` composes it with the ancestors.
-/

namespace Bone.SlangCodegen.Compose

open LeanSlang
open Drape.SlangCodegen.Dsl
open Anny.SlangCodegen.Common

/-- The local pose `[M | lp]` of bone `b`: `M = R(q) · diag(scale)`. -/
def localMatrix (b : E) : List St :=
  [ let_ uT "lb" (b * u 10)
  , let_ fT "qx" (at_ "localPose" (v "lb" + u 3))
  , let_ fT "qy" (at_ "localPose" (v "lb" + u 4))
  , let_ fT "qz" (at_ "localPose" (v "lb" + u 5))
  , let_ fT "qw" (at_ "localPose" (v "lb" + u 6))
  , let_ fT "qd" (v "qx" * v "qx" + v "qy" * v "qy" + v "qz" * v "qz" + v "qw" * v "qw")
  , let_ fT "qs" (fl 2.0 / fmax (v "qd") tiny)
  , let_ fT "xs" (v "qx" * v "qs"), let_ fT "ys" (v "qy" * v "qs"), let_ fT "zs" (v "qz" * v "qs")
  , let_ fT "wx" (v "qw" * v "xs"), let_ fT "wy" (v "qw" * v "ys"), let_ fT "wz" (v "qw" * v "zs")
  , let_ fT "xx" (v "qx" * v "xs"), let_ fT "xy" (v "qx" * v "ys"), let_ fT "xz" (v "qx" * v "zs")
  , let_ fT "yy" (v "qy" * v "ys"), let_ fT "yz" (v "qy" * v "zs"), let_ fT "zz" (v "qz" * v "zs") ] ++
  vec "S" (fun k => at_ "localPose" (v "lb" + u (7 + k))) ++
  vec "lp" (fun k => at_ "localPose" (v "lb" + u k)) ++
  mat "R" (fun i j =>
    match i, j with
    | 0, 0 => fl 1.0 - (v "yy" + v "zz") | 0, 1 => v "xy" - v "wz" | 0, 2 => v "xz" + v "wy"
    | 1, 0 => v "xy" + v "wz" | 1, 1 => fl 1.0 - (v "xx" + v "zz") | 1, 2 => v "yz" - v "wx"
    | 2, 0 => v "xz" - v "wy" | 2, 1 => v "yz" + v "wx" | _, _ => fl 1.0 - (v "xx" + v "yy")) ++
  mat "M" (fun i j => m "R" i j * s "S" j)

end Bone.SlangCodegen.Compose
