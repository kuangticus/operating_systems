# OTP
Client and server side encoding and decoding

The Keygen program is used to make a encoding and decoding key that will be used to encode plaintext (human readable) into ciphtertext or, decode ciphertext into plaintext (human readable). 

Encoding uses the alphabet and the random index generation (the key from earlier) to make plain text that is human readable into ciphertext which is a bunch of letters jumble up so that they become illegible hence encoded. There are two parts to the encoding program which is the client and server, the client is solely responsible for sending the plaintext and recieving the ciphertext, while the server is responsible for taking the recieve plaintext and then encoding it into ciphertext and then sending it back to the client.

Decoding uses the key that was first generated and the alphabet to solve the encoded message from the encoding programs and the changing it back from ciphertext to plaintext so the message can be read again, the set is exact the same as the encoder set up for the client and the server, essentially these programs do the exact opposite of their counterparts.
