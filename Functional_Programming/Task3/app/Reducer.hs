module Reducer (reduce, buildMatchesMap) where
import Parser (Def(..), Match(..), Expr(..), Prog(..), Pat(..), Name)
import Zipper
import SnocList (SnocList, snoc, fromList)
import Data.Map (Map, fromListWith, lookup)
import Control.Monad.State (modify, execState, State, gets)
import Control.Monad (when)

stepsLimit :: Int
stepsLimit = 30

matchTriesPerArgLimit :: Int
matchTriesPerArgLimit = 30

type MatchesMap = Map Name [Match]

data StateData = StateData {stateMatchesMap :: MatchesMap, stateLocs :: SnocList Loc}
type StateMonad a = State StateData a

saveLocForPrinting :: Loc -> StateMonad ()
saveLocForPrinting newLoc = modify $ \s -> s {stateLocs = snoc (stateLocs s) newLoc}

saveLocListForPrinting :: SnocList Loc -> StateMonad ()
saveLocListForPrinting l = modify $ \s -> s {stateLocs = stateLocs s <> l}

buildMatchesMap :: Prog -> MatchesMap
buildMatchesMap (Prog definitions) = fromListWith (++) $ map (\def -> (matchName $ head $ defMatches def, map renameArgs $ defMatches def)) $ reverse definitions -- `reverse` because `fromListWith` concatenates elements from last to first

renameArgs :: Match -> Match
renameArgs (Match name pats expr) = Match name (map renamePat pats) (renameExpr expr)
    where
        renamePat (PVar v) = PVar ("$" ++ v)
        renamePat (PApp con ps) = PApp con (map renamePat ps)

        renameExpr (exp1 :$ exp2) = renameExpr exp1 :$ renameExpr exp2  -- add $ to all arguments and only arguments (right-hand side)
        renameExpr (Var n)
            | n `elem` varsInPats = Var ("$" ++ n) -- 'check if this Var was also on the left-hand side'
            | otherwise = Var n
        renameExpr (Con n) = Con n

        varsInPats :: [Name]
        varsInPats = concat (map collect pats)
        collect :: Pat -> [Name]
        collect (PVar v) = [v]
        collect (PApp _ ps) = concat (map collect ps)

-- (name, value to substitute) -> expression to substitute in -> new expression
subst :: (Name, Expr) -> Expr -> Expr
subst (name, replacement) expr = case expr of
    e1 :$ e2 -> subst (name, replacement) e1 :$ subst (name, replacement) e2
    Var n | n == name -> replacement
          | otherwise -> Var n
    Con c -> Con c

reduce :: Prog -> SnocList Loc
reduce prog = stateLocs $ execState state initialState
    where
        initialState = StateData (buildMatchesMap prog) mempty
        state = do
            let initialLoc = top (Var "main")
            saveLocForPrinting initialLoc
            rpath initialLoc stepsLimit

rpath :: Loc -> Int -> StateMonad ()
rpath loc stepsLeft = if stepsLeft == 0 then return () else
    do
        result <- rstep 0 True loc
        case result of
            Just (_, _, nextLoc) -> rpath nextLoc (stepsLeft-1)
            Nothing -> return ()


-- times we went left (number of args available) -> Whether to save reduction step -> Current loc -> Maybe (number of args left to collect, functionBeingReducedName, result loc)
-- `Current loc` and `result loc` have the same context, but different current exp
rstep :: Int ->  Bool -> Loc -> StateMonad (Maybe (Int, String, Loc))

