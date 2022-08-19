#ifndef MLH
#define MLH

#include <ga/GAAllele.h>
#include <vector>

#define GA_CAST(expr) dynamic_cast<const GARealGenome &>(expr)

namespace Fractal {
  template <typename ItemType>
  inline GAAlleleSet<float>
  __vector_to_alleleset (std::vector<ItemType> v)
  {
    GAAlleleSet<float> alleleset;
    for (ItemType item : v)
      alleleset.add (item);
    return alleleset;
  }
  
  template <typename ItemType>
  inline GAAlleleSet<float>
  __atomic_to_alleleset (ItemType v)
  {
    GAAlleleSet<float> alleleset;
    alleleset.add (v);
    return alleleset;
  }

  inline int*
  __genome_to_array (const GARealGenome &g)
  {
    const int kLength = g.length ();
    int *arr = new int[kLength];
    
    if (!arr)
      {
	printf ("Could not allocate array to clone the genome\n");
	exit (EXIT_FAILURE);
      }
    
    for (int i=0; i<kLength; ++i)
      arr[i] = g.gene (i);
    return arr;
  }
} // namespace Fractal
#endif
