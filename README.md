# OmcTextDecoder
Decode/Encode Samsung CSC files

## How To Use
Download the latest binary [here](https://github.com/saadelasfur/OmcTextDecoder/releases/latest)

Decode
```shell
./omc-decoder -i cscfeature.xml -o cscfeature_decoded.xml
```
or try to decode all files under a directory
```shell
./omc-decoder -i omc -o omc_decoded
```

Encode
```shell
./omc-decoder -e -i cscfeature_decoded.xml -o cscfeature.xml
```
or try to encode all files under a directory
```shell
./omc-decoder -e -i omc_decoded -o omc_encoded
```

## How To Build
```shell
make
```

then the binary is output to `out/`
