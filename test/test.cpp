//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//#define LRUCACHE8_ENABLE_INTRINSICS

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

#include "../LRU8Cache.h"
#include <string>

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

struct RawStringHash
{
  uint32_t operator() (const char * k) const
  {
    uint32_t hash = 5381;
    char c;
    while ((c = *k++) != 0)
    {
      hash = ((hash << 5) + hash) + c;
    }
    return hash;
  }
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

struct RawStringEqual
{
  uint32_t operator() (const char * k1, const char *k2) const
  {
    return (strcmp (k1, k2) == 0);
  }
};

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void run_test ()
{
  bool ok = false;
  void *out = NULL;

  {
    lru_cache8<const char *, void *> cache;    

    cache.write ("one", (void *) 1);
    cache.write ("two", (void *) 2);
  }

  {
    lru_cache8<std::string, void *> cache;

    cache.write ("one",   (void *) 0);
    cache.write ("one",   (void *) 1);
    cache.write ("two",   (void *) 2);
    cache.write ("three", (void *) 3);
    cache.write ("four",  (void *) 4);
    cache.write ("five",  (void *) 5);
    cache.write ("six",   (void *) 6);

    ok = cache.read ("one", &out);

    cache.write ("seven", (void *) 7);
    cache.write ("eight", (void *) 8);
    cache.write ("nine",  (void *) 9);
    cache.write ("ten",   (void *) 10);
    cache.write ("eleven",  (void *) 11);
    cache.write ("twelve",  (void *) 12);
    cache.write ("thirteen", (void *) 13);
    cache.write ("fourteen", (void *) 14);
    cache.write ("fifteen",  (void *) 15);
  }

  {
    lru_cache8<const char *, void *, RawStringHash, RawStringEqual> cache;

    cache.write ("one", (void *) 1);
    cache.write ("two", (void *) 2);

    ok = cache.read ("one", &out);
    ok = cache.read ("two", &out);
  }

#if LRUCACHE8_CPP11
  {
    lru_cache8<const char *, void *, std::hash<const char *>> cache;

    cache.write ("one", (void *) 1);
    cache.write ("two", (void *) 2);

    ok = cache.read ("one", &out);
    ok = cache.read ("two", &out);
  }
#endif

  {
    lru_cache8<uint32_t, void *> cache;

    cache.write (0, (void *) 10);
    cache.write (1, (void *) 11);
    cache.write (3, (void *) 13);

    ok = cache.read (0, &out);
    ok = cache.read (7, &out);
    ok = cache.read (3, &out);
  }

  {
    lru_cache8<std::string, std::string> cache;

    cache.write ("pi", "3.14");
    cache.write ("street", "sesame");

    std::string val;
    ok = cache.read ("pi", &val);

    val = "3.142";

    cache.write ("pi", val);
    ok = cache.read ("pi", &val);
    ok = cache.read ("street", &val);
  }

  {
    lru_cache8<std::string *, void *> cache;

    std::string tk ("test");
    cache.write (&tk, (void *) 0);
  }

#if LRUCACHE8_CPP11
  {
    enum enomnom
    {
      nom_nom_1,
      nom_nom_2,
    };

    lru_cache8<enomnom, void *> cache;
    cache.write (nom_nom_1, (void *) 0);
  }
#endif

#if 0
  {
    struct dumb
    {
      const char *m_str;
      dumb () {}
      dumb (const char *str) : m_str (str) {}
      bool operator== (const dumb &d)
      {
        return (strcmp (this->m_str, d.m_str) == 0);
      }
    };

    lru_cache8<dumb, void *> cache;
    cache.write ("one", (void *) 0);
  }
#endif
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void run_debug ()
{
  // The appended comments on each line shows the internal cache state 
  // (of the data location and keys) after execution of each statement

  lru_cache8<uint32_t, std::string> cache;   // LRU == 0, MRU == INVALID

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
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

int main ()
{
  run_test ();
  run_debug ();

  return 0;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
