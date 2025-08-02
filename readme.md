Simple OTP
----------

This is an easy-to-use tool to encrypt and decrypt data.
It uses one-time pad encryption.


Definition (One-Time Pad):
--------------------------

"The one-time pad is a private-key encryption scheme where
the key is a string of truly random bits equal in length to
the message. The encryption is performed by computing the
bitwise XOR of the message and the key. The one-time pad is
perfectly secure when the key is chosen uniformly at random,
is used only once, and kept secret."

   — from *Introduction to Modern Cryptography*  
     Jonathan Katz and Yehuda Lindell, 2nd ed. (Section 2.4.1)


How to Compile
--------------
Extremely simple, this project has no dependencies, just libc.

1. Inside project root, run:
```
make
```

2. To install system-wide:
```
sudo make install
```

All binaries are compiled into the `src/` directory. After compilation,
you will find:

- `otp`: the main OTP tool
- `genkey`: used to generate cryptographically secure random keys


Usages
------
```
 Simple OTP 2.0.0 (c) 2022-2025 trusted-ws 

 Usage: otp <filename> <keyfile> <output> [options]

 Required arguments:
   <filename>             Path of file to be encrypted.
   <keyfile>              Path of key file.
   <output>               Output filename.

 Unique arguments:
   --show <filename>      Show details of encrypted file (OTP only).
   --help / -h            Show this message.
   --version              Show the version.

 Optional arguments:
   --force / -f           Force overwriting when the output file exists.
   --description <text>   Set a description text to the encrypted file.
   --no-warnings          Suppress warning messages during execution.
   --ignore-otp-filetype  Ignore OTP filetype header during encryption.

 Notes:
   If the key size is smaller than the plaintext, the key will be reused cyclically.

   You are free to use this software as you wish, but we highly recommend following
   best practices whilst using OTP, such as employing a truly random key as long as
   plaintext, never reusing keys, securely distributing and storing keys, and
   destroying them after use.

```


Extras
------
Encrypted files generated without the `--ignore-otp-filetype` flag include a custom
binary header that defines a unique file type. To allow the file command to
recognize these files, you can use the *`simple_otp_type.magic`* file located in the
project root directory.

1. To install it you can:
```bash
# make sure to have a backup:
sudo cp /etc/magic /etc/magic.bak

# install with:
sudo tee -a /etc/magic < simple_otp_type.magic
```

2. Recompile file database:
```bash
sudo file -C -m /etc/magic
```


References:
-----------

- Claude Shannon, "Communication Theory of Secrecy Systems,"  
  *Bell System Technical Journal*, vol. 28, no. 4, pp. 656–715, 1949.  
  https://ieeexplore.ieee.org/document/6769090

- Jonathan Katz and Yehuda Lindell, *Introduction to Modern Cryptography*,  
  2nd ed., CRC Press, 2014. ISBN: 978-1466570269
