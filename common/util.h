/*
 *  nextpnr -- Next Generation Place and Route
 *
 *  Copyright (C) 2018  David Shah <david@symbioticeda.com>
 *
 *  Permission to use, copy, modify, and/or distribute this software for any
 *  purpose with or without fee is hereby granted, provided that the above
 *  copyright notice and this permission notice appear in all copies.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 *  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 *  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 *  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 *  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 *  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 *  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 */

#ifndef UTIL_H
#define UTIL_H

#include <map>
#include <set>
#include <string>
#include "nextpnr.h"

NEXTPNR_NAMESPACE_BEGIN

// Get a value from a map-style container, returning default if value is not
// found
template <typename Container, typename KeyType, typename ValueType>
ValueType get_or_default(const Container &ct, const KeyType &key, ValueType def = ValueType())
{
    auto found = ct.find(key);
    if (found == ct.end())
        return def;
    else
        return found->second;
};

// Get a value from a map-style container, returning default if value is not
// found (forces string)
template <typename Container, typename KeyType>
std::string str_or_default(const Container &ct, const KeyType &key, std::string def = "")
{
    auto found = ct.find(key);
    if (found == ct.end())
        return def;
    else
        return found->second;
};

template <typename KeyType>
std::string str_or_default(const std::unordered_map<KeyType, Property> &ct, const KeyType &key, std::string def = "")
{
    auto found = ct.find(key);
    if (found == ct.end())
        return def;
    const Property &prop = found->second;
    if (prop.is_string)
        return prop.as_string();
    // A string-valued parameter that arrived as bits: a VHDL boolean generic
    // (GHDL writes FALSE/TRUE as one bit), a string some frontends emit as
    // its ASCII bits, or an integer for a string-typed parameter (BUFR_DIVIDE).
    if (!prop.is_fully_def())
        return def;
    if (prop.size() == 1)
        return prop.as_bool() ? "TRUE" : "FALSE";
    if (prop.size() % 8 == 0) {
        std::string s;
        bool printable = true;
        for (int i = int(prop.size()) - 8; i >= 0; i -= 8) {
            int c = 0;
            for (int j = 7; j >= 0; j--)
                c = (c << 1) | (prop.str.at(i + j) == Property::S1 ? 1 : 0);
            if (c == 0 && s.empty())
                continue; // leading NUL padding
            if (c < 0x20 || c > 0x7e) {
                printable = false;
                break;
            }
            s += char(c);
        }
        if (printable && !s.empty())
            return s;
    }
    return std::to_string(prop.as_int64());
};

// Get a value from a map-style container, converting to int, and returning
// default if value is not found
template <typename Container, typename KeyType> int int_or_default(const Container &ct, const KeyType &key, int def = 0)
{
    auto found = ct.find(key);
    if (found == ct.end())
        return def;
    else
        return std::stoi(found->second);
};

template <typename KeyType>
int int_or_default(const std::unordered_map<KeyType, Property> &ct, const KeyType &key, int def = 0)
{
    auto found = ct.find(key);
    if (found == ct.end())
        return def;
    else {
        if (found->second.is_string)
            return std::stoi(found->second.as_string());
        else
            return found->second.as_int64();
    }
};

// As above, but convert to bool
template <typename Container, typename KeyType>
bool bool_or_default(const Container &ct, const KeyType &key, bool def = false)
{
    return bool(int_or_default(ct, key, int(def)));
};

// Wrap an unordered_map, and allow it to be iterated over sorted by key
template <typename K, typename V> std::map<K, V *> sorted(const std::unordered_map<K, std::unique_ptr<V>> &orig)
{
    std::map<K, V *> retVal;
    for (auto &item : orig)
        retVal.emplace(std::make_pair(item.first, item.second.get()));
    return retVal;
};

// Wrap an unordered_set, and allow it to be iterated over sorted by key
template <typename K> std::set<K> sorted(const std::unordered_set<K> &orig)
{
    std::set<K> retVal;
    for (auto &item : orig)
        retVal.insert(item);
    return retVal;
};

// Return a net if port exists, or nullptr
inline const NetInfo *get_net_or_empty(const CellInfo *cell, const IdString port)
{
    auto found = cell->ports.find(port);
    if (found != cell->ports.end())
        return found->second.net;
    else
        return nullptr;
}

inline NetInfo *get_net_or_empty(CellInfo *cell, const IdString port)
{
    auto found = cell->ports.find(port);
    if (found != cell->ports.end())
        return found->second.net;
    else
        return nullptr;
}

NEXTPNR_NAMESPACE_END

#endif
