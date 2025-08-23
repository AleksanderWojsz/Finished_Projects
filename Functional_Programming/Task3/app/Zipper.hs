{-# LANGUAGE FlexibleInstances #-} -- Source: https://stackoverflow.com/a/8663534/22553511
{-# OPTIONS_GHC -Wno-incomplete-patterns #-}
module Zipper where
import Parser (Expr(..))


data Cxt = Top | L Cxt Expr | R Expr Cxt
type Loc = (Expr, Cxt)

top :: Expr -> Loc
top e = (e, Top)

left, right, up :: Loc -> Loc
left  (e1 :$ e2, c) = (e1, L c e2)
right (e1 :$ e2, c) = (e2, R e1 c)

up (e, L c r) = (e :$ r, c)
up (e, R l c) = (l :$ e, c)

modifyHere :: (Expr -> Expr) -> Loc -> Loc
modifyHere f (e, c) = (f e, c)

goMaxLeft :: Loc -> Loc
goMaxLeft loc@(e, _) = case e of
    _ :$ _ -> goMaxLeft (left loc)
    _ -> loc

goNextArg :: Loc -> Loc
goNextArg = right . up . up


instance {-# OVERLAPPING #-} Show Loc where
    showsPrec _ (focus, context) = showContext context False (showFocus focus)
        where
            showFocus :: Expr -> ShowS
            showFocus e = showString "{" . showsPrec 0 e . showString "}"

            showContext :: Cxt -> Bool -> ShowS -> ShowS
            showContext Top _ s = s
            showContext (L c r) _ s = showContext c True (s . showString " " . showsPrec 1 r) -- 1 to show parenthasis
            showContext (R l c) showParenthasis s = showContext c True (showsPrec 0 l . showString " " . showParen showParenthasis s)
