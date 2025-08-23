module Reducer (reduce) where
import Parser (Def(..), Expr(..), Prog(..), Name)
import Data.Map (Map, fromList, lookup)

type DefMap = Map Name Def
buildDefMap :: Prog -> DefMap
buildDefMap (Prog definitions) = fromList $ map (\def -> (defName def, renameArgs def)) definitions
 
renameArgs :: Def -> Def
renameArgs (Def name pats expr) = Def name (renamePats pats) (renameExpr expr)
    where
        renamePats ps = map (\pat -> "$" ++ pat) ps -- add $ to all arguments (left-hand side)
        renameExpr (exp1 :$ exp2) = (renameExpr exp1 :$ renameExpr exp2)  -- add $ to all arguments and only arguments (right-hand side)
        renameExpr (Var n)
            | n `elem` pats = Var ("$" ++ n)
            | otherwise = Var n

reduce :: Prog -> [Expr]
reduce prog = 
    case Data.Map.lookup "main" defMap of
        Just (Def _ [] expr) -> rpath defMap expr
        _ -> error "'main' not found or it has arguments"
    where defMap = buildDefMap prog

rpath :: DefMap -> Expr -> [Expr]
rpath defMap expr = expr : case rstep defMap 0 expr of
                            Just (_, expr') -> rpath defMap expr'
                            Nothing -> []

-- (name, value to substitute) -> expression to substitute in -> new expression
subst :: (Name, Expr) -> Expr -> Expr
subst (name, replacement) expr = case expr of
    e1 :$ e2 -> subst (name, replacement) e1 :$ subst (name, replacement) e2
    Var n | n == name -> replacement
          | otherwise -> Var n

{-
    1. Rekurencyjnie szukam wyrażenia do uproszczenia. Jak jest `(exp1 :$ exp2)` to próbuję uprościć `exp1` i jak uda się to kończę, a jak się nie uda to próbuje uprościć `exp2`.
    2. Po drodze liczę ile razy poszedłem "w lewo" (`exp1`), bo odpowiada to liczbie argumentów, które będę mógł przyłożyć do kombinatora, którego znajdę. Jak pójdę w prawo, to licznik się zeruje.
    3. Jak znajdę wyrażenie do uproszczenia to szukam go w mapie. 
        - Jak znajdę to sprawdzam, czy mam wystarczająco argumentów (pójść w lewo), żeby je uprościć.
            - jak tak to zwracam listę potrzebnych zmiennych [Pat] (np. ["x","y","z"]) i definicję kombinatora Expr (np. `(Var "x" :$ Var "z") :$ (Var "y" :$ Var "z")`). 
            - jak nie to 'Nothing', bo wyrażenia nie da się w pełni uprościć.
        - Jak kombinatora nie ma w mapie (trafiliśmy na stałą) to też 'Nothing'.
    4. Wracając z rekurencji:
        - Jak mam `Nothing`, to próbuję upraszać `exp2`, albo mówię, że wyrażenia w ogóle nie da się uprościć
        - Jak mam `Just ([potrzebny argument], aktualny wynik)`, to usuwam pierwszą potrzebną zmienną z listy i za jej wartość w aktualnym wyniku podstawiam `exp2` korzystając z `subst`
        - Jak mam `Just ([], aktualny wynik)`, czyli wszystkie zmienne zostały podstawione, to po prostu doklejam `exp2` do aktualnego wyniku.

*Komentarz po polsku dla łatwiejszego zrozumienia
-}
-- Returns `Just a` if expression can be simplified, else `Nothing`
-- DefMap -> number of times `exp1` was chosen in a row -> expression to simplify -> Maybe ([argument left to substitute], current result)
rstep :: DefMap -> Int -> Expr -> Maybe ([String], Expr)
rstep defMap leftDepth (exp1 :$ exp2) = 
    case rstep defMap (leftDepth + 1) exp1 of
        Just ([], a) -> Just ([], a :$ exp2)
        Just (missingArgs, a) -> Just (tail missingArgs, subst (head missingArgs, exp2) a)
        Nothing -> case rstep defMap 0 exp2 of
            Just ([], a) -> Just ([], exp1 :$ a)
            Just ((_:_), _) -> error "Impossible state. Expression is still being reduced but no more args will be passed, as it is `exp2`"
            Nothing -> Nothing
rstep defMap leftDepth (Var name) = 
    case Data.Map.lookup name defMap of
        Just (Def _ pats expr) -> -- Definition is in the map
            if leftDepth < length pats then Nothing else Just (pats, expr)
        Nothing -> Nothing -- Definition is not in the map, so expression cannot be simplified
