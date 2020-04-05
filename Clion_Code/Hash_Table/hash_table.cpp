
#include "hash_table.hpp"

template<typename T>
hash_table<T>::hash_table(size_t max_capacity, size_t element_length, size_t bucket_size, double max_load_factor)
        :max_capacity(max_capacity), element_length(element_length), bucket_size(bucket_size),
         max_load_factor(max_load_factor), capacity(0),
         table_size(std::ceil(((double) max_capacity) / max_load_factor)),
         seed1(42), seed2(43), max_cuckoo_insert(0), cuckoo_insert_counter(0), max_capacity_reached(0) {
    table = new T[table_size];
    cout << "table_size is: " << table_size << endl;
    for (int i = 0; i < table_size; ++i) {
        table[i] = EMPTY;
    }
}


template<typename T>
hash_table<T>::hash_table(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity,
                          size_t remainder_length) {
    cout << "Const 3" << endl;
    hash_table(compute_max_capacity(number_of_pd, quotient_range, single_pd_capacity, remainder_length),
               compute_element_length(number_of_pd, quotient_range, single_pd_capacity, remainder_length));

}


template<typename T>
hash_table<T>::~hash_table() {
//    cout << "max_cuckoo_insert " << this->max_cuckoo_insert << endl;
//    cout << "cuckoo_insert_counter " << this->cuckoo_insert_counter << endl;
    delete[] table;
}

template<typename T>
auto hash_table<T>::find(const T x) -> bool {
    assert((x & MASK(element_length)) == x);

    uint32_t b1 = -1, b2 = -1;
    my_hash(x, &b1, &b2);

    return ((find_helper(x, b1)) || find_helper(x, b2));

}

template<typename T>
void hash_table<T>::insert(const T x) {
    assert((x & MASK(element_length)) == x);
    assert(!find(x));
    if (capacity >= max_capacity) {
        cout << "Trying to insert into fully loaded hash table" << endl;
        assert(false);
    }
    capacity++;
    /*http://www.cs.toronto.edu/~noahfleming/CuckooHashing.pdf*/

    max_capacity_reached = (max_capacity_reached >= capacity) ? max_capacity_reached : capacity;
    uint32_t b1 = -1, b2 = -1;
    my_hash(x, &b1, &b2);
    if (insert_if_bucket_not_full(x, b2)) return;
    T hold = x;
    size_t bucket_index = b1;
    cuckoo_insert_counter -= bucket_size;
    for (int i = 0; i < MAX_CUCKOO_LOOP; ++i) {
        max_cuckoo_insert = (max_cuckoo_insert >= i) ? max_cuckoo_insert : i;
        this->cuckoo_insert_counter += bucket_size;
        if (insert_if_bucket_not_full(hold, bucket_index)) return;

        auto rand_table_index = (bucket_index * bucket_size) + (random() % bucket_size);
        auto temp = table[rand_table_index];

        table[rand_table_index] = hold;
        hold = temp;

        uint32_t temp_b1 = -1, temp_b2 = -1;
        my_hash(hold, &temp_b1, &temp_b2);

        assert(temp_b2 != temp_b1);

        if (temp_b1 == bucket_index)
            bucket_index = temp_b2;
        else if (temp_b2 == bucket_index)
            bucket_index = temp_b1;
        else {
            assert(false);
        }

//        bucket_index = (temp_b1 == bucket_index) ? temp_b2 : temp_b2;
    }
    assert(false);

}

template<typename T>
void hash_table<T>::remove(const T x) {
    assert(find(x));
    if (capacity == 0) {
        cout << "Trying to delete from empty hash table" << endl;
        assert(false);
    }
    uint32_t b1 = -1, b2 = -1;
    my_hash(x, &b1, &b2);

    if (remove_helper(x, b1)) {
        assert(!find(x));
        capacity--;
        return;
    }
    assert(remove_helper(x, b2));
    assert(!find(x));

    capacity--;

}


template<typename T>
auto hash_table<T>::find_helper(const T x, size_t bucket_index) -> bool {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_equal(table[table_index + i], x))
            return true;
    }
    return false;
}

template<typename T>
auto hash_table<T>::insert_if_bucket_not_full(T x, size_t bucket_index) -> bool {
    assert((x & MASK(element_length)) == x);
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_empty(table_index + i)) {
            auto prev_val = table[table_index + i];
            table[table_index + i] = x;
            return true;
        }
        /*Add pop attempt*/
    }
    return false;
}

template<typename T>
auto hash_table<T>::remove_helper(const T x, size_t bucket_index) -> bool {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_equal(table[table_index + i], x)) {
            auto prev_val = table[table_index + i];
            table[table_index + i] = DELETED;
            if (find(x)) {
                auto res = find_table_location(x);
                find(x);
                assert(false);
            }

            return true;
        }
    }
    return false;
}

