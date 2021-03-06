/* Copyright 2019 Intel Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <malloc.h>
#include <stdint.h>
#include <string.h>

#include "StringArray.h"
#include "WasmExtensions.h"

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// CLASS: StringArray
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
StringArray::StringArray(void)
{
    size_ = 0;
    value_ = NULL;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
StringArray::StringArray(const size_t size)
{
    size_ = 0;
    value_ = NULL;

   (void) resize(size);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
StringArray::StringArray(const uint8_t* buffer, size_t size)
{
    size_ = size;
    value_ = NULL;

    (void) assign(buffer, size);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
StringArray::StringArray(const char* buffer)
{
    size_ = 0;
    value_ = NULL;

    (void) assign(buffer);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copy constructor
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
StringArray::StringArray(const StringArray& value)
{
    size_ = 0;
    value_ = NULL;

    (void) assign(value.value_, value.size_);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
StringArray::~StringArray(void)
{
    clear();
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool StringArray::clear(void)
{
    if (value_ != NULL)
    {
        free(value_);
    }

    size_ = 0;
    value_ = NULL;

    return true;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool StringArray::resize(const size_t size)
{
    // StringArray does not attempt to reuse memory, we clear
    // the allocation and start afresh
    clear();

    // if size is 0, then this is equivalent to a clear meaning
    // that space is deallocated
    if (size == 0)
        return true;

    size_ = size;
    value_ = (uint8_t*)malloc(size_);
    if (value_ == NULL)
        return false;

    memset(value_, 0, size_);
    return true;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool StringArray::assign(const uint8_t* buffer, size_t size)
{
    if (buffer == NULL)
        return false;

    if (size == 0)
        return false;

    if (! resize(size))
        return false;

    memcpy(value_, buffer, size_);
    return true;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool StringArray::assign(const char* buffer)
{
    if (buffer == NULL)
        return false;

    size_t size;
    const char *b;

    for (b = buffer, size = 0; *b; b++, size++) ;

    if (! resize(size + 1))
        return false;

    memcpy(value_, (uint8_t*)buffer, size);
    value_[size] = 0;

    return true;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool StringArray::set(uint8_t v, size_t p)
{
    if (value_ == NULL)
        return false;

    if (p < size_)
    {
        value_[p] = v;
        return true;
    }

    return false;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool StringArray::take(uint8_t* buffer, size_t size)
{
    if (buffer == NULL)
        return false;

    if (size == 0)
        return false;

    clear();

    size_ = size;
    value_ = buffer;

    return true;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool StringArray::equal(const StringArray& sarray) const
{
    if (size_ != sarray.size_)
        return false;

    return (memcmp(value_, sarray.value_, size_) == 0);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
bool StringArray::null_terminated(void) const
{
    if (value_ == NULL || size_ == 0)
        return false;

    return (value_[size_ - 1] == 0);
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
const size_t StringArray::size(void) const
{
    return size_;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
uint8_t* StringArray::data(void)
{
    return value_;
}

// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
const uint8_t* StringArray::c_data(void) const
{
    return value_;
}
