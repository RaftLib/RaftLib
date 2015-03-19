/* Copyright 2004,2007,2010,2012 IPB, Universite de Bordeaux, INRIA & CNRS
**
** This file is part of the Scotch software package for static mapping,
** graph partitioning and sparse matrix ordering.
**
** This software is governed by the CeCILL-C license under French law
** and abiding by the rules of distribution of free software. You can
** use, modify and/or redistribute the software under the terms of the
** CeCILL-C license as circulated by CEA, CNRS and INRIA at the following
** URL: "http://www.cecill.info".
** 
** As a counterpart to the access to the source code and rights to copy,
** modify and redistribute granted by the license, users are provided
** only with a limited warranty and the software's author, the holder of
** the economic rights, and the successive licensors have only limited
** liability.
** 
** In this respect, the user's attention is drawn to the risks associated
** with loading, using, modifying and/or developing or reproducing the
** software by the user in light of its specific status of free software,
** that may mean that it is complicated to manipulate, and that also
** therefore means that it is reserved for developers and experienced
** professionals having in-depth computer knowledge. Users are therefore
** encouraged to load and test the software's suitability as regards
** their requirements in conditions enabling the security of their
** systems and/or data to be ensured and, more generally, to use and
** operate it in the same conditions as regards security.
** 
** The fact that you are presently reading this means that you have had
** knowledge of the CeCILL-C license and that you accept its terms.
*/
/************************************************************/
/**                                                        **/
/**   NAME       : hgraph_induce_edge.c                    **/
/**                                                        **/
/**   AUTHOR     : Francois PELLEGRINI                     **/
/**                                                        **/
/**   FUNCTION   : This commodity file contains the edge   **/
/**                arrays building subroutine which is     **/
/**                duplicated, with minor modifications,   **/
/**                into hgraph_induce.c                    **/
/**                                                        **/
/**   DATES      : # Version 4.0  : from : 10 jan 2002     **/
/**                                 to     17 jan 2003     **/
/**                # Version 5.0  : from : 19 dec 2006     **/
/**                                 to     19 dec 2006     **/
/**                # Version 5.1  : from : 24 oct 2010     **/
/**                                 to     24 oct 2010     **/
/**                # Version 6.0  : from : 22 mar 2012     **/
/**                                 to     22 mar 2012     **/
/**                                                        **/
/************************************************************/

