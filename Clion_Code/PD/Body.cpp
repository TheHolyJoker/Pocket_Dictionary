//
// Created by tomer on 10/24/19.
//

#include "Body.h"


Body::Body(size_t m, size_t f, size_t l) : fp_size(l), capacity(0), max_capacity(f) {
    if (BODY_BLOCK_SIZE != (8 * sizeof(BODY_BLOCK_TYPE)))
        assert(false);
    size = INTEGER_ROUND((max_capacity * fp_size), (BODY_BLOCK_SIZE));
    B = new BODY_BLOCK_TYPE[size]();
    for (size_t i = 0; i < size; ++i) B[i] = 0;

//    this->vec.resize(max_capacity * fp_size);

}

bool Body::wrap_lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
//    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
    return lookup(abstract_body_start_index, abstract_body_end_index, remainder);
}

void Body::wrap_insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
//    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
    insert(abstract_body_start_index, abstract_body_end_index, remainder);
//    vector_insert(abstract_body_start_index, abstract_body_end_index, remainder);
//    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
}

void Body::wrap_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
//    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
    remove(abstract_body_start_index, abstract_body_end_index, remainder);
//    vector_remove(abstract_body_start_index, abstract_body_end_index, remainder);
//    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);

}

bool Body::lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
//    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
    return find_attempt(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index) == 1;
//    return find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index) == 1;

}

/*
int Body::vector_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder,
                      size_t *p_B_index, size_t *p_bit_index) {

    size_t vector_start_index = abstract_body_start_index * fp_size;
    size_t vector_end_index = abstract_body_end_index * fp_size;
    size_t i = vector_start_index;
    for (; i < vector_end_index; i += fp_size) {
        auto temp = read_FP_from_vector_by_index(i);
        if (temp < remainder)
            continue;
        else if (temp == remainder) {
            *p_B_index = i / BODY_BLOCK_SIZE;
            *p_bit_index = i % BODY_BLOCK_SIZE;
            return 1;
        } else {
            *p_B_index = i / BODY_BLOCK_SIZE;
            *p_bit_index = i % BODY_BLOCK_SIZE;
            return 2;
        }
    }
    assert(i == vector_end_index);
    *p_B_index = vector_end_index / BODY_BLOCK_SIZE;
    *p_bit_index = vector_end_index % BODY_BLOCK_SIZE;
    return 2;

    */
/*if (this->compare_remainder_and_vector(i, remainder)) {
        *p_B_index = i / BODY_BLOCK_SIZE;
        *p_bit_index = i % BODY_BLOCK_SIZE;
        return 1;
    }
    *p_B_index = vector_end_index / BODY_BLOCK_SIZE;
    *p_bit_index = vector_end_index % BODY_BLOCK_SIZE;
    return 2;*/
/*

}
*/


