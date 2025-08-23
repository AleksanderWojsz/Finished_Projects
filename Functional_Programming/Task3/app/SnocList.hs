module SnocList where
import Control.Applicative (Alternative(..))

newtype SnocList a = SnocList {unSnocList :: [a]}
snoc :: SnocList a -> a -> SnocList a
snoc (SnocList xs) x = SnocList (x:xs)

toList :: SnocList a -> [a]
toList l = reverse $ unSnocList l

fromList :: [a] -> SnocList a
fromList l = SnocList $ reverse l

instance Eq a => Eq (SnocList a) where
    SnocList l1 == SnocList l2 = l1 == l2

instance Show a => Show (SnocList a) where
    show l = show $ toList l 

instance Semigroup (SnocList a) where
    SnocList l1 <> SnocList l2 = SnocList $ l2 ++ l1

instance Monoid (SnocList a) where
    mempty = SnocList []

instance Functor SnocList where
    fmap f (SnocList l) = SnocList (fmap f l)

instance Applicative SnocList where
    SnocList l1 <*> SnocList l2 = SnocList $ l1 <*> l2
    pure v = SnocList [v]

instance Alternative SnocList where
    l1 <|> l2 = l1 <> l2
    empty = SnocList []
