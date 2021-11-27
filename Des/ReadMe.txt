Parallel implementation of DES algorithm.

Create Plaintext.txt and enter your original text in there.

To run the code:

In Windows:

Make sure to download C++ compiler.

-Parallel implementation: 
  >Open VSCode terminal in the /Des/ directory
  >g++ -o p5 -fopenmp parallelimplementation.cpp
  >./p5
  
-Serial implementation: 
  >Open VSCode terminal in the /Des/ directory
  >g++ -o s5 -fopenmp serialimplementation.cpp
  >./s5
  
Encrypted text is in the file: encrypted.txt

Decrypted text is in the file: decrypted.txt
