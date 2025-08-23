Execution example:
```
cabal run -- zadanie2 examples/peanob.uhs
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

