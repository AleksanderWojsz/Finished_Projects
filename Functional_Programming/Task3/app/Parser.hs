module Parser (fromHsString, Prog(..), Match(..), Def(..), Expr(..), Pat(..), Name) where
import Language.Haskell.Parser (parseModule, ParseResult(..))
import Language.Haskell.Syntax

newtype Prog = Prog {progDefs :: [Def]}
type Name = String
data Def = Def { defMatches :: [Match] }
data Match = Match
    { matchName :: Name
    , matchPats :: [Pat]
    , matchRhs  ::Expr
    }

infixl 9 :$
data Expr
    = Var Name
    | Con Name
    | Expr :$ Expr
data Pat = PVar Name | PApp Name [Pat]

instance Show Def where
    showsPrec _ (Def matches) = foldr (.) id (map showMatchWithNewline (zip matches [0..]))
        where showMatchWithNewline (match, index) -- Add '\n' for all matches but the last one 
                | index == length matches - 1 = showString (show match)
                | otherwise = showString (show match) . showString "\n"

instance Show Match where
    showsPrec _ (Match name [] expr) = showString name . showString " = " . showsPrec 0 expr
    showsPrec _ (Match name pats expr) = showString name . showString " " . showPats pats . showString " = " . showsPrec 0 expr

showPats :: [Pat] -> (String -> String)
showPats [] = id
showPats [p] = showsPrec 0 p
showPats (p:ps) = showsPrec 0 p . showString " " . showPats ps

instance Show Pat where
    showsPrec _ (PVar name) = showString name
    showsPrec _ (PApp name []) = showString name
    showsPrec _ (PApp name pats) = showString "(" . showString name . showString " " . showPats pats . showString ")"

instance Show Expr where
    showsPrec = showsPrecExpr
        where
            showsPrecExpr :: Int -> Expr -> ShowS
            showsPrecExpr _ (Var name) = showString name
            showsPrecExpr _ (Con name) = showString name
            showsPrecExpr p (exp1 :$ exp2) = showParen (p > 0) $ (showsPrecExpr 0 exp1) . showString " " . (showsPrecExpr 1 exp2)

fromHsString :: String -> Prog
fromHsString s = Prog (fromParseResult $ parseModule s)

fromParseResult :: ParseResult HsModule -> [Def]
fromParseResult (ParseOk hsModule) = fromHsModule hsModule
fromParseResult (ParseFailed _ message) = error $ "Error: " ++ message

fromHsModule :: HsModule -> [Def]
fromHsModule (HsModule _ _ _ _ hsDecls) = concat (map parseDeclaration hsDecls)

parseDeclaration :: HsDecl -> [Def]
parseDeclaration (HsFunBind hsMatches) = map parseFunction hsMatches -- If there are combinators with the same name in adjacent lines, then `[HsMatch]` has more than one element
parseDeclaration (HsPatBind _ name hsRhs _) = [parsePattern name hsRhs]
parseDeclaration _ = error "Parse error"

-- Declarations with arguments
parseFunction :: HsMatch -> Def
parseFunction (HsMatch _ (HsIdent name) hsPats (HsUnGuardedRhs hsExp) _) = Def [Match name (map parseHsPat hsPats) (parseExpression hsExp)] 
parseFunction _ = error "Parse error"

-- Declarations without arguments
parsePattern :: HsPat -> HsRhs -> Def
parsePattern (HsPVar (HsIdent name)) (HsUnGuardedRhs hsExp) =  Def [Match name [] (parseExpression hsExp)]
parsePattern _ _ = error "Parse error"

parseHsPat :: HsPat -> Pat
parseHsPat (HsPParen pat) = parseHsPat pat
parseHsPat (HsPApp (UnQual (HsIdent constructorName)) hsPats) = PApp constructorName (map parseHsPat hsPats)
parseHsPat (HsPVar (HsIdent name)) = PVar name
parseHsPat _ = error "Parse error"

-- Recursively parses HsExp into Expr
parseExpression :: HsExp -> Expr
parseExpression (HsApp hsExp1 hsExp2) = parseExpression hsExp1 :$ parseExpression hsExp2
parseExpression (HsVar (UnQual (HsIdent v))) = Var v
parseExpression (HsCon (UnQual (HsIdent v))) = Con v
parseExpression (HsParen hsExp) = parseExpression hsExp
parseExpression _ = error "Parse error"
