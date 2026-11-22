echo "OpenSSL start"

openssl genrsa -out server_program_code.ca.key 2048

openssl req -new -key server_program_code.ca.key -out server.csr

openssl x509 -req -in server.csr -signkey server_program_code.ca.key -out server_program_code.ca.cert -days 365

echo "All OpenSSL commands completed"
