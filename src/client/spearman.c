#include "spearman.h"

#include "../memory.h"

#include <math.h>   // sqrt
#include <stdlib.h> // qsort

/*
 * Code has been stolen and adapted from:
 *
 * https://www.geeksforgeeks.org/dsa/program-spearmans-rank-correlation/
 */

// Structure to store element value and its original index
typedef struct {
  double val;
  size_t index;
} Spearman_Element;

// Comparison function to sort elements by value
static int Spearman_Element_Compare(const void* a, const void* b) {
  Spearman_Element* ea = (Spearman_Element*) a;
  Spearman_Element* eb = (Spearman_Element*) b;
  if (ea->val < eb->val) return -1;
  if (ea->val > eb->val) return 1;
  return 0;
}

// Function to assign ranks (handles average rank for ties)
static void Spearman_ComputeRanks(double arr[], double ranks[], size_t n) {
  Spearman_Element* elems = (Spearman_Element*) Mem_Malloc(n * sizeof(Spearman_Element));
  for (size_t i = 0; i < n; i++) {
    elems[i].val = arr[i];
    elems[i].index = i;
  }

  qsort(elems, n, sizeof(Spearman_Element), Spearman_Element_Compare);

  size_t i = 0;
  while (i < n) {
    size_t j = i;
    // Find all identical elements (ties)
    while (j < n && elems[j].val == elems[i].val) {
      j++;
    }

    // Average rank for ties (1-based indexing)
    double avg_rank = (i + 1 + j) / 2.0; 
    for (size_t k = i; k < j; k++) {
      ranks[elems[k].index] = avg_rank;
    }
    i = j;
  }

  Mem_Free(elems);
}

// Function to calculate Pearson correlation coefficient of two rank arrays
static double pearson_correlation(double x[], double y[], size_t n) {
  double sum_x = 0, sum_y = 0, sum_xy = 0, sum_x2 = 0, sum_y2 = 0;

  for (size_t i = 0; i < n; i++) {
    sum_x += x[i];
    sum_y += y[i];
    sum_xy += x[i] * y[i];
    sum_x2 += x[i] * x[i];
    sum_y2 += y[i] * y[i];
  }

  double numerator = (n * sum_xy) - (sum_x * sum_y);
  double denominator = sqrt((n * sum_x2 - sum_x * sum_x) * (n * sum_y2 - sum_y * sum_y));

  if (denominator == 0) return 0;
  return numerator / denominator;
}

// Main function to calculate Spearman's rank correlation
double spearman_correlation(double x[], double y[], size_t n) {
  double* rank_x = (double*) Mem_Malloc(n * sizeof(double));
  double* rank_y = (double*) Mem_Malloc(n * sizeof(double));

  Spearman_ComputeRanks(x, rank_x, n);
  Spearman_ComputeRanks(y, rank_y, n);

  double rho = pearson_correlation(rank_x, rank_y, n);

  Mem_Free(rank_x);
  Mem_Free(rank_y);

  return rho;
}
