//
// Created by Wilfried Wu on 2024-03-02.
//
#include "a2_matrix_sum.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * Leverage dynamic programming for this problem
 * get sums for the neighbors to the up and left of the current element,
 * and subtract the sum for the up-left neighbor which was already included.
 */

