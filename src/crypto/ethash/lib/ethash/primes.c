// Copyright (c) 2021-2022 The Raven Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <crypto/ethash/include/ethash/ethash.h>

/** Checks if the number is prime. Requires the number to be > 2 and odd. */
static int is_odd_prime(int number)
{
    int d;

    /* Check factors up to sqrt(number).
       To avoid computing sqrt, compare d*d <= number with 64-bit precision. */
    for (d = 3; (int64_t)d * (int64_t)d <= (int64_t)number; d += 2)
    {
        if (number % d == 0)
            return 0;
    }

    return 1;
}

int ethash_find_largest_prime(int upper_bound)
{
    int n = upper_bound;

    if (n < 2)
        return 0;

    if (n == 2)
        return 2;

    /* If even number, skip it. */
    if (n % 2 == 0)
        --n;

    /* Test descending odd numbers. */
    while (!is_odd_prime(n))
        n -= 2;

    return n;
}