int
Body::find_attempt(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder, size_t *p_B_index,
                   size_t *p_bit_index) {

    if (abstract_body_start_index == abstract_body_end_index) {
        *p_B_index = (abstract_body_start_index * fp_size) / BODY_BLOCK_SIZE;
        *p_bit_index = (abstract_body_start_index * fp_size) % BODY_BLOCK_SIZE;
        if (CASE_PRINT) cout << "a1" << endl;
        return 2;
    }

    size_t total_bit_counter = abstract_body_start_index * fp_size;
    /*
//    size_t b_start_index = (abstract_body_start_index * fp_size) / BODY_BLOCK_SIZE;
//    size_t b_end_index = (abstract_body_end_index * fp_size) / BODY_BLOCK_SIZE;
*/
    size_t comparing_counter = 0, comparing_lim = abstract_body_end_index - abstract_body_start_index;

    for (; comparing_counter < comparing_lim; ++comparing_counter) {
        size_t B_index = total_bit_counter / BODY_BLOCK_SIZE;
        size_t bit_index_inside_slot = total_bit_counter % BODY_BLOCK_SIZE;
        size_t bits_left_to_read_inside_slot = BODY_BLOCK_SIZE - bit_index_inside_slot;

        BODY_BLOCK_TYPE current_cell = B[B_index];
        if (bits_left_to_read_inside_slot > fp_size) {
            ulong shift = bits_left_to_read_inside_slot - fp_size;
            assert(shift < BODY_BLOCK_SIZE);
            BODY_BLOCK_TYPE current_remainder = (current_cell >> (shift)) & MASK(fp_size);
            if (remainder <= current_remainder) {
                if (CASE_PRINT) cout << "a2" << endl;
                return find_helper_attempt(remainder == current_remainder, B_index, bits_left_to_read_inside_slot,
                                           p_B_index,
                                           p_bit_index);
            }
        } else if (bits_left_to_read_inside_slot == fp_size) {
            BODY_BLOCK_TYPE current_remainder = current_cell & MASK(fp_size);
            if (remainder <= current_remainder) {
                if (CASE_PRINT) cout << "a3" << endl;
                return find_helper_attempt(remainder == current_remainder, B_index, bits_left_to_read_inside_slot,
                                           p_B_index,
                                           p_bit_index);
            }
        } else {
            size_t number_of_bits_to_read_from_next_slot = fp_size - bits_left_to_read_inside_slot;
            ulong upper_shift = fp_size - bits_left_to_read_inside_slot;
            assert(upper_shift >= 0 and upper_shift < BODY_BLOCK_SIZE);
            ulong upper = (current_cell & MASK(bits_left_to_read_inside_slot)) << (upper_shift);
            assert(size > B_index + 1);
            ulong lower_shift = BODY_BLOCK_SIZE - number_of_bits_to_read_from_next_slot;
            assert(0 <= lower_shift and lower_shift < BODY_BLOCK_SIZE);
            ulong lower = (B[B_index + 1] >> lower_shift) & MASK(number_of_bits_to_read_from_next_slot);
            BODY_BLOCK_TYPE current_remainder = upper | lower;
            if (remainder <= current_remainder) {
                if (CASE_PRINT) cout << "a4" << endl;
                return find_helper_attempt(remainder == current_remainder, B_index, bits_left_to_read_inside_slot,
                                           p_B_index,
                                           p_bit_index);
            }
        }
        total_bit_counter += fp_size;
    }
    *p_B_index = total_bit_counter / BODY_BLOCK_SIZE;
    *p_bit_index = total_bit_counter % BODY_BLOCK_SIZE;
    if (CASE_PRINT) cout << "a5" << endl;
    return 2;
}

int
Body::find_helper_attempt(bool did_find, size_t current_b_index, size_t bits_left, size_t *p_B_index, size_t *p_bit_index) {
    *p_B_index = current_b_index;
    *p_bit_index = BODY_BLOCK_SIZE - bits_left;
    return 2 - did_find;
    return (did_find) ? 1 : 2;
}

void Body::insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = -1, bit_index = -1;
    auto res = find_attempt(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &bit_index);
