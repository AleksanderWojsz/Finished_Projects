module Main where
import System.Environment (getArgs)
import Parser (fromHsString, Prog(..))
import Reducer (reduce)
import Validator (getErrosInProg)

stepsLimitPrint :: Int
stepsLimitPrint = 30

-- Source -> lecture: https://github.com/mbenke/pf25/blob/main/w05io.md#przykład
main :: IO ()
main = do
    args <- getArgs
    case args of
        ["--help"] -> usage
        [f] -> readFile f >>= run
        _   -> usage

-- Source -> lecture: https://github.com/mbenke/pf25/blob/main/w05io.md#przykład
usage :: IO ()
usage = do
    putStrLn "Usage: zadanie2 [--help] [file]"
    putStrLn "  --help  - display this message"
    putStrLn "  file    - file with program to reduce" 

run :: String -> IO ()
run contents = 
    case getErrosInProg (Prog definitions) of 
        Nothing -> do 
            printList definitions
            putStrLn "------------------------------------------------------------"
            printList . take stepsLimitPrint . reduce $ (Prog definitions)
        Just error_message -> error $ "Error: " ++ error_message
    where (Prog definitions) = fromHsString contents

printList :: Show a => [a] -> IO ()
printList (x:xs) = do 
    print x
    printList xs
printList [] = return ()
