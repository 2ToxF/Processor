#include <limits.h>
#include <stdint.h>
#include <sys\stat.h"

#include "utils.h"

static const char NULL_TARGET = '\0';

/// @brief Maximum number of retries while looking for random number
static const uint64_t MAX_RAND_RETRIES = 10;

/// @brief Minimum random number
static const uint64_t MIN_RAND = 100000000000;

/// @brief Maximum random number
static const uint64_t MAX_RAND = ULONG_LONG_MAX;

/// @brief Number of bits temporary hash should be shifted by
static const int           HASH_SHIFT_COEF = 5;

/// @brief Start value of temporary hash
static const unsigned long START_HASH      = 5381;


void SeparateBigString(char* buffer, int buffer_length)
{
    int read_strings_number = 0;
    char* const max_buffer_adress = buffer + buffer_length - 1;
    char* last_str_ptr = buffer;
    char* new_str_ptr = buffer;

    while(true)
    {
        new_str_ptr = strchr(last_str_ptr, NULL_TARGET) + 1;

        buffer[read_strings_number] = {last_str_ptr, (int) (new_str_ptr - last_str_ptr)};

        last_str_ptr = new_str_ptr;
        read_strings_number++;

        if (new_str_ptr == max_buffer_adress)
            break;
    }
}


CodeError fsize(const char* file_name, int* input_buffer_length)
{
    struct stat input_file_stat = {};
    if (stat(file_name, &input_file_stat) != 0)
        return FILLING_FSTAT_ERR;

    *input_buffer_length = input_file_stat.st_size + 1;
    return NO_ERROR;
}


uint64_t MyGetRandom64()
{
    uint64_t random_number = 0;

    for (uint64_t i = 0; i < MAX_RAND_RETRIES; i++)
        if (__builtin_ia32_rdrand64_step(&random_number))
        {
            random_number = random_number/ULONG_MAX*(MAX_RAND - MIN_RAND) + MIN_RAND;
            return random_number;
        }

    return 0;
}

//----------------------------------------------------------------------------------------------------------------------

unsigned long MyHash(void* ptr, int number_of_bytes)
{
    char* char_ptr = (char*) ptr;
    unsigned long calc_hash = START_HASH;

    for (int i = 0; i < number_of_bytes; i++)
    {
        calc_hash = ((calc_hash << HASH_SHIFT_COEF) + calc_hash) + *char_ptr;
        ++char_ptr;
    }

    return calc_hash;
}