//    auto res = find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &bit_index);
//    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);

    /*
    size_t my_bit_index = abstract_body_end_index * fp_size;
//    if (res == 2) {
//        my_bit_index += fp_size;
//    }
    size_t exp_B_index = (my_bit_index) / BODY_BLOCK_SIZE;
    size_t exp_bit_index = (my_bit_index) % BODY_BLOCK_SIZE;
    bool cond = (exp_B_index == B_index) and (exp_bit_index == bit_index);
    if (!cond) {
        printf("exp_B_index: %zu, B_index: %zu\n", exp_B_index, B_index);
        printf("exp_bit_index: %zu, bit_index: %zu\n", exp_bit_index, bit_index);
    }
    assert(cond);
*/
    size_t left_bit_index = BODY_BLOCK_SIZE - bit_index;
    for (size_t i = size - 1; i > B_index; --i) {
        B[i] = (B[i] >> fp_size) | ((B[i - 1] ) << (BODY_BLOCK_SIZE - fp_size));
//        B[i] = (B[i] >> fp_size) | ((B[i - 1] & MASK(fp_size)) << (BODY_BLOCK_SIZE - fp_size));
    }

    if (BODY_BLOCK_SIZE >= fp_size + bit_index) {

        size_t shift = left_bit_index;
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
        /*BODY_BLOCK_TYPE upper = (B[B_index] >> shift) << shift;*/
        BODY_BLOCK_TYPE mid = remainder << (left_bit_index - fp_size);
        BODY_BLOCK_TYPE lower = (B[B_index] >> fp_size) & (MASK(BODY_BLOCK_SIZE - (bit_index + fp_size)));
        B[B_index] = (upper | lower | mid);

//        assert(fp_size - shift <= 0);
    } else { // Prevents negative shifting.
        size_t shift = left_bit_index;
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
//        BODY_BLOCK_TYPE upper = (B[B_index] >> shift) << shift;
        BODY_BLOCK_TYPE lower = remainder >> (fp_size - shift);
        B[B_index] = (upper | lower);

        assert(fp_size - shift > 0);

        //same amount that remainder was shifted right by. (fp_size - shift)
        size_t bits_left = fp_size - shift; // bits_left =  fp_size + bit_index - BODY_BLOCK_SIZE
        upper = (remainder & MASK(bits_left)) << (BODY_BLOCK_SIZE - bits_left); // todo Check this.
        lower = (B[B_index + 1] << bits_left) >> bits_left; // clear lower's upper-bits.
        B[B_index + 1] = (upper | lower);
    }
    capacity++;
}

void Body::remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = -1, bit_index = 0;
    auto res = find_attempt(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &bit_index);
//    validate_find(abstract_body_start_index, abstract_body_end_index, remainder);
    if (res == 2) {
        cout << "Trying to delete element with remainder that is not in the Body." << endl;
        return;
    }

    size_t left_fp_start_index = BODY_BLOCK_SIZE - bit_index;
    capacity--;

    if (B_index == size - 1) {
        assert(bit_index + fp_size <= BODY_BLOCK_SIZE);

        ulong shift = left_fp_start_index;
        assert(shift >= 0);
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
        BODY_BLOCK_TYPE mid = (B[B_index] & MASK(shift - fp_size)) << fp_size;
        assert(shift >= fp_size);
        B[B_index] = (upper | mid);
        return;
    }

    if (BODY_BLOCK_SIZE >= fp_size + bit_index) {
        ulong shift = left_fp_start_index;
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
        BODY_BLOCK_TYPE mid = (B[B_index] & MASK(left_fp_start_index - fp_size)) << fp_size;
        BODY_BLOCK_TYPE lower = (B[B_index + 1]) >> (BODY_BLOCK_SIZE - fp_size);
        B[B_index] = (upper | lower | mid);

        assert(shift >= fp_size);
        assert(BODY_BLOCK_SIZE - fp_size >= 0);

        for (size_t i = B_index + 1; i < size - 1; ++i) {
            B[i] = (B[i] << fp_size) | (B[i + 1] >> (BODY_BLOCK_SIZE - fp_size));
        }
        B[size - 1] <<= fp_size;

        /*
    //        BODY_BLOCK_TYPE mid = remainder << (bit_index - fp_size);

            //        BODY_BLOCK_TYPE lower = (B[B_index] & MASK(shift - fp_size)) << fp_size;
        //
        //        BODY_BLOCK_TYPE lower = (B[B_index] & MASK(shift)) & (MASK(BODY_BLOCK_SIZE - (bit_index + fp_size)));
        //        BODY_BLOCK_TYPE lower = (B[B_index] >> fp_size) & (MASK(BODY_BLOCK_SIZE - (bit_index + fp_size)));
    */
    } else {
        /*cout << "/n$$$$$$$$$$$$$$$" << endl;
        print_array_as_consecutive_memory(B, size);
        cout << "$$$$$$$$$$$$$$$/n" << endl;*/
        ulong shift = left_fp_start_index;
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
        ulong lower_shift = BODY_BLOCK_SIZE - fp_size;
        BODY_BLOCK_TYPE lower = B[B_index + 1] >> lower_shift;
        lower &= MASK(left_fp_start_index);
        B[B_index] = upper | lower;
        /*
        ulong shift = left_fp_start_index;
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
//        BODY_BLOCK_TYPE mid = (B[B_index] & MASK(shift - fp_size)) << fp_size;
        size_t next_slot_bit_index = (bit_index + fp_size) % BODY_BLOCK_SIZE;
        size_t next_slot_bits_left_to_read = BODY_BLOCK_SIZE - next_slot_bit_index;

        size_t extra_shift = fp_size - left_fp_start_index;
        size_t lower_shift = (BODY_BLOCK_SIZE - fp_size + extra_shift);
        BODY_BLOCK_TYPE lower = (B[B_index + 1]) >> (lower_shift);
        *//*BODY_BLOCK_TYPE lower = (lower_shift < BODY_BLOCK_SIZE) ? (B[B_index + 1]) >> (lower_shift) : 0;*//*
//        BODY_BLOCK_TYPE lower = (B[B_index + 1]) >> (BODY_BLOCK_SIZE - fp_size + extra_shift);
        B[B_index] = (upper | lower);// | mid);*/
        /*cout << "/n$$$$$$$$$$$$$$$" << endl;
        print_array_as_consecutive_memory(B, size);
        cout << "$$$$$$$$$$$$$$$/n" << endl;
*/
        for (size_t i = B_index + 1; i < size - 1; ++i) {
            B[i] = (B[i] << fp_size) | (B[i + 1] >> (BODY_BLOCK_SIZE - fp_size));
        }
        B[size - 1] <<= fp_size;

        assert(0 <= shift and shift < fp_size);
        assert(0 <= lower_shift and lower_shift < BODY_BLOCK_SIZE);
        /*      cout << "/n$$$$$$$$$$$$$$$" << endl;
              print_array_as_consecutive_memory(B, size);
              cout << "$$$$$$$$$$$$$$$/n" << endl;*/
    }
}


