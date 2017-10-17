   #define PRId8   "d"
   #define PRId16   "d"
   #define PRId32   "ld"

   #define PRIi8   "i"
   #define PRIi16   "i"
   #define PRIi32   "li"
   #define PRIi64   "lli"
   #define PRIint   "i"

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

   #define PRIX8   "X"
   #define PRIXX8   "02X"
   #define PRIX16   "X"
   #define PRIXX16   "04X"
#ifndef PRIX32
   #define PRIX32   "lX"
#endif
#ifndef PRIXX32
   #define PRIXX32   "08lX"
#endif


#define CHAR   "s"

