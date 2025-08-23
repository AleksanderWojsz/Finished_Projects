module Parser (fromHsString, Prog(..), Def(..), Expr(..), Name) where
import Language.Haskell.Parser
import Language.Haskell.Syntax

data Def = Def {defName :: Name, defPats :: [Pat], defExpr :: Expr}
data Expr = Var Name | Expr :$ Expr
type Pat = Name
type Name = String
newtype Prog = Prog {progDefs :: [Def]}

instance Show Def where
    showsPrec _ (Def name [] expr) = showString name . showString " = " . (showString $ show expr) 
    showsPrec _ (Def name pats expr) = showString name . showString " " . (showString $ unwords pats) . showString " = " . (showString $ show expr) 

instance Show Expr where
    showsPrec _ (Var name) = showString name
    showsPrec p (exp1 :$ exp2) = showParen (p > 0) $ (showsPrec 0 exp1) . showString " " . (showsPrec 1 exp2)

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
parseFunction (HsMatch _ (HsIdent name) hsPats (HsUnGuardedRhs hsExp) _) = 
    Def name
        (map (\hsPVar -> case hsPVar of
                        HsPVar (HsIdent arg_name) -> arg_name
                        _ -> error "Parse error"
                        ) hsPats) 
        (parseExpression hsExp)
parseFunction _ = error "Parse error"

-- Declarations without arguments
parsePattern :: HsPat -> HsRhs -> Def
parsePattern (HsPVar (HsIdent name)) (HsUnGuardedRhs hsExp) =  Def name [] (parseExpression hsExp)
parsePattern _ _ = error "Parse error"

-- Recursively parses HsExp into Expr
parseExpression :: HsExp -> Expr
parseExpression (HsApp hsExp1 hsExp2) = parseExpression hsExp1 :$ parseExpression hsExp2
parseExpression (HsVar (UnQual (HsIdent v))) = Var v
parseExpression (HsCon (UnQual (HsIdent v))) = Var v
parseExpression (HsParen hsExp) = parseExpression hsExp
parseExpression _ = error "Parse error"