bool Body::conditional_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = -1, bit_index = 0;
    auto res = find_attempt(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &bit_index);
    if (res == 2) {
        return false;
    }

    size_t left_fp_start_index = BODY_BLOCK_SIZE - bit_index;
    capacity--;

    if (B_index == size - 1) {
        assert(bit_index + fp_size <= BODY_BLOCK_SIZE);

        ulong shift = left_fp_start_index;
        assert(shift >= 0);
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
        BODY_BLOCK_TYPE mid = (B[B_index] & MASK(shift - fp_size)) << fp_size;
        assert(shift >= fp_size);
        B[B_index] = (upper | mid);
        return true;
    }

    if (BODY_BLOCK_SIZE >= fp_size + bit_index) {
        ulong shift = left_fp_start_index;
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
        BODY_BLOCK_TYPE mid = (B[B_index] & MASK(left_fp_start_index - fp_size)) << fp_size;
        BODY_BLOCK_TYPE lower = (B[B_index + 1]) >> (BODY_BLOCK_SIZE - fp_size);
        B[B_index] = (upper | lower | mid);

        assert(shift >= fp_size);
        assert(BODY_BLOCK_SIZE - fp_size >= 0);

        for (size_t i = B_index + 1; i < size - 1; ++i) {
            B[i] = (B[i] << fp_size) | (B[i + 1] >> (BODY_BLOCK_SIZE - fp_size));
        }
        B[size - 1] <<= fp_size;

        /*
    //        BODY_BLOCK_TYPE mid = remainder << (bit_index - fp_size);

            //        BODY_BLOCK_TYPE lower = (B[B_index] & MASK(shift - fp_size)) << fp_size;
        //
        //        BODY_BLOCK_TYPE lower = (B[B_index] & MASK(shift)) & (MASK(BODY_BLOCK_SIZE - (bit_index + fp_size)));
        //        BODY_BLOCK_TYPE lower = (B[B_index] >> fp_size) & (MASK(BODY_BLOCK_SIZE - (bit_index + fp_size)));
    */
    } else {
        /*cout << "/n$$$$$$$$$$$$$$$" << endl;
        print_array_as_consecutive_memory(B, size);
        cout << "$$$$$$$$$$$$$$$/n" << endl;*/
        ulong shift = left_fp_start_index;
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
        ulong lower_shift = BODY_BLOCK_SIZE - fp_size;
        BODY_BLOCK_TYPE lower = B[B_index + 1] >> lower_shift;
        lower &= MASK(left_fp_start_index);
        B[B_index] = upper | lower;
        /*
        ulong shift = left_fp_start_index;
        BODY_BLOCK_TYPE upper = (shift < BODY_BLOCK_SIZE) ? (B[B_index] >> shift) << shift : 0;
//        BODY_BLOCK_TYPE mid = (B[B_index] & MASK(shift - fp_size)) << fp_size;
        size_t next_slot_bit_index = (bit_index + fp_size) % BODY_BLOCK_SIZE;
        size_t next_slot_bits_left_to_read = BODY_BLOCK_SIZE - next_slot_bit_index;

        size_t extra_shift = fp_size - left_fp_start_index;
        size_t lower_shift = (BODY_BLOCK_SIZE - fp_size + extra_shift);
        BODY_BLOCK_TYPE lower = (B[B_index + 1]) >> (lower_shift);
        *//*BODY_BLOCK_TYPE lower = (lower_shift < BODY_BLOCK_SIZE) ? (B[B_index + 1]) >> (lower_shift) : 0;*//*
//        BODY_BLOCK_TYPE lower = (B[B_index + 1]) >> (BODY_BLOCK_SIZE - fp_size + extra_shift);
        B[B_index] = (upper | lower);// | mid);*/
        /*cout << "/n$$$$$$$$$$$$$$$" << endl;
        print_array_as_consecutive_memory(B, size);
        cout << "$$$$$$$$$$$$$$$/n" << endl;
*/
        for (size_t i = B_index + 1; i < size - 1; ++i) {
            B[i] = (B[i] << fp_size) | (B[i + 1] >> (BODY_BLOCK_SIZE - fp_size));
        }
        B[size - 1] <<= fp_size;

        assert(0 <= shift and shift < fp_size);
        assert(0 <= lower_shift and lower_shift < BODY_BLOCK_SIZE);
        /*      cout << "/n$$$$$$$$$$$$$$$" << endl;
              print_array_as_consecutive_memory(B, size);
              cout << "$$$$$$$$$$$$$$$/n" << endl;*/
    }
    return true;
}

