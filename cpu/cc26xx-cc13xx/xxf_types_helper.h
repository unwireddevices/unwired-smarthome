   #define PRId8   "d"
   #define PRId16   "d"
   #define PRId32   "ld"

   #define PRIi8   "i"
   #define PRIi16   "i"
   #define PRIi32   "li"
   #define PRIi64   "lli"

   #define PRIu8   "u"
   #define PRIu16   "u"

#ifndef PRIu32
   #define PRIu32   "lu"
#endif

   #define PRIu64   "llu"


   #define PRIx8   "x"
   #define PRIx16   "x"
#ifndef PRIx32
   #define PRIx32   "lx"
#endif
   #define PRIXX8   "02X"

   #define PRIX8   "X"
   #define PRIX16   "X"
#ifndef PRIX32
   #define PRIX32   "lX"
#endif
#ifndef PRIXX32
   #define PRIXX32   "08lX"
#endif


#define CHAR   "s"

