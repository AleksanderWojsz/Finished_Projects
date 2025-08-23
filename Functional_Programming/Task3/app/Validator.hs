module Validator (getErrorsInProg) where
import Parser (Def(..), Match(..), Prog(..), Pat(..))
import Data.Map (map, foldr)
import Reducer (buildMatchesMap)

-- returns:
-- Errors -> Left error_message
-- No errors -> Right ()
getErrorsInProg :: Prog -> Either String ()
getErrorsInProg (Prog defs) = do
    checkUniqueParameters defs
    checkNumberOfParams defs
    checkMain defs

checkUniqueParameters :: [Def] -> Either String ()
checkUniqueParameters [] = Right ()
checkUniqueParameters (Def matches : restDefs) = do 
    checkMatches matches
    checkUniqueParameters restDefs
  where
    checkMatches :: [Match] -> Either String ()
    checkMatches [] = Right ()
    checkMatches (Match name pats _ : restOfTheMatches) =
        case checkforDuplicates (allVars pats) of
            Just dup -> Left ("Combinator " ++ name ++ " has arguments with the same name: " ++ dup)
            Nothing -> checkMatches restOfTheMatches

    allVars :: [Pat] -> [String]
    allVars [] = []
    allVars (p : ps) = collectVars p ++ allVars ps
    collectVars :: Pat -> [String]
    collectVars (PVar v) = [v]
    collectVars (PApp _ subPats) = allVars subPats

checkNumberOfParams :: [Def] -> Either String ()
checkNumberOfParams defs = if Data.Map.foldr allTrue True (Data.Map.map (allTheSame . toNumberOfParams) matchesMap) then Right () else Left "Combinators with the same names, but a different number of parameters"
    where 
        matchesMap = buildMatchesMap $ Prog defs
        toNumberOfParams matches = Prelude.map (\match -> length $ matchPats match) matches
        allTheSame (x1:x2:xs) = x1 == x2 && allTheSame (x2:xs) 
        allTheSame _ = True 
        allTrue v res = res && v  

checkMain :: [Def] -> Either String ()
checkMain defs = if countMains defs == 1 then Right () else Left "'main' not declared exactly once"
    where
        countMains :: [Def] -> Int
        countMains [] = 0
        countMains (Def matches : rest) = countMainMatches matches + countMains rest

        countMainMatches :: [Match] -> Int
        countMainMatches [] = 0
        countMainMatches (Match name pats _ : restOfTheMatches)
            | name == "main" && null pats = 1 + countMainMatches restOfTheMatches
            | name == "main" = error "'main' has arguments"
            | otherwise = 0

checkforDuplicates :: (Ord a, Show a) => [a] -> Maybe String
checkforDuplicates l = checkforDuplicates' $ mergeSort l

checkforDuplicates' :: (Ord a, Show a) => [a] -> Maybe String
checkforDuplicates' [] = Nothing
checkforDuplicates' [_] = Nothing
checkforDuplicates' (x1:x2:xs)
    | x1 == x2 = Just (show x1)
    | otherwise = checkforDuplicates' (x2:xs)

-- Source -> lab04 (implementation is mine)
merge :: Ord a => [a] -> [a] -> [a]
merge x [] = x
merge [] y = y
merge (x:xs) (y:ys)
    | x < y = x : merge xs (y:ys)
    | otherwise = y : merge (x:xs) ys

-- Source -> lab04 (implementation is mine)
mergeSort :: Ord a => [a] -> [a]
mergeSort [] = []
mergeSort [x] = [x]
mergeSort l = merge (mergeSort l1) (mergeSort l2) where
  l1 = take (length l `div` 2) l
  l2 = drop (length l `div` 2) l