/*

bool Body::vector_lookup(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    return vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index) == 1;
}

void Body::vector_insert(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    auto res = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index);
    size_t vector_bit_counter = B_index * BODY_BLOCK_SIZE + p_bit_index;
    vector_push(vector_bit_counter);
    write_FP_to_vector_by_index(vector_bit_counter, remainder);

}

void Body::vector_remove(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
    size_t B_index = 0, p_bit_index = 0;
    auto res = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &B_index, &p_bit_index);
    if (res == 2) {
        cout << "vector - Trying to delete element with remainder that is not in the Body." << endl;
        return;
    }
    size_t vector_bit_counter = B_index * BODY_BLOCK_SIZE + p_bit_index;
    for (size_t i = vector_bit_counter; i < vec.size() - fp_size; ++i) {
        vec[i] = vec[i + fp_size];
    }
    for (size_t i = 0; i < fp_size; ++i)
        vec[vec.size() - fp_size + i] = false;

}
*/


void Body::store_consecutive_remainders(uint32_t *a) {
    size_t start_index = 0, end_index = max_capacity;
    BODY_BLOCK_TYPE mask = MASK(fp_size);
    size_t b_start_index = (start_index * fp_size) / BODY_BLOCK_SIZE;
    size_t b_end_index = (end_index * fp_size) / BODY_BLOCK_SIZE;
    size_t compared_counter = 0, comparing_lim = end_index - start_index;

    assert (comparing_lim <= max_capacity);
    size_t bit_index = (start_index * fp_size) % BODY_BLOCK_SIZE;
    for (size_t i = b_start_index; i <= b_end_index; ++i) {
        size_t bits_left = BODY_BLOCK_SIZE - bit_index;
        BODY_BLOCK_TYPE cell = B[i];
        while ((bits_left > fp_size) and (compared_counter < comparing_lim)) {
            size_t right_shift = bits_left - fp_size;
            // size_t right_shift = BODY_BLOCK_SIZE - (i * fp_size) % BODY_BLOCK_SIZE - fp_size;

            BODY_BLOCK_TYPE slot = (cell >> (right_shift)) & mask;
            a[compared_counter] = slot;
            compared_counter++;

            bits_left -= fp_size;
        }
        if (compared_counter >= comparing_lim)
            return;
        BODY_BLOCK_TYPE slot;
        if (bits_left == fp_size) {
            slot = cell & mask;

            bit_index = 0;
        } else {
            size_t bits_to_take_from_current_cell = bits_left;
            size_t bits_to_take_from_next_cell = fp_size - bits_left;
            assert(bits_to_take_from_next_cell > 0);
            slot = (cell & MASK(bits_to_take_from_current_cell)) << bits_to_take_from_next_cell;
            assert((BODY_BLOCK_SIZE - bits_to_take_from_next_cell) >= 0);
            BODY_BLOCK_TYPE slot_lower = B[i + 1] >> (BODY_BLOCK_SIZE - bits_to_take_from_next_cell);
            slot |= slot_lower;
            assert(slot == (slot & mask));

            bit_index = BODY_BLOCK_SIZE - bits_to_take_from_next_cell;
        }
        a[compared_counter] = slot;
        compared_counter++;

    }
}

