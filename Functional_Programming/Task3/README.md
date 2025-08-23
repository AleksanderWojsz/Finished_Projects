Execution example:
```
cabal run -- zadanie3 examples/e1.hs
```

Generate *.tar.gz*:
```
cabal sdist
```

Use *.tag.gz*:
```
<unpack the file>
cd package-name
cabal update
cabal install
```

