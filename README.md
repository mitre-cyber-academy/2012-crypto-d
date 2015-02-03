Crypto Challenge -- 400 points.

Summary: Two files are available to the solvers: "flag.encrypted", which 
contains the flag padded to 256 bits, then encrypted by the cipher described 
in the second file "encryptor.c".  They are also informed of a network 
service that runs encryptor with the secret key.

	I've included a file "key" which contains the secret key used to 
encrypt the flag.  encryptor.c is used as follows:

	encryptor key input

	It outputs the ciphertext on stdout.  My intention is that the network
service just receives messages, uses them to create input files, runs
encryptor with key and that file, and returns the result as a message.  I don't 
have the time/skills to do this part of the development.  Also, I'm not providing
a makefile; this program only needs the OpenSSL library plus standard C libraries.

Challenge files:

	index.html -- The challenge entry point.
	/files -- this should be a readable sub-folder.
      /files/flag.encrypted -- pointed to by index.html
      /files/encryptor.c -- pointed to by index.html

Building:

	gcc -std=gnu99 -I/usr/local/include -Wall -c -o encryptor.o encryptor.c
	gcc -std=gnu99 -Wall -L/usr/local/lib -o encryptor.exe encryptor.o -lcrypt
#If your system doesn't build, change this previous line to 
	gcc -std=gnu99 -Wall -L/usr/local/lib -o encryptor.exe encryptor.o -lcrypto

	Requires libcrypt.


Solution walkthrough:

	encryptor.c gives the exact design and implementation of the mystery cipher.
It turns out the mystery cipher is a 2-round Feistel cipher; the round functions are
made from existing crypto primitives (DES encryption), and the two round functions are
each different.  The thing is, it's possible to do a message recovery on a 2-round 
Feistel cipher regardless of how good or bad the round function is, when you can get
encryptions of your choice.  

	Walkthrough:
	1.  Call X and Y the first and second parts of flag.encrypted.  So
		X = 3C 76 ... 8D EC
		Y = D6 8B ... 27 20
	2.  In order to decrypt, we want to calculate R = f2(X) xor Y and then L = f1(R)
          xor X.  Let's encrypt /dev/null: we get null.encrypted.  Call A and B the
	    first and second parts of null.encrypted.  So
		A = EE 04 ... C4 FA 
		B = 9A 57 ... 69 DF
	3.  The input to f2 in encrypting /dev/null was A.  We want it to be X; we make 
          this happen by calculating the XOR of A and X and encrypting an input whose
	    first half is A ^ X and whose second half is 0.  This can be found in the file
	    "query".  The result of encrypting "query" is "query.encrypted."  The first half
          of query.encrypted is equal to X.  The second half is f2(X).  But we know that
          Y = f2(X) ^ R where R is the right half of the flag.  So if we take the xor of
          the second half of query.encrypted and the second half of flag.encrypted, we should
          get the second half of the flag.  The result of this is in flag.recovered; when 
          viewed as text, the flag appears.

	Just for fun, I'll note it's possible to recover the ENTIRE message: now that we know
      the right half, we can now input flag.recovered to the encryption oracle.  The left half
      is all 0s and the right half is R, so the left half of the output will be f1(R).  But X =
      L ^ f1(R) so if we xor these values together we get L.  flag.recovered.encrypted is the
      output of the encryptor on input flag.recovered, and its left half ^ X is "OWDqk3ij3WpQXwKO".
	The result, combining the left and right halves is in the file "flag" and you can verify that
	running encryptor on flag gives flag.encrypted as an output.