void Body::print_consecutive() {
    uint32_t a[max_capacity];
    for (size_t i = 0; i < max_capacity; ++i) {
        a[i] = 0;
    }
    store_consecutive_remainders(a);
    ::print_array(a, max_capacity);
    /*cout << "[" << a[0];
    for (size_t i = 1; i < max_capacity; ++i) {
        cout << ", " << a[i];
    }
    cout << "]" << endl;*/
}

void Body::naive_print() {
    print_array(B, size);
    /*cout << "[" << B[0];
    for (size_t i = 1; i < this->size; ++i) {
        cout << ", " << B[i];
    }
    cout << "]" << endl;*/
}
/*

void Body::print_consecutive_with_vector() {
    print_consecutive();
    print_vector_by_unpacking(&vec, fp_size);
}

void Body::naive_print_with_vector() {
    print_array(B, size);
    print_vector_as_words(&vec);
}

bool Body::compare_remainder_and_vector(size_t bit_start_index, FP_TYPE remainder) {
    assert(bit_start_index + fp_size <= vec.size());
    for (size_t i = 0; i < fp_size; ++i) {
        if (vec[bit_start_index + 1] ^ remainder)
            return false;
        remainder >>= 1ul;
    }
    return true;
}


void Body::validate_find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder) {
//    return;
    size_t a = -1, b = 0, c = -1, d = 0;
    auto r1 = find_att(abstract_body_start_index, abstract_body_end_index, remainder, &a, &b);
    auto r2 = vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &c, &d);

    bool c1 = r1 == r2;
    bool c2 = c1 and (a == c);
    bool c3 = c2 and (b == d);

    if (!c3) {
        cout << "Error start" << endl;

        printf("abst_start_index = %zu, abst_end_index = %zu\n", abstract_body_start_index, abstract_body_end_index);
        printf("actual result(r1) = %d, expected result(r2) = %d\n", r1, r2);
        printf("actual B_index(a)= %zu, expected B_index(c) = %zu\n", a, c);
        printf("actual bit_index(b) = %zu, expected bit_index(d) = %zu\n", b, d);

        cout << "vector print_with no spaces" << endl;
        print_bool_vector_no_spaces(&vec);
        cout << endl;
        print_array_as_consecutive_memory(B, size);
        cout << "Above line is B as consecutive memory" << endl;

        cout << "print vector as words" << endl;
        print_vector_as_words(&vec);
        cout << "naive_print B" << endl;
        naive_print();
        cout << "print vector by unpacking" << endl;
        print_vector_by_unpacking(&vec, fp_size);
        cout << "print B remainder consecutively" << endl;
        print_consecutive();

        cout << "\nPrints end" << endl;

        find_attempt(abstract_body_start_index, abstract_body_end_index, remainder, &a, &b);
        vector_find(abstract_body_start_index, abstract_body_end_index, remainder, &c, &d);
        assert(false);
    }
}

void Body::vector_push(size_t vector_bit_counter) {
    for (size_t i = vec.size() - 1; i >= vector_bit_counter + fp_size; --i)
        vec[i] = vec[i - fp_size];
}

BODY_BLOCK_TYPE Body::read_FP_from_vector_by_index(size_t bit_start_index) {
    return ::read_FP_from_vector_by_index(&vec, bit_start_index, fp_size);
    */
