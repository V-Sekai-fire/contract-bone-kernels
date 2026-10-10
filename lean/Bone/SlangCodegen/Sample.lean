import Drape.SlangCodegen.Dsl
import Anny.SlangCodegen.Common

/-!
# `Bone.SlangCodegen.Sample` — keyframe track sampling, one thread per track

Each track samples one channel of one bone's local pose at the clip time:
position or scale (kind 0, three floats, linear) or rotation (kind 1, a
unit quaternion `x y z w`, spherical linear). The key pair is found by a
binary search over the track's key times; past the last key a looping clip
interpolates towards the first key across the loop end, and before the
first key from the last key, which is the linear loop mode's reading.

The spherical interpolation is the engine's `Quaternion::slerp`: the shorter
arc (the second key negated when the dot product is negative), the
`sin`-weighted form while `1 − cos ω > 1e-5`, plain linear weights below it,
then normalised.

Local pose layout, 10 floats per bone: position 0–2, rotation 3–6 (x y z w),
scale 7–9. `trackDst` is the first float the track writes.

Bindings (set 0):

  0  ConstantBuffer<BoneSampleParams> { uint T; float time; float length; uint loop; }
  1  StructuredBuffer<uint>    trackKeys  (2T)  first key, key count
  2  StructuredBuffer<uint>    trackDst   (T)
  3  StructuredBuffer<uint>    trackKind  (T)   0 vec3 linear, 1 quaternion slerp
  4  StructuredBuffer<float>   keyTime    (K)
  5  StructuredBuffer<float>   keyVal     (4K)
  6  RWStructuredBuffer<float> localPose  (10 per bone)
-/

namespace Bone.SlangCodegen.Sample

open LeanSlang
open Drape.SlangCodegen.Dsl
open Anny.SlangCodegen.Common

private def r4 : List Nat := [0, 1, 2, 3]

def shader : SlangShaderModule :=
  { structs := [ { name := "BoneSampleParams"
                 , fields := [fld "T" uT, fld "time" fT, fld "length" fT, fld "loop" uT] } ]
  , globals :=
      [ paramsCB "BoneSampleParams", roU "trackKeys" 1, roU "trackDst" 2, roU "trackKind" 3,
        roF "keyTime" 4, roF "keyVal" 5, rwF "localPose" 6 ]
  , functions :=
      [ entry 64 [dtid]
          ([ let_ uT "i" (.member (v "tid") "x")
           , if_ (ge (v "i") (p "T")) [ ret ]
           , let_ uT "off" (at_ "trackKeys" (v "i" * u 2))
           , let_ uT "cnt" (at_ "trackKeys" (v "i" * u 2 + u 1))
           , let_ uT "dst" (at_ "trackDst" (v "i"))
           , let_ uT "last" (v "off" + v "cnt" - u 1)
           , let_ bT "looping" (ne (p "loop") (u 0))
           , let_ fT "L" (p "length")
           , let_ fT "t" (p "time")
           , if_ (and_ (v "looping") (gt (v "L") (fl 0.0)))
               [ setv "t" (v "t" - v "L" * call "floor" [v "t" / v "L"]) ]
           , let_ fT "t0" (at_ "keyTime" (v "off"))
           , let_ fT "t1" (at_ "keyTime" (v "last"))
           , let_ fT "wrap" (v "t0" + v "L" - v "t1")
           , let_ uT "a" (v "off")
           , let_ uT "b" (v "off")
           , let_ fT "w" (fl 0.0)
           , if_ (gt (v "cnt") (u 1))
               [ if_ (ge (v "t") (v "t1"))
                   [ setv "a" (v "last")
                   , if_ (and_ (v "looping") (gt (v "wrap") (fl 0.0)))
                       [ setv "b" (v "off"), setv "w" ((v "t" - v "t1") / v "wrap") ]
                       [ setv "b" (v "last") ] ]
                   [ if_ (lt (v "t") (v "t0"))
                       [ if_ (and_ (v "looping") (gt (v "wrap") (fl 0.0)))
                           [ setv "a" (v "last"), setv "b" (v "off")
                           , setv "w" ((v "t" + v "L" - v "t1") / v "wrap") ] ]
                       [ let_ uT "lo" (v "off")
                       , let_ uT "hi" (v "last")
                       , while_ (gt (v "hi" - v "lo") (u 1))
                           [ let_ uT "mid" ((v "lo" + v "hi") / u 2)
                           , if_ (le (at_ "keyTime" (v "mid")) (v "t"))
                               [ setv "lo" (v "mid") ] [ setv "hi" (v "mid") ] ]
                       , setv "a" (v "lo"), setv "b" (v "hi")
                       , setv "w" ((v "t" - at_ "keyTime" (v "lo")) /
                                   (at_ "keyTime" (v "hi") - at_ "keyTime" (v "lo"))) ] ] ] ] ++
           (r4.map fun k => let_ fT ("qa" ++ toString k) (at_ "keyVal" (v "a" * u 4 + u k))) ++
           (r4.map fun k => let_ fT ("qb" ++ toString k) (at_ "keyVal" (v "b" * u 4 + u k))) ++
           [ if_ (eq (at_ "trackKind" (v "i")) (u 0))
               (r3.map fun k => setAt "localPose" (v "dst" + u k)
                  (s "qa" k + (s "qb" k - s "qa" k) * v "w"))
               ([ let_ fT "c" (s "qa" 0 * s "qb" 0 + s "qa" 1 * s "qb" 1 + s "qa" 2 * s "qb" 2 + s "qa" 3 * s "qb" 3)
                , let_ fT "sg" (sel (lt (v "c") (fl 0.0)) (fl (-1.0)) (fl 1.0))
                , setv "c" (v "c" * v "sg")
                , let_ fT "s0" (fl 1.0 - v "w")
                , let_ fT "s1" (v "w")
                , if_ (gt (fl 1.0 - v "c") (.litFloatExact 1e-5))
                    [ let_ fT "om" (call "acos" [v "c"])
                    , let_ fT "si" (call "sin" [v "om"])
                    , setv "s0" (call "sin" [(fl 1.0 - v "w") * v "om"] / v "si")
                    , setv "s1" (call "sin" [v "w" * v "om"] / v "si") ]
                , setv "s1" (v "s1" * v "sg") ] ++
                (r4.map fun k => let_ fT ("q" ++ toString k) (v "s0" * s "qa" k + v "s1" * s "qb" k)) ++
                [ let_ fT "n" (call "sqrt" [s "q" 0 * s "q" 0 + s "q" 1 * s "q" 1 + s "q" 2 * s "q" 2 + s "q" 3 * s "q" 3])
                , setv "n" (fl 1.0 / fmax (v "n") tiny) ] ++
                (r4.map fun k => setAt "localPose" (v "dst" + u k) (s "q" k * v "n"))) ]) ] }

end Bone.SlangCodegen.Sample
