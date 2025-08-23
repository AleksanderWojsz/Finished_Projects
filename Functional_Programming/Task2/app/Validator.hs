module Validator (getErrosInProg) where
import Parser (Def(..), Prog(..))

-- returns:
-- No errors -> `Nothing`
-- Errors -> Just error_message
getErrosInProg :: Prog -> Maybe String
getErrosInProg (Prog defs) = 
    case checkDuplicateNames defs of
        Just error_message -> Just error_message
        Nothing -> case checkUniqueParameters defs of
            Just error_message -> Just error_message
            Nothing -> checkMain defs

checkDuplicateNames :: [Def] -> Maybe String
checkDuplicateNames defs = case checkforDuplicates names_list of 
                                Just duplicate_name -> Just ("Multiple combinators with the same name: " ++ duplicate_name)
                                Nothing -> Nothing
                           where names_list = map (\def -> defName def) defs

checkUniqueParameters :: [Def] -> Maybe String
checkUniqueParameters [] = Nothing
checkUniqueParameters (def:xs) = case checkforDuplicates (defPats def) of
                                    Just duplicate_name -> Just ("Combinator " ++ defName def ++ " has arguments with the same name: " ++ duplicate_name)
                                    Nothing -> checkUniqueParameters xs

checkMain :: [Def] -> Maybe String
checkMain defs = case correctMainFound of 
                    False -> Just "'main' not found or it has arguments"
                    True -> Nothing
                 where correctMainFound = length (filter 
                                                (\def -> defName def == "main" && null (defPats def)) 
                                                defs) == 1

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
