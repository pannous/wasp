/*
 * Copyright 2016 WebAssembly Community Group participants
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

#ifndef WABT_BINDING_HASH_H_
#define WABT_BINDING_HASH_H_

#include <functional>
//#include <string>
// #include <vector>
#include "../List.h"
#include "../String.h"
// #include <unordered_map>

#include "common.h"
#include "wasm-link.h"


namespace std {
    // Removed specialization of std::hash<::String>
}

namespace wabt {
    struct Var;

    struct Binding {
        explicit Binding(Index index) : index(index) {
        }

        Binding(const Location &loc, Index index) : loc(loc), index(index) {
        }

        Location loc;
        Index index;
    };

    // This class derives from a C++ container, which is usually not advisable
    // because they don't have virtual destructors. So don't delete a BindingHash
    // object through a pointer to std::unordered_multimap.
    class BindingHash : public List<std::pair<String, Binding> > {
    public:
        using value_type = std::pair<String, Binding>;

        typedef std::function<void(const value_type &, const value_type &)>
        DuplicateCallback;

        void FindDuplicates(DuplicateCallback callback) const;

        Index FindIndex(const Var &) const;

        Index FindIndex(String name) const {
            for (const auto &v: *this) {
                if ((String) v.first == name)
                    return v.second.index;
            }
            return kInvalidIndex;
        }

        bool contains(const String &name) const {
            return FindIndex(name) != kInvalidIndex;
        }

        void insert(const value_type &v) {
            this->push_back(v);
        }

        void emplace(const String &name, const Binding &binding) {
            this->push_back(std::make_pair(name, binding));
        }

        void emplace(const String &name, const Index &index) {
            this->push_back(std::make_pair(name, Binding(index)));
        }

    private:
        typedef List<const value_type *> ValueTypeVector;

        void CreateDuplicatesVector(ValueTypeVector *out_duplicates) const;

        void SortDuplicatesVectorByLocation(ValueTypeVector *duplicates) const;

        void CallCallbacks(const ValueTypeVector &duplicates,
                           DuplicateCallback callback) const;

        // this;
    };
} // namespace wabt

#endif /* WABT_BINDING_HASH_H_ */
