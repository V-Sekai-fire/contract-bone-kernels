/-!
# `Bone.Proofs.Jump` — pointer jumping computes the engine's skeleton-space pose

The engine defines a bone's skeleton-space pose by recursion on its parent:

  `global b = global a * local b`  when `parent b = some a`,
  `global b = local b`             when `b` is a root.

`bone_local` starts each bone at `(local b, parent b)`; each `bone_jump` round
replaces `(acc b, anc b)` with `(acc a * acc b, anc a)` where `a = anc b`, and
leaves a bone with no ancestor as it is. `rounds r` is the state after `r`
rounds.

`jump_correct`: whenever bone `b`'s `2^r`-th ancestor does not exist (that is,
its depth is below `2^r`), `rounds r b = (global b, none)`, for any forest of
parent links and any associative multiplication. The proof uses only
associativity, so it holds for any tree shape.

`jump_correct_of_depth` restates that with depth: if bone `b` has
at most `d` ancestors and `d < 2^r`, then `r` rounds suffice. The host
dispatches `⌈log₂(maxDepth + 1)⌉` rounds, which meets this for every bone.

What this does not cover: that the emitted 3×4 code computes the affine product
(`M` here is any type with an associative `*`), and floating-point rounding.
The fixture tests in `tests/bone_kernels` measure both against the engine.
-/

namespace Bone.Proofs.Jump

section
variable {M : Type} [Mul M]

/-- `up parent k b`: the `k`-th ancestor of `b` (`up 0 b = some b`), `none` past the root. -/
def up (parent : Nat → Option Nat) : Nat → Nat → Option Nat
  | 0, b => some b
  | k + 1, b => (up parent k b).bind parent

/-- `seg k b`: the product of the local poses of `b` and its first `k` ancestors
(fewer if the root comes first), root-side on the left. -/
def seg (parent : Nat → Option Nat) (loc : Nat → M) : Nat → Nat → M
  | 0, b => loc b
  | k + 1, b =>
    match parent b with
    | none => loc b
    | some a => seg parent loc k a * loc b

/-- One `bone_jump` round over the whole skeleton. -/
def step (s : Nat → M × Option Nat) (b : Nat) : M × Option Nat :=
  match (s b).2 with
  | none => s b
  | some a => ((s a).1 * (s b).1, (s a).2)

/-- `bone_local`, then `r` rounds of `bone_jump`. -/
def rounds (parent : Nat → Option Nat) (loc : Nat → M) : Nat → Nat → M × Option Nat
  | 0, b => (loc b, parent b)
  | r + 1, b => step (rounds parent loc r) b

variable (parent : Nat → Option Nat) (loc : Nat → M)

theorem up_one (b : Nat) : up parent 1 b = parent b := by
  simp [up]

/-- Walking `k + 1` steps is one step, then `k`. -/
theorem up_succ' (k b : Nat) : up parent (k + 1) b = (parent b).bind (up parent k) := by
  induction k generalizing b with
  | zero => simp [up]
  | succ k ih =>
    show (up parent (k + 1) b).bind parent = (parent b).bind (up parent (k + 1))
    rw [ih]
    cases parent b <;> simp [up]

/-- Walking `n + m` steps is `m` steps, then `n`. -/
theorem up_add (n m b : Nat) : up parent (n + m) b = (up parent m b).bind (up parent n) := by
  induction n with
  | zero => simp [up]
  | succ n ih =>
    rw [Nat.succ_add]
    show (up parent (n + m) b).bind parent = (up parent m b).bind (up parent (n + 1))
    rw [ih]
    cases up parent m b <;> simp [up]

/-- Past the root, every further walk stays past the root. -/
theorem up_none_add {m b : Nat} (h : up parent m b = none) (n : Nat) : up parent (n + m) b = none := by
  rw [up_add, h]; rfl

/-- Unfolding `seg` one step at a bone with a parent. -/
theorem seg_some {a b : Nat} (h : parent b = some a) (k : Nat) :
    seg parent loc (k + 1) b = seg parent loc k a * loc b := by
  simp [seg, h]

/-- At a root, every segment is the root's own local pose. -/
theorem seg_none {b : Nat} (h : parent b = none) (k : Nat) : seg parent loc k b = loc b := by
  cases k <;> simp [seg, h]

