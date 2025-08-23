module Main where
import System.Environment (getArgs)
import Parser (fromHsString, Prog(..))
import Reducer (reduce)
import Validator (getErrorsInProg)
import SnocList (toList)

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
    putStrLn "Usage: zadanie3 [--help] [file]"
    putStrLn "  --help  - display this message"
    putStrLn "  file    - file with program to reduce" 

run :: String -> IO ()
run contents = 
    case getErrorsInProg prog of 
        Right () -> do 
            printList $ progDefs prog
            putStrLn "------------------------------------------------------------"
            printList . toList . reduce $ prog
        Left error_message -> error $ "Error: " ++ error_message
    where prog = fromHsString contents

printList :: Show a => [a] -> IO ()
printList list = putStr (unlines (map show list))