/*assert(bit_start_index + fp_size <= vec.size());
    BODY_BLOCK_TYPE res = vec[bit_start_index];
    for (size_t i = 1; i < fp_size; ++i) {
        res <<= 1ul;
        res |= vec[i];
    }
    return res;*//*

}

void Body::write_FP_to_vector_by_index(size_t index, FP_TYPE remainder) {
    ::write_FP_to_vector_by_index(&vec, index, remainder, fp_size);
    */
/*size_t valid_size = vec.size();
    assert(index + fp_size <= vec.size());
    ulong b = 1ULL << (fp_size - 1);
//    vec[index] = (remainder & b) != 0;
    for (size_t i = 0; i < fp_size; ++i) {
        vec[index + i] = (remainder & b) != 0;
        b >>= 1ul;
    }*//*

}

*/

/*
BODY_BLOCK_TYPE Body::get_mb(size_t qIndex) {
    size_t address = qIndex * (dataSize + MB) + dataSize;
    return uint32_t(get_bits(address, MB));
}*/



int
Body::find(size_t abstract_body_start_index, size_t abstract_body_end_index, FP_TYPE remainder, size_t *p_B_index,
           size_t *p_bit_index) {
    assert (false);
    if (abstract_body_start_index == abstract_body_end_index) {
        *p_B_index = (abstract_body_start_index * fp_size) / BODY_BLOCK_SIZE;
        *p_bit_index = (abstract_body_start_index * fp_size) % BODY_BLOCK_SIZE;
        cout << "h1" << endl;
        return 2;
    }

    BODY_BLOCK_TYPE mask = MASK(fp_size);
    size_t b_start_index = (abstract_body_start_index * fp_size) / BODY_BLOCK_SIZE;
    size_t b_end_index = (abstract_body_end_index * fp_size) / BODY_BLOCK_SIZE;
    size_t compared_counter = 0, comparing_lim = abstract_body_end_index - abstract_body_start_index;


    size_t bit_index = (abstract_body_start_index * fp_size) % BODY_BLOCK_SIZE;
    for (size_t i = b_start_index; i <= b_end_index; ++i) {

        size_t bits_left = BODY_BLOCK_SIZE - bit_index;
        BODY_BLOCK_TYPE cell = B[i];
        while ((bits_left > fp_size) and (compared_counter < comparing_lim)) {
            compared_counter++;
            size_t right_shift = bits_left - fp_size;
            // size_t right_shift = BODY_BLOCK_SIZE - (i * fp_size) % BODY_BLOCK_SIZE - fp_size;
            BODY_BLOCK_TYPE slot;
            if (right_shift == BODY_BLOCK_SIZE)
                slot = 0;
            else
                slot = (cell >> (right_shift)) & mask;

            if (remainder <= slot) {
                cout << "h2";
                return find_helper(remainder == slot, i, bits_left, p_B_index, p_bit_index);
            }
            /*
            if (remainder == slot) {
                *p_B_index = i;
                *p_bit_index = BODY_BLOCK_SIZE - bits_left;
                return 1;
            } else if (remainder > slot) {
                *p_B_index = i;
                *p_bit_index = BODY_BLOCK_SIZE - bits_left;
                return 2;
            }*/

            bits_left -= fp_size;
        }
        if (compared_counter >= comparing_lim) {
            *p_B_index = i;
            *p_bit_index = BODY_BLOCK_SIZE - bits_left;
            cout << "h3" << endl;
            return 2;
        }

        compared_counter++;
        if (bits_left == fp_size) {
            BODY_BLOCK_TYPE slot = cell & mask;
            if (remainder <= slot) {
                cout << "h4";
                return find_helper(remainder == slot, i, bits_left, p_B_index, p_bit_index);
            }
            bit_index = 0;
            /*
                if (remainder == slot) {
                    *p_B_index = i;
                    *p_bit_index = BODY_BLOCK_SIZE - bits_left;
                    return 1;
                } else if (remainder > slot) {
                    *p_B_index = i;
                    *p_bit_index = BODY_BLOCK_SIZE - bits_left;
                    return 2;
                }
    */
        } else {

            size_t bits_to_take_from_current_cell = bits_left;
            size_t bits_to_take_from_next_cell = fp_size - bits_left;
            BODY_BLOCK_TYPE slot = (cell & MASK(bits_to_take_from_current_cell)) << bits_to_take_from_next_cell;
            BODY_BLOCK_TYPE slot_lower = B[i + 1] >> (BODY_BLOCK_SIZE - bits_to_take_from_next_cell);
            slot |= slot_lower;
            assert(slot == (slot & mask));

            if (slot <= remainder) {
                cout << "h5";
                return find_helper(remainder == slot, i, bits_left, p_B_index, p_bit_index);
            }

            bit_index = BODY_BLOCK_SIZE - bits_to_take_from_next_cell;

            /*if (remainder == slot){
                *p_B_index = i;
                *p_bit_index = BODY_BLOCK_SIZE - bits_left;
                return 1;
            }else if (slot < remainder){
                *p_B_index = i;
                *p_bit_index = bits_left;
                return 2;
            }*/
//                return find_helper(remainder == slot, i, bits_left, p_B_index, p_bit_index);
            /*if (remainder == slot) {
                *p_B_index = i;
                *p_bit_index = BODY_BLOCK_SIZE - bits_left;
                return 1;
            } else if (remainder > slot) {
                *p_B_index = i;
                *p_bit_index = BODY_BLOCK_SIZE - bits_left;
                return 2;
            }*/
        }
    }
    assert(false);
}

