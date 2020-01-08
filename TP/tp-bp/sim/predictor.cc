#include "predictor.h"

/////////////// STORAGE BUDGET JUSTIFICATION ////////////////

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

// Constructeur du prédicteur
PREDICTOR::PREDICTOR(char *prog, int argc, char *argv[])
{
#ifdef BIMODAL
   // La trace est tjs présente, et les arguments sont ceux que l'on désire
   if (argc != 2) {
      fprintf(stderr, "usage: %s <trace> pcbits countbits\n", prog);
      exit(-1);
   }

   uint32_t pcbits    = strtoul(argv[0], NULL, 0);
   uint32_t countbits = strtoul(argv[1], NULL, 0);

   nentries = (1 << pcbits);        // nombre d'entrées dans la table
   pcmask   = (nentries - 1);       // masque pour n'accéder qu'aux bits significatifs de PC
   countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
   table    = new uint32_t[nentries]();
#else
   // La trace est tjs présente, et les arguments sont ceux que l'on désire
   if (argc != 2) {
      fprintf(stderr, "usage: %s <trace> histlen countbits\n", prog);
      exit(-1);
   }

   uint32_t histlen   = strtoul(argv[0], NULL, 0);
   uint32_t countbits = strtoul(argv[1], NULL, 0);

   nentries = (1 << histlen);        // nombre d'entrées dans la table
   histval  = 0;
   countmax = (1 << countbits) - 1; // valeur max atteinte par le compteur à saturation
   table    = new uint32_t[nentries]();
#endif
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

bool PREDICTOR::GetPrediction(UINT64 PC)
{
#ifdef BIMODAL
   uint32_t v = table[PC & pcmask];
#else
   uint32_t v = table[histval];
#endif
   return (v > (countmax / 2)) ? TAKEN : NOT_TAKEN;
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::UpdatePredictor(UINT64 PC, OpType opType, bool resolveDir, bool predDir, UINT64 branchTarget)
{
#ifdef BIMODAL
   uint32_t v = table[PC & pcmask];
   table[PC & pcmask] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
#else
   uint32_t v = table[histval];
   table[histval] = (resolveDir == TAKEN) ? SatIncrement(v, countmax) : SatDecrement(v);
   histval = ((histval << 1) | resolveDir) & (nentries - 1);
#endif
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

void PREDICTOR::TrackOtherInst(UINT64 PC, OpType opType, bool branchDir, UINT64 branchTarget)
{
   // This function is called for instructions which are not
   // conditional branches, just in case someone decides to design
   // a predictor that uses information from such instructions.
   // We expect most contestants to leave this function untouched.
}

/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////


/***********************************************************/