/-- Splitting a segment at the `(k+1)`-th ancestor. -/
theorem seg_add (assoc : ∀ x y z : M, x * y * z = x * (y * z)) (j k b : Nat) :
    seg parent loc (j + k + 1) b =
      match up parent (k + 1) b with
      | none => seg parent loc k b
      | some a => seg parent loc j a * seg parent loc k b := by
  induction k generalizing b with
  | zero =>
    rw [up_one]
    show seg parent loc (j + 1) b = _
    cases h : parent b <;> simp [seg, h]
  | succ k ih =>
    rw [up_succ']
    show seg parent loc ((j + k + 1) + 1) b = _
    cases h : parent b with
    | none => simp [seg, h]
    | some a =>
      rw [seg_some parent loc h, seg_some parent loc h, Option.bind_some]
      rw [ih a]
      cases up parent (k + 1) a with
      | none => rfl
      | some c => exact assoc _ _ _

/-- After `r` rounds, bone `b` holds the product over its first `2^r` nodes and
points at its `2^r`-th ancestor. -/
theorem rounds_eq (assoc : ∀ x y z : M, x * y * z = x * (y * z)) (r b : Nat) :
    rounds parent loc r b = (seg parent loc (2 ^ r - 1) b, up parent (2 ^ r) b) := by
  induction r generalizing b with
  | zero => simp [rounds, seg, up_one]
  | succ r ih =>
    have hp : 2 ^ r ≥ 1 := Nat.one_le_two_pow
    have e1 : 2 ^ (r + 1) - 1 = (2 ^ r - 1) + (2 ^ r - 1) + 1 := by rw [Nat.pow_succ]; omega
    have e2 : 2 ^ (r + 1) = 2 ^ r + 2 ^ r := by rw [Nat.pow_succ]; omega
    have e3 : 2 ^ r = (2 ^ r - 1) + 1 := by omega
    show step (rounds parent loc r) b = _
    rw [e1, e2, seg_add parent loc assoc, ← e3, up_add]
    unfold step
    rw [ih b]
    cases hb : up parent (2 ^ r) b with
    | none => simp
    | some a => simp [ih a]

/-- `seg` agrees with the engine's recursion once it covers every ancestor. -/
theorem seg_eq_global (G : Nat → M)
    (hG : ∀ b, G b = match parent b with
      | none => loc b
      | some a => G a * loc b) :
    ∀ n b, up parent (n + 1) b = none → seg parent loc n b = G b := by
  intro n
  induction n with
  | zero =>
    intro b h
    rw [up_one] at h
    rw [hG b, h]; rfl
  | succ n ih =>
    intro b h
    rw [up_succ'] at h
    rw [hG b]
    cases hb : parent b with
    | none => simp [seg, hb]
    | some a =>
      rw [hb] at h
      simp only [seg, hb]
      rw [ih a h]

/-- **Pointer jumping is correct.** If bone `b` has no `2^r`-th ancestor, `r`
rounds leave it holding the engine's skeleton-space pose with no ancestor left. -/
theorem jump_correct (assoc : ∀ x y z : M, x * y * z = x * (y * z)) (G : Nat → M)
    (hG : ∀ b, G b = match parent b with
      | none => loc b
      | some a => G a * loc b)
    (r b : Nat) (h : up parent (2 ^ r) b = none) :
    rounds parent loc r b = (G b, none) := by
  rw [rounds_eq parent loc assoc, h]
  have e3 : 2 ^ r = (2 ^ r - 1) + 1 := by have := @Nat.one_le_two_pow r; omega
  rw [e3] at h
  rw [seg_eq_global parent loc G hG _ b h]

/-- The same with depth: a bone with at most `d` ancestors (its `(d+1)`-th
ancestor does not exist) is done after any `r` with `d < 2^r`. -/
theorem jump_correct_of_depth (assoc : ∀ x y z : M, x * y * z = x * (y * z)) (G : Nat → M)
    (hG : ∀ b, G b = match parent b with
      | none => loc b
      | some a => G a * loc b)
    (r d b : Nat) (hd : up parent (d + 1) b = none) (hr : d < 2 ^ r) :
    rounds parent loc r b = (G b, none) := by
  apply jump_correct parent loc assoc G hG
  have e : 2 ^ r = (2 ^ r - (d + 1)) + (d + 1) := by omega
  rw [e]
  exact up_none_add parent hd _

end

/-! ## Control

A three-bone chain `0 ← 1 ← 2` with "poses" that are lists under append, which
is associative but not commutative, so any mistake in order or round count
shows. Bone 2 has two ancestors: one round (`2^1 = 2` nodes) is not enough and
gives the wrong product; two rounds give the engine's `[0, 1, 2]`. -/

section Control

local instance : Mul (List Nat) := ⟨(· ++ ·)⟩

private def chainParent : Nat → Option Nat
  | 0 => none
  | n + 1 => some n

private def chainLoc (b : Nat) : List Nat := [b]

example : (rounds chainParent chainLoc 1 2).1 ≠ [0, 1, 2] := by decide
example : rounds chainParent chainLoc 2 2 = ([0, 1, 2], none) := by decide

end Control

end Bone.Proofs.Jump
