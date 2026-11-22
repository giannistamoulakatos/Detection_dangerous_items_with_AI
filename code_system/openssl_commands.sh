echo "OpenSSL start"

openssl genrsa -out private.key 2048

openssl req -new -key private.key -out request.csr

openssl x509 -req -in request.csr -signkey private.key -out certificate.crt -days 365

echo "All OpenSSL commands completed"
