//
// Created by tomer on 11/4/19.
//

#include "pow2c_filter.h"

pow2c_filter::pow2c_filter(size_t size, size_t fp_size, size_t interval_length, size_t single_pd_capacity)
        : size(size), capacity(0), h(P31), perm(P31),
          interval_length(interval_length), single_pd_capacity(single_pd_capacity), fp_size(fp_size) {
    for (size_t i = 0; i < size; ++i) {
        pd_vec.emplace_back(PD(interval_length, single_pd_capacity, fp_size));
    }
    quotient_length = ceil(log2(single_pd_capacity + 1));
    pd_index_length = ceil(log2(size));
}

bool pow2c_filter::lookup(string *s) {
    size_t hash1 = h(s) << 1ul;
    size_t hash2 = (perm.get_perm(h(s)) << 1ul) | 1ul;
//    printf("hash1 is %zu, hash2 is: %zu\n", hash1, hash2);
    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    split(hash1, &pd_index1, &q1, &r1);
    split(hash2, &pd_index2, &q2, &r2);

    return pd_vec[pd_index1].lookup(q1, r1) || pd_vec[pd_index1].lookup(q2, r2);


}

void pow2c_filter::insert(string *s) {
    size_t hash1 = h(s) << 1ul;
    size_t hash2 = (perm.get_perm(h(s)) << 1ul) | 1ul;

    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    split(hash1, &pd_index1, &q1, &r1);
    split(hash2, &pd_index2, &q2, &r2);

    if (pd_vec[pd_index2].get_capacity() <= pd_vec[pd_index1].get_capacity()) {
        pd_vec[pd_index2].insert(q2, r2);
    } else {
        pd_vec[pd_index1].insert(q1, r1);
    }
    capacity++;
}

void pow2c_filter::remove(string *s) {
    size_t hash1 = h(s) << 1ul;
    size_t hash2 = (perm.get_perm(h(s)) << 1ul) | 1ul;

    size_t pd_index1 = -1, pd_index2 = -1;
    D_TYPE q1 = -2, q2 = -2, r1 = -3, r2 = -3;

    split(hash1, &pd_index1, &q1, &r1);
    split(hash2, &pd_index2, &q2, &r2);

    if (pd_vec[pd_index1].conditional_remove(q1, r1))
        return;

    if (not pd_vec[pd_index2].conditional_remove(q2, r2)) {
        cout << "Trying to delete an element that is not in the filter.";
    }
    capacity--;
}

void pow2c_filter::split(size_t hash_index, size_t *pd_index, D_TYPE *quotient, D_TYPE *remainder) {
    *remainder = hash_index & MASK(fp_size);
    hash_index >>= fp_size;
    *quotient = hash_index & MASK(quotient_length);
    hash_index >>= quotient_length;
    *pd_index = hash_index & MASK(pd_index_length);

}
