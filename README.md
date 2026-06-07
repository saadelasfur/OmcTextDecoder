# OmcTextDecoder
Decode/Encode Samsung CSC files

## How To Use
Download the latest binary [here](https://github.com/saadelasfur/OmcTextDecoder/releases/latest)

## Decode
Decode a single file:
```shell
./omc-decoder -i cscfeature.xml -o cscfeature_decoded.xml
```

Decode all files under a directory:
```shell
./omc-decoder -i omc -o omc_decoded
```

### In-place decode
You can decode files directly without specifying an output path:
```shell
./omc-decoder -p -i cscfeature.xml
```
Or for a directory:
```shell
./omc-decoder -p -i omc
```

## Encode
Encode a single file:
```shell
./omc-decoder -e -i cscfeature_decoded.xml -o cscfeature.xml
```

Encode all files under a directory:
```shell
./omc-decoder -e -i omc_decoded -o omc_encoded
```

### In-place encode
Encode files directly in place:
```shell
./omc-decoder -e -p -i cscfeature_decoded.xml
```
Or for a directory:
```shell
./omc-decoder -e -p -i omc_decoded
```

## How To Build
```shell
make
```

The binary is output to `out/`
