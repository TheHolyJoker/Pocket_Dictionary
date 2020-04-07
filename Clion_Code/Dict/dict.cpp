#include "dict.hpp"

/*template<class D, class S>
dict<D, S>::dict(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity, size_t remainder_length,
                 size_t bucket_size, double max_load_factor)
        :number_of_pd(number_of_pd),
         capacity(0),
         quotient_range(quotient_range),
         single_pd_capacity(single_pd_capacity),
         remainder_length(remainder_length),
         pd_index_length(ceil(log2(number_of_pd))),
         quotient_length(ceil(log2(single_pd_capacity + 1))),
         sparse_element_length(remainder_length + pd_index_length + quotient_length),
         sparse_counter_length(sizeof(D_TYPE) * CHAR_BIT - sparse_element_length) {

    assert(sparse_element_length <= sizeof(D_TYPE) * CHAR_BIT);

    //todo: this constructor is wrong.
    spare = new S(number_of_pd, quotient_range, single_pd_capacity, remainder_length);
    pd_capacity_vec.resize(number_of_pd);

    for (size_t i = 0; i < number_of_pd; ++i) {
        pd_vec.emplace_back(D(quotient_range, single_pd_capacity, remainder_length));
    }
    get_info();
}*/


template<class D, class S>
dict<D, S>::dict(size_t max_number_of_elements, size_t error_power_inv, double level1_load_factor,
                 double level2_load_factor):
        capacity(0),
        single_pd_capacity(DEFAULT_PD_CAPACITY),
        number_of_pd(my_ceil(max_number_of_elements, (size_t) DEFAULT_PD_CAPACITY)),
        quotient_range(DEFAULT_QUOTIENT_RANGE),
        remainder_length(error_power_inv),
        pd_index_length(ceil(log2(my_ceil(max_number_of_elements, (size_t) DEFAULT_PD_CAPACITY)))),
        quotient_length(DEFAULT_QUOTIENT_LENGTH),
        sparse_element_length(remainder_length + pd_index_length + quotient_length),
        sparse_counter_length(sizeof(D_TYPE) * CHAR_BIT - sparse_element_length) {

    assert(sparse_element_length <= sizeof(D_TYPE) * CHAR_BIT);

//    todo: this constructor is wrong?.

    size_t spare_max_capacity = get_spare_max_capacity(max_number_of_elements, level1_load_factor);
    spare = new S(spare_max_capacity, sparse_element_length, level2_load_factor);

    pd_capacity_vec.resize(number_of_pd);

    for (size_t i = 0; i < number_of_pd; ++i) {
        pd_vec.emplace_back(D(quotient_range, single_pd_capacity, remainder_length));
    }
    get_info();
}

/*

template<class D, class S>
dict<D, S>::dict(size_t filter_max_capacity, size_t error_power_inv, double level1_load_factor,
                 double level2_load_factor)
        :dict(filter_max_capacity / floor(level1_load_factor * 64), 1u << 6u, 1u << 6u, error_power_inv - 6u,
              DEFAULT_BUCKET_SIZE, level2_load_factor) {
    cout << "D2" << endl;
//    size_t temp_quotient_range = 64u;
//    size_t temp_single_pd_capacity = 64u;
//    assert(error_power_inv > 6);
//    size_t temp_remainder_length = error_power_inv - 6;
//    size_t expected_number_of_elements_in_each_PD =
//    size_t temp_number_of_pd = filter_max_capacity / expected_number_of_elements_in_each_PD;
//    size_t temp_number_of_pd = filter_max_capacity / floor(level1_load_factor * 64);


}
*/


template<class D, class S>
auto dict<D, S>::lookup(const string *s) -> bool {
    auto hash_val = wrap_hash(s);

    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);
    if (pd_vec[pd_index].lookup(quot, r)) return true;

    return spare->find(hash_val % SL(sparse_element_length));
}

template<class D, class S>
void dict<D, S>::insert(const string *s) {
    auto hash_val = wrap_hash(s);

    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);
    if (pd_capacity_vec[pd_index] == single_pd_capacity) {
        assert(pd_vec[pd_index].is_full());
        insert_to_spare_with_pop(hash_val % SL(sparse_element_length));
//        spare->insert(hash_val % SL(sparse_element_length));
        return;
    }
    pd_vec[pd_index].insert(quot, r);
    ++(pd_capacity_vec[pd_index]);

}


template<class D, class S>
void dict<D, S>::insert_to_spare_without_pop(S_TYPE hash_val) {
    spare->insert(hash_val % SL(sparse_element_length));
}


template<class D, class S>
void dict<D, S>::insert_to_spare_with_pop(S_TYPE hash_val) {
    uint32_t b1 = -1, b2 = -1;
    spare->my_hash(hash_val, &b1, &b2);

    if (pop_attempt_with_insertion_by_bucket(hash_val, b2))
        return;

    S_TYPE hold = hash_val;
    size_t bucket = b1;
    for (size_t i = 0; i < MAX_CUCKOO_LOOP; ++i) {
        if (pop_attempt_with_insertion_by_bucket(hold, bucket)) {
            spare->update_max_cuckoo_insert(i);
            spare->update_cuckoo_insert_counter(i);
            return;
        }
        assert(spare->is_bucket_full_by_index(bucket));
        spare->cuckoo_swap(&hold, &bucket);
    }
}