rstep leftDepth saveStep loc@(_ :$ _, _) = do
    leftRes <- rstep (leftDepth + 1) saveStep (left loc)
    case leftRes of
        Just (0, name, newLoc) -> do  -- Reduction was already done, just go up
            return $ Just (0, name, up newLoc)
        Just (1, name, _) -> do -- One arg remaining. Collect it and do the reduction
            evalResult <- evaluate name (right . up . goMaxLeft $ loc)
            let newLoc = modifyHere (const evalResult) loc
            when saveStep $ saveLocForPrinting newLoc -- `when` source -> https://stackoverflow.com/a/5921045/22553511
            return $ Just (0, name, newLoc)
        Just (n, name, _) -> return $ Just (n-1, name, loc) -- Not all args were collected (We can return original `loc`, as we haven't reduce anything yet)
        Nothing -> do -- Couldn't reduce the left part. Try reducing the right one
            rightRes <- rstep 0 saveStep (right loc)
            case rightRes of
                Just (0, name, newLoc) -> return $ Just (0, name, up newLoc)
                Just (_, _, _) -> error "Impossible state. Combinator is being reduced and it needs more args but no more will be passed, as it is `exp2`. If there is not enough args we don't start reducing in the first place."
                Nothing -> return Nothing

rstep leftDepth saveStep loc@(Var name, _) = do
    matchesMap <- gets stateMatchesMap
    case Data.Map.lookup name matchesMap of
        Just ((Match _ pats expr):_) -> -- Definition is in the map
            if leftDepth < length pats then return Nothing -- Not enough args to reduce
            else if null pats -- No args needed
                then do
                    let newLoc = modifyHere (const expr) loc
                    when saveStep $ saveLocForPrinting newLoc -- `when` source -> https://stackoverflow.com/a/5921045/22553511
                    return $ Just (0, name, newLoc)
            else return $ Just (length pats, name, up loc) -- Start collecting args
        Just [] -> error "If definition is in the map, it should have at least one match"
        Nothing -> return Nothing -- Definition is not in the map, so expression cannot be reduced

rstep _ _ (Con _, _) = return Nothing -- Constructor cannot be reduced


evaluate :: String -> Loc -> StateMonad Expr
evaluate name argsLoc = do
    matchesMap <- gets stateMatchesMap
    case Data.Map.lookup name matchesMap of
        Just matches -> tryMatches matches argsLoc
        Nothing -> error "Definition name not in the MatchesMap"

-- Matches -> Loc with args in focus -> reduced expr
tryMatches :: [Match] -> Loc -> StateMonad Expr
tryMatches matches loc = tryMatches' matches loc False
    where
        tryMatches' [] _ False = error "non-exhaustive pattern matches"
        tryMatches' [] _ True = error "non-exhaustive pattern matches (skipped pattern matching for combinators where matchTriesPerArgLimit was exceeded. There was at least one such case)"
        tryMatches' (m:ms) argsLoc limitExceeded = do
            maybeResult <- tryMatch m argsLoc
            case maybeResult of
                Right (result, intermediateReductions) -> do
                    saveLocListForPrinting intermediateReductions
                    return result
                Left hitLimit -> tryMatches' ms argsLoc (hitLimit || limitExceeded)

-- Match -> Loc with args in focus -> Maybe (reduced expr, intermediate reductions)
tryMatch :: Match -> Loc -> StateMonad (Either Bool (Expr, SnocList Loc))
tryMatch (Match _ pats matchExpr) argsLoc = do
    matchesMap <- gets stateMatchesMap
    tryMatch' matchesMap pats argsLoc matchExpr (fromList []) matchTriesPerArgLimit
  where
        tryMatch' :: MatchesMap -> [Pat] -> Loc -> Expr -> SnocList Loc -> Int -> StateMonad (Either Bool (Expr,  SnocList Loc))
        tryMatch' _ _ _ _ _ (-1) = return $ Left True
        tryMatch' _ [] _ expr intermediateReductions _ = return $ Right (expr, intermediateReductions) -- No pats and args to check, or we have just checked them all and they matched
        tryMatch' matchesMap (param:params) loc expr intermediateReductions limit = case doesArgMatch param (fst loc) of
            Just substitutionsToDo -> do
                tryMatch' matchesMap params (goNextArg loc) (applyAllSubstitutions substitutionsToDo expr) intermediateReductions matchTriesPerArgLimit -- Arg matched; substitute it and move on to the next one
            Nothing -> do
                result <- rstep 0 False loc
                case result of
                    Just (_, _, loc') -> do
                        tryMatch' matchesMap (param:params) loc' expr (intermediateReductions `snoc` loc') (limit - 1) -- Reduction successful; try matching this arg again
                    Nothing -> return $ Left False -- Cannot reduce anymore; match unsuccessful


-- pat -> expr -> Maybe [Substitution to do]
doesArgMatch :: Pat -> Expr -> Maybe [(Name, Expr)]
doesArgMatch pat arg = match (patToExpr pat) arg
  where
    match :: Expr -> Expr -> Maybe [(Name, Expr)]
    match (Var v) e = Just [(v, e)]
    match (Con c1) (Con c2)
        | c1 == c2 = Just []
        | otherwise = Nothing
    match (p1 :$ p2) (a1 :$ a2) = do
        r1 <- match p1 a1
        r2 <- match p2 a2
        return $ r1 ++ r2
    match _ _ = Nothing

patToExpr :: Pat -> Expr
patToExpr (PVar v) = Var v
patToExpr (PApp name []) = Con name
patToExpr (PApp name args) = foldl (\acc e -> acc :$ e) (Con name) (map patToExpr args)

applyAllSubstitutions :: [(Name, Expr)] -> Expr -> Expr
applyAllSubstitutions subs expr = foldr (\s res -> subst s res) expr subs
