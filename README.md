LRU8Cache
========================================================================
A very small and fast 8-element LRU cache.

### Usage
~~~~~~~~~~cpp
#include "LRU8Cache.h"

MainStorage mainStorage;
LRU8Cache<std::string, std::string *> cache;  

int main ()
{
  std::string *data = NULL;
  if (!cache.read ("name", &data))  // fast read
  {
    mainStorage.Read (data);        // slow read
    cache.write ("name", data);
  }
}
~~~~~~~~~~

### LRU Algorithm
A software implementation of the "Reference Matrix" method typically used in hardware combined with a linear probing hash table implementation

#### Reference
- [Hacker's Delight: 7-9](http://www.amazon.co.uk/Hackers-Delight-Henry-S-Warren/dp/0321842685/ref=dp_ob_title_bk)
- [PHP's new hashtable implementation](https://nikic.github.io/2014/12/22/PHPs-new-hashtable-implementation.html)
- [Linear Probing](https://en.wikipedia.org/wiki/Linear_probing)

#### Demo
~~~~~~~~~~cpp
#include "LRU8Cache.h"
// The appended comments on each line shows the internal cache state 
// (of the data location and keys) after execution of each statement

LRU8Cache<uint32_t, std::string> cache;   // LRU == 0, MRU == INVALID

cache.write (0, "zero");                  // LRU == 1 [Key == nil], MRU == 0 [Key == 0]
cache.write (1, "one");                   // LRU == 2 [Key == nil], MRU == 1 [Key == 1]
cache.write (2, "two");                   // LRU == 3 [Key == nil], MRU == 2 [Key == 2]
cache.write (3, "three");                 // LRU == 4 [Key == nil], MRU == 3 [Key == 3]
cache.write (4, "four");                  // LRU == 5 [Key == nil], MRU == 4 [Key == 4]
cache.write (5, "five");                  // LRU == 6 [Key == nil], MRU == 5 [Key == 5]
cache.write (6, "six");                   // LRU == 7 [Key == nil], MRU == 6 [Key == 6]
cache.write (7, "seven");                 // LRU == 0 [Key == 0],   MRU == 7 [Key == 7]

std::string out;
cache.read (0, &out);                     // LRU == 1 [Key == 1], MRU == 0 [Key == 0]
cache.read (3, &out);                     // LRU == 1 [Key == 1], MRU == 3 [Key == 3]

cache.write (8, "eight");                 // LRU == 2 [Key == 2], MRU == 1 [Key == 8]
cache.write (9, "nine");                  // LRU == 4 [Key == 4], MRU == 2 [Key == 9]
cache.write (10, "ten");                  // LRU == 5 [Key == 5], MRU == 4 [Key == 10]
cache.write (11, "eleven");               // LRU == 6 [Key == 6], MRU == 5 [Key == 11]
cache.write (12, "twelve");               // LRU == 7 [Key == 7], MRU == 6 [Key == 12]
cache.write (13, "thirteen");             // LRU == 0 [Key == 8], MRU == 7 [Key == 13]
cache.write (14, "fourteen");             // LRU == 3 [Key == 9], MRU == 0 [Key == 14]
cache.write (15, "fifteen");              // LRU == 1 [Key == 8], MRU == 3 [Key == 15]

cache.clear ();                           // LRU == 0, MRU == INVALID
~~~~~~~~~~
