//
// Created by tomer on 11/4/19.
//

#ifndef CLION_CODE_PERMUTATION_H
#define CLION_CODE_PERMUTATION_H

#include <zconf.h>
#include <cstdint>

// P31 = (1<<31) - 1.
#define P31 (2147483647)

/**
 * Primitive roots and their inverse modulo (1<<31) - 1.
 */
static ulong primitive_root_array[5] = {21759941, 84476145, 96692034, 161856978, 168441100};
static ulong inv_primitive_root_array[5] = {2059219856, 1309127614, 1139066240, 1665930690, 1904044902};


class Permutation {
    size_t mod, mult_const, mult_inv_const;
public:
    Permutation(size_t mod);

    uint32_t get_perm(size_t el);

    uint32_t get_perm_inv(size_t el);

};


#endif //CLION_CODE_PERMUTATION_H