int Body::find_helper(bool did_find, size_t current_b_index, size_t bits_left, size_t *p_B_index, size_t *p_bit_index) {
    assert (false);
    if (did_find) {
        cout << "\th_h1" << endl;
        *p_B_index = current_b_index;
        *p_bit_index = BODY_BLOCK_SIZE - bits_left;
        return 1;
    }


    if (bits_left >= fp_size) {
        cout << "\th_h2" << endl;
        *p_B_index = current_b_index;
        *p_bit_index = BODY_BLOCK_SIZE - (bits_left + fp_size);
    } else {
        cout << "\th_h3" << endl;
        *p_B_index = current_b_index + 1;
        assert (BODY_BLOCK_SIZE + (bits_left - fp_size) > 0);
        *p_bit_index = fp_size - bits_left;
//        size_t expected_res = (BODY_BLOCK_SIZE - (bits_left + fp_size)) % (BODY_BLOCK_SIZE);
//        assert(*p_bit_index == expected_res);
    }
    return 2;

    /*else {

        *p_B_index = i;
        *p_bit_index = BODY_BLOCK_SIZE - bits_left;
        return 2;
    }*/
}

size_t Body::get_fp_size() const {
    return fp_size;
}

size_t Body::get_size() const {
    return size;
}

uint32_t *Body::get_b() const {
    return B;
}
