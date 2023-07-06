#include <bits/stdc++.h>

#include <openssl/sha.h>

using namespace std;

int main(){

  unsigned char hash[SHA256_DIGEST_LENGTH];

  string s = "never, under any circumstances, open this: https://youtu.be/dQw4w9WgXcQ";

  SHA256((unsigned char*)s.c_str(), s.size(), hash);

  stringstream ss;

  for(int i = 0; i < SHA256_DIGEST_LENGTH; i++){
    ss << hex << setw(2) << setfill('0') << static_cast<int>(hash[i]);
  }
  
  cout << ss.str() << endl;

  return 0;
}