template<class D, class S>
void dict<D, S>::remove(const string *s) {
    auto hash_val = wrap_hash(s);

    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(hash_val, &pd_index, &quot, &r);
    bool BPC = (hash_val == 80735794) or (*s == "]AHWVF]NK[X");
    if (pd_vec[pd_index].conditional_remove(quot, r)) {
        --(pd_capacity_vec[pd_index]);
        return;
    }
    spare->remove(hash_val);
}

/*
template<class D, class S>
auto dict<D, S>::pop_attempt(string *s) -> S_TYPE * {
    auto hash_val = wrap_hash(s);

//    size_t pd_index = -1;
//    uint32_t quot = -1, r = -1;
//    split(hash_val, &pd_index, &quot, &r);
    S_TYPE b1 = -1, b2 = -1;
    spare->my_hash(hash_val % SL(sparse_element_length), &b1, &b2);
    auto res1 = pop_attempt_by_bucket(b1);
    auto res2 = pop_attempt_by_bucket(b2);
    return (res2 != nullptr) ? res2 : res1;

}*/

template<class D, class S>
auto dict<D, S>::pop_attempt_with_insertion_by_bucket(S_TYPE hash_val, size_t bucket_index) -> bool {
    /*//Todo: try to avoid using pointers?.
//    S_TYPE *pointer = spare->get_bucket_address(bucket_index);

//    S_TYPE * res = nullptr;*/
    for (int i = 0; i < spare->get_bucket_size(); ++i) {
        /*bool is_empty1 = spare->is_empty_by_slot(*pointer);
        bool is_empty2 = spare->is_empty_by_bucket_index_and_location(bucket_index, i);
//        assert(is_empty1 == is_empty2);*/
        if (spare->is_empty_by_bucket_index_and_location(bucket_index, i)) {
            spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
//            spare->insert_by_slot_pointer(hash_val, pointer);
            return true;
        }
        if (single_pop_attempt(spare->get_element_without_counter_by_bucket_index_and_location(bucket_index, i))) {
            spare->insert_by_bucket_index_and_location(hash_val, bucket_index, i);
            return true;
        }
//        pointer++;
    }
    return false;
}

/*
template<class D, class S>
auto dict<D, S>::pop_attempt_by_bucket(size_t bucket_index) -> S_TYPE * {
    auto pointer = spare->get_bucket_address(bucket_index);
    S_TYPE *res = nullptr;
    for (int i = 0; i < spare->get_bucket_size(); ++i) {
        if (single_pop_attempt(spare->get_element_without_counter_by_index(*pointer))) {
            *pointer = DELETED;
            res = pointer;
            assert(*res == *pointer);
        }
        pointer++;
    }
    return res;
}*/

template<class D, class S>
auto dict<D, S>::single_pop_attempt(uint32_t element) -> bool {
    size_t pd_index = -1;
    uint32_t quot = -1, r = -1;
    split(element, &pd_index, &quot, &r);
    if (pd_capacity_vec[pd_index] != single_pd_capacity) {
        assert(!pd_vec[pd_index].is_full());

        pd_vec[pd_index].insert(quot, r);
        ++(pd_capacity_vec[pd_index]);
        spare->decrease_capacity();
        return true;
    }
    assert(pd_vec[pd_index].is_full());
    return false;
}

template<class D, class S>
void dict<D, S>::get_info() {

    const size_t num = 9;
    size_t val[num] = {number_of_pd, capacity, quotient_range, single_pd_capacity, remainder_length, pd_index_length,
                       quotient_length, sparse_element_length, sparse_counter_length};

    string names[num] = {"number_of_pd", "capacity", "quotient_range", "single_pd_capacity", "remainder_length",
                         "pd_index_length", "quotient_length", "sparse_element_length", "sparse_counter_length"};
    for (int i = 0; i < num; ++i) {
        cout << names[i] << ": " << val[i] << endl;
    }
}


//template<class D, class S>
//dict<D, S>::~dict() {
//
//}



auto get_max_elements_in_level1(size_t number_of_pd, size_t single_pd_capacity, double level1_load_factor) -> size_t {
    auto single_PD_expected_max_capacity = (size_t) (single_pd_capacity * level1_load_factor);
    return number_of_pd * single_PD_expected_max_capacity;
}


auto get_max_elements_in_level2(size_t number_of_pd, size_t single_pd_capacity, double level1_load_factor,
                                double level2_load_factor) -> size_t {
//    auto;
}

static auto get_spare_max_capacity(size_t dict_max_capacity, double level1_load_factor) -> size_t {
    size_t log2_size = ceil(log2(dict_max_capacity));
//    auto res = my_ceil(dict_max_capacity, log2_size * log2_size * log2_size);
    auto res = my_ceil(dict_max_capacity, log2_size);
    return res;
//    return my_ceil(dict_max_capacity, log2_size * log2_size * log2_size);
}


template
class dict<cg_PD, hash_table<uint32_t>>;