template<typename T>
auto hash_table<T>::find_table_location(T x) -> size_t {
    uint32_t b1 = -1, b2 = -1;
    my_hash(x, &b1, &b2);

    auto res = find_helper_table_location(x, b1);
    if (res != -1)
        return res;
    return find_helper_table_location(x, b2);


}

template<typename T>
auto hash_table<T>::find_helper_table_location(const T x, size_t bucket_index) -> int {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_equal(table[table_index + i], x))
            return table_index + i;
    }
    return -1;
}

template<typename T>
auto hash_table<T>::get_bucket_capacity(size_t bucket_index) -> size_t {
    auto table_index = bucket_index * bucket_size;
    size_t counter = 0;
    for (int i = 0; i < bucket_size; ++i) {
        if (!is_empty(table_index + i))
            counter++;
    }
    return counter;

}

template<typename T>
auto hash_table<T>::is_equal(T with_counter, T without_counter) -> bool {
    T after_mask = without_counter & MASK(element_length);
    assert((without_counter & MASK(element_length)) == without_counter);
    return (with_counter & MASK(element_length)) == without_counter;
}

template<typename T>
auto hash_table<T>::is_empty(size_t table_index) -> bool {
    return (table[table_index] & DELETED);
}

template<typename T>
auto hash_table<T>::is_deleted(size_t table_index) -> bool {
    return table[table_index] == DELETED;
}

template<typename T>
auto hash_table<T>::is_bucket_elements_unique(size_t bucket_index) -> bool {
    auto table_index = bucket_index * bucket_size;
    for (int j = 0; j < bucket_size; ++j) {
        if (is_empty(table_index + j))
            continue;
        T x = table[table_index + j];
        for (int i = j + 1; i < bucket_size; ++i) {
            if (is_empty(table_index + i))
                continue;
            if (is_equal(table[table_index + i], x))
                return false;
        }
    }
    return true;
}


template<typename T>
auto hash_table<T>::move_el_from_bucket_to_other(size_t bucket_index) -> T * {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {

        auto x = table[table_index + i];
        uint32_t b1 = -1, b2 = -1;
        my_hash(x, &b1, &b2);

        size_t other_bucket = (b1 == bucket_index) ? b1 : b2;
        assert((b1 == bucket_index) or (b2 == bucket_index));
        if (insert_if_bucket_not_full(x, other_bucket)) {
            return &(table[table_index + i]);
        }
    }
    return nullptr;
}

template<typename T>
auto hash_table<T>::insert_helper(const T x, size_t bucket_index) {
    auto table_index = bucket_index * bucket_size;
    for (int i = 0; i < bucket_size; ++i) {
        if (is_empty(table_index + i)) {
            table[table_index + i] = x;
            return;
        }
        /*Add pop attempt*/
    }
    /*Here need to use cuckoo hash scheme*/
    cout << "Trying to insert to full bucket" << endl;
    assert(false);
}


template<typename T>
void hash_table<T>::full_buckets_handler(T x, size_t b1, size_t b2) {
//    T *possible_free_slot = move_el_from_bucket_to_other(b1);

}

template<typename T>
void hash_table<T>::insert_without_counter(T x) {
    insert(x | SL(element_length));
}


template<typename T>
void hash_table<T>::pop_attempt(T x) {

}


template<typename T>
auto hash_table<T>::is_state_valid() -> bool {
    return false;
}


template<typename T>
auto hash_table<T>::get_max_cuckoo_insert() const -> size_t {
    return max_cuckoo_insert;
}

template<typename T>
auto hash_table<T>::get_cuckoo_insert_counter() const -> size_t {
    return cuckoo_insert_counter;
}

template<typename T>
auto hash_table<T>::get_max_capacity_reached() const -> size_t {
    return max_capacity_reached;
}

template<typename T>
void hash_table<T>::get_data() {
    cout << "max_cuckoo_insert " << get_max_cuckoo_insert() << endl;
    cout << "cuckoo_insert_counter " << get_cuckoo_insert_counter() << endl;
    cout << "get_max_capacity_reached " << get_max_capacity_reached() << endl;
}


auto
compute_max_capacity(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity,
                     size_t remainder_length) -> size_t {
    auto number_of_over_flow_elements = (size_t) ceil(log2(number_of_pd));
    return number_of_over_flow_elements << 4u;
}

auto compute_element_length(size_t number_of_pd, size_t quotient_range, size_t single_pd_capacity,
                            size_t remainder_length) -> size_t {
    auto a = (size_t) ceil(log2(number_of_pd));
    auto b = (size_t) ceil(log2(quotient_range));
    auto c = (size_t) ceil(log2(remainder_length));
    return a + b + c;
//    return (size_t) ceil(log2(number_of_pd));
}


template
class hash_table<uint32_t>;