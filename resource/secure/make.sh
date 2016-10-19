export OPENSSL_EXE=openssl

rm *.xml *.csr *.pem demoCA/* demoCA/private/*

mkdir output
mkdir -p 'demoCA/private'

echo ">> Generating cakey and cacert, input is input/openssl.cnf:"
$OPENSSL_EXE genrsa -out demoCA/private/cakey.pem 2048
$OPENSSL_EXE req -new -key demoCA/private/cakey.pem -out ca.csr -config input/openssl.cnf
$OPENSSL_EXE x509 -req -days 3650 -in ca.csr -signkey demoCA/private/cakey.pem -out demoCA/cacert.pem
mkdir -p 'demoCA/newcerts'
touch demoCA/index.txt
echo 01 > demoCA/serial

echo ">> Generating pubkey and signing PerftestPermissionsPub:"
$OPENSSL_EXE genrsa -out pubkey.pem 2048
$OPENSSL_EXE req -config input/pub.cnf -new -key pubkey.pem -out temp.csr
$OPENSSL_EXE ca -days 365 -in temp.csr -out pub.pem
$OPENSSL_EXE smime -sign -in input/PerftestPermissionsPub.xml -text -out signed_PerftestPermissionsPub.xml -signer demoCA/cacert.pem -inkey demoCA/private/cakey.pem

echo ">> Generating subkey and signing PerftestPermissionsSub:"
$OPENSSL_EXE genrsa -out subkey.pem 2048
$OPENSSL_EXE req -config input/sub.cnf -new -key subkey.pem -out temp.csr
$OPENSSL_EXE ca -days 365 -in temp.csr -out sub.pem
$OPENSSL_EXE smime -sign -in input/PerftestPermissionsSub.xml -text -out signed_PerftestPermissionsSub.xml -signer demoCA/cacert.pem -inkey demoCA/private/cakey.pem

for i in $( ls input/governances ); do
    echo Signing: $i
    $OPENSSL_EXE smime -sign -in input/governances/$i -text -out signed_$i -signer demoCA/cacert.pem -inkey demoCA/private/cakey.pem
done

cp demoCA/cacert.pem cacert.pem