static
void
HGRAPHINDUCE2NAME (
const Hgraph * restrict const orggrafptr,         /* Pointer to original halo graph        */
Gnum * restrict const         orgindxtax,         /* Array of numbers of selected vertices */
Hgraph * restrict const       indgrafptr)         /* Pointer to induced halo graph         */
{
  Gnum                indvertnum;                 /* Number of current induced vertex                */
  Gnum                indvertnnd;                 /* Number of after-last induced (halo) vertex      */
  Gnum                indvelosum;                 /* Overall induced vertex load                     */
  Gnum                indedgenum;                 /* Number of current edge in induced halo subgraph */
  Gnum                indenohnbr;                 /* Number of non-halo edges in halo subgraph       */
  Gnum                inddegrmax;                 /* Maximum degree                                  */
#ifdef SCOTCH_DEBUG_HGRAPH2
  Gnum                indedgenbs;                 /* Revised number of edges in halo subgraph        */
#endif /* SCOTCH_DEBUG_HGRAPH2 */
#ifdef HGRAPHINDUCE2L                             /* If edge loads present */
  Gnum                indedlosum;
  Gnum                indenohsum;

  indedlosum =
  indenohsum = 0;
#endif /* HGRAPHINDUCE2L */
  inddegrmax = 0;
  for (indvertnum = indedgenum = indgrafptr->s.baseval, indvelosum = indenohnbr = 0, indvertnnd = indgrafptr->vnohnnd; /* For all non-halo vertices */
       indvertnum < indgrafptr->vnohnnd; indvertnum ++) {
    Gnum                orgvertnum;               /* Number of current vertex in original halo graph       */
    Gnum                orgedgenum;               /* Number of current edge in original halo graph         */
    Gnum                indedgennd;               /* Index of after-last edge position in edge array       */
    Gnum                indedhdnum;               /* Index of after-last edge linking to non-halo vertices */
    Gnum                inddegrval;

    orgvertnum = indgrafptr->s.vnumtax[indvertnum];
    indgrafptr->s.verttax[indvertnum] = indedgenum;
    indenohnbr -= indedgenum;                     /* Subtract base of non-halo edges */
    if (indgrafptr->s.velotax != NULL) {          /* If graph has vertex weights     */
      indvelosum +=                               /* Accumulate vertex loads         */
      indgrafptr->s.velotax[indvertnum] = orggrafptr->s.velotax[orgvertnum];
    }

    inddegrval = orggrafptr->s.vendtax[orgvertnum] - orggrafptr->s.verttax[orgvertnum]; /* Get degree of non-halo node */
    if (inddegrmax < inddegrval)                  /* Keep maximum degree */
      inddegrmax = inddegrval;

    for (orgedgenum = orggrafptr->s.verttax[orgvertnum], indedhdnum = indedgennd = indedgenum + inddegrval;
         orgedgenum < orggrafptr->s.vendtax[orgvertnum]; orgedgenum ++) {
      Gnum                orgvertend;             /* Number of current end vertex in original halo graph   */
      Gnum                indvertend;             /* Number of current end vertex in induced halo subgraph */

      orgvertend = orggrafptr->s.edgetax[orgedgenum];
      indvertend = orgindxtax[orgvertend];
      if (indvertend == ~0) {                     /* If neighbor is yet undeclared halo vertex */
        indgrafptr->s.vnumtax[indvertnnd] = orgvertend; /* Add number of halo vertex to array  */
        indvertend = orgindxtax[orgvertend] = indvertnnd ++; /* Get induced number of vertex   */
      }
      if (indvertend >= indgrafptr->vnohnnd) {    /* If neighbor is halo vertex            */
        indedhdnum --;                            /* Add neighbor at end of edge sub-array */
        indgrafptr->s.edgetax[indedhdnum] = indvertend;
        HGRAPHINDUCE2EDLOINIT (indedhdnum);
      }
      else {                                      /* If heighbor is non-halo vertex                    */
        indgrafptr->s.edgetax[indedgenum] = indvertend; /* Add neighbor at beginning of edge sub-array */
        HGRAPHINDUCE2EDLOINIT (indedgenum);
        HGRAPHINDUCE2ENOHINIT;
        indedgenum ++;
      }
    }
#ifdef SCOTCH_DEBUG_HGRAPH2
    if (indedgenum != indedhdnum) {
      errorPrint (STRINGIFY (HGRAPHINDUCE2NAME) ": internal error (1)");
      return;
    }
#endif /* SCOTCH_DEBUG_HGRAPH2 */
    indenohnbr += indedhdnum;                     /* Add position to number of non-halo edges */
    indgrafptr->vnhdtax[indvertnum] = indedhdnum; /* Set end of non-halo sub-array            */
    indedgenum = indedgennd;                      /* Point to next free space in edge array   */
  }
  indgrafptr->vnlosum = (indgrafptr->s.velotax != NULL) ? indvelosum : indgrafptr->vnohnbr;
  indgrafptr->enohnbr = indenohnbr;

#ifdef SCOTCH_DEBUG_HGRAPH2
  indedgenbs = 2 * (indedgenum - indgrafptr->s.baseval) - indenohnbr; /* Compute total number of edges */
#endif /* SCOTCH_DEBUG_HGRAPH2 */
#ifdef HGRAPHINDUCE2L                             /* If edge loads present */
  {
    Gnum *              indedgetab;               /* Dummy area to recieve un-based edgetab */
    Gnum *              indedlotab;               /* Save of old position of edgetab array  */
#ifndef SCOTCH_DEBUG_HGRAPH2
    Gnum                indedgenbs;               /* Revised number of edges in halo subgraph */

    indedgenbs = 2 * (indedgenum - indgrafptr->s.baseval) - indenohnbr; /* Compute total number of edges */
#endif /* SCOTCH_DEBUG_HGRAPH2 */

    indedlotab = indgrafptr->s.edlotax + indgrafptr->s.baseval; /* Save old offset of move area */
    memOffset (indgrafptr->s.edgetax + indgrafptr->s.baseval, /* Compute new offsets            */
               &indedgetab,            (size_t) (indedgenbs * sizeof (Gnum)),
               &indgrafptr->s.edlotax, (size_t) (indedgenbs * sizeof (Gnum)), NULL);
    memMov (indgrafptr->s.edlotax, indedlotab, (indedgenum - indgrafptr->s.baseval) * sizeof (Gnum)); /* Move already existing edge load array */
    indgrafptr->s.edlotax -= indgrafptr->s.baseval;
  }
#endif /* HGRAPHINDUCE2L */

  for ( ; indvertnum < indvertnnd; indvertnum ++) { /* For all halo vertices found during first pass */
    Gnum                orgvertnum;               /* Number of current vertex in original halo graph */
    Gnum                orgedgenum;               /* Number of current edge in original halo graph   */

    orgvertnum = indgrafptr->s.vnumtax[indvertnum];
    indgrafptr->s.verttax[indvertnum] = indedgenum;
    if (indgrafptr->s.velotax != NULL) {          /* If graph has vertex weights */
      indvelosum +=                               /* Accumulate vertex loads     */
      indgrafptr->s.velotax[indvertnum] = orggrafptr->s.velotax[orgvertnum];
    }

    for (orgedgenum = orggrafptr->s.verttax[orgvertnum];
         orgedgenum < orggrafptr->s.vendtax[orgvertnum]; orgedgenum ++) {
      Gnum                orgvertend;             /* Number of current end vertex in original halo graph   */
      Gnum                indvertend;             /* Number of current end vertex in induced halo subgraph */

      orgvertend = orggrafptr->s.edgetax[orgedgenum];
      indvertend = orgindxtax[orgvertend];
      if ((indvertend != ~0) &&                   /* If end vertex in induced halo subgraph */
          (indvertend < indgrafptr->vnohnnd)) {   /* And in its non-halo part only          */
        indgrafptr->s.edgetax[indedgenum] = indvertend;
        HGRAPHINDUCE2EDLOINIT (indedgenum);
        indedgenum ++;
      }
    }
    if (inddegrmax < (indedgenum - indgrafptr->s.verttax[indvertnum]))
      inddegrmax = (indedgenum - indgrafptr->s.verttax[indvertnum]);
  }
#ifdef SCOTCH_DEBUG_HGRAPH2
  if ((indedgenum - indgrafptr->s.baseval) != indedgenbs) {
    errorPrint (STRINGIFY (HGRAPHINDUCE2NAME) ": internal error (2)");
    return;
  }
#endif /* SCOTCH_DEBUG_HGRAPH2 */
  indgrafptr->s.verttax[indvertnnd] = indedgenum; /* Set end of compact vertex array */
  indgrafptr->s.vertnbr = indvertnnd - indgrafptr->s.baseval;
  indgrafptr->s.vertnnd = indvertnnd;
  indgrafptr->s.velosum = (indgrafptr->s.velotax != NULL) ? indvelosum : indgrafptr->s.vertnbr;
  indgrafptr->s.edgenbr = indedgenum - indgrafptr->s.baseval; /* Set actual number of edges */
  indgrafptr->s.edlosum = HGRAPHINDUCE2EDLOSUM;
  indgrafptr->s.degrmax = inddegrmax;
  indgrafptr->enohsum   = HGRAPHINDUCE2ENOHSUM;
}
