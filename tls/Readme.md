# Private certificate authority

I'm using the LimeGPSRemote from a script running in a browser. That means that the browser has to contact different servers. 
When the original host delivering the content is TLS secured the browser will complain if it has to call HTTP to an non TLS secured server e.g.:

```
suitcase-main.js:73 Mixed Content: The page at 'https://sierrademo.foo.bar/app/suitcase' was loaded over HTTPS, 
but requested an insecure resource 'http://<my local LimeGPSRemote address>:8080/stop'. This content should also be served over HTTPS.
```

One way to overvcome this is to enable insecure conent through the Browser policies which ends often in a ugly "Unsecure Page" warning in the URL bar.
To avid that it is possible to run LimeGPSRemote as HTTPS server. As it is a private service a little CA with self signed certifcates should be enough.

## create relevant files:


```
$ mkdir newcerts
$ touch index.txt
$ echo '01' > serial
```

## setup CA
generate the CA private key (the resulting file (*limegpsremote.local.CA.key*) should never be shared with anyone):
```
$ openssl genrsa -out limegpsremote.local.CA.key 2048
Generating RSA private key, 2048 bit long modulus
......................................................+++
.......+++
e is 65537 (0x10001)
```

Generate the public root certificate (this must be installed to the browser or operating system which should trust the device and can be safely sharead)
Fill the questions according to your needs.

```
$ openssl req -new -x509 -key limegpsremote.local.CA.key -out limegpsremote.local.CA.crt
You are about to be asked to enter information that will be incorporated
into your certificate request.
What you are about to enter is what is called a Distinguished Name or a DN.
There are quite a few fields but you can leave some blank
For some fields there will be a default value,
If you enter '.', the field will be left blank.
-----
Country Name (2 letter code) [AU]:DE
State or Province Name (full name) [Some-State]:Bavaria
Locality Name (eg, city) []:Munich
Organization Name (eg, company) [Internet Widgits Pty Ltd]:tsei
Organizational Unit Name (eg, section) []:tsei.mdn
Common Name (e.g. server FQDN or YOUR name) []:.
Email Address []:An optional company name []:.

```
Copy the sample files limegpsremote.sample.cnf and limegpsremote.sample.server.extensions.cnf
```
cp limegpsremote.sample.cnf limegpsremote.local.cnf
cp limegpsremote.sample.server.extensions.cnf limegpsremote.local.server.extensions.cnf
```
Edit the hostname (DNS) and IP address (you can have multiple with IP.1=, IP.2=, IP.3 ... and DNS.1=, DNS.2=, DNS.3= ...) in *limegpsremote.local.server.extensions.cnf* and *limegpsremote.local.cnf* file.

Genrate then the CSR (Certificate Signing Request) from:
```
$ openssl req -new -out limegpsremote.local.server.csr -config limegpsremote.local.cnf
Generating a RSA private key
............+++++
..............................................................................................................+++++
writing new private key to 'limegpsremote.local.server.key'
-----
```
Now we sign the Server Certificate:

```
openssl ca -config ca.cnf -out limegpsremote.local.server.crt -extfile limegpsremote.local.server.extensions.cnf  -in limegpsremote.local.server.csr
Using configuration from ca.cnf
Check that the request matches the signature
Signature ok
The Subject's Distinguished Name is as follows
countryName           :PRINTABLE:'DE'
stateOrProvinceName   :ASN.1 12:'Bavaria'
localityName          :ASN.1 12:'Munich'
organizationName      :ASN.1 12:'TSEI'
commonName            :ASN.1 12:'*.tsei.mdn'
Certificate is to be certified until May 28 14:27:31 2023 GMT (365 days)
Sign the certificate? [y/n]:y


1 out of 1 certificate requests certified, commit? [y/n]y
Write out database with 1 new entries
Data Base Updated
```

you may face an error message indicating that the index.txt.attr file is not there on first time executing the command
To load the Certificate file you may want to strip the infomrational part before "---- Begin Certificate ----":

```
openssl x509 -in limegpsremote.local.server.crt >limegpsremote.local.server.short.crt
```


 