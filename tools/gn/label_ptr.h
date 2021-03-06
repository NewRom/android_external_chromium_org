// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TOOLS_GN_LABEL_PTR_H_
#define TOOLS_GN_LABEL_PTR_H_

#include <functional>

#include "tools/gn/label.h"

class Config;
class ParseNode;
class Target;

// Structure that holds a labeled "thing". This is used for various places
// where we need to store lists of targets or configs. We sometimes populate
// the pointers on another thread from where we compute the labels, so this
// structure lets us save them separately. This also allows us to store the
// location of the thing that added this dependency.
template<typename T>
struct LabelPtrPair {
  typedef T DestType;

  LabelPtrPair() : label(), ptr(NULL), origin(NULL) {}

  explicit LabelPtrPair(const Label& l) : label(l), ptr(NULL), origin(NULL) {
  }

  // This contructor is typically used in unit tests, it extracts the label
  // automatically from a given pointer.
  explicit LabelPtrPair(const T* p) : label(p->label()), ptr(p), origin(NULL) {
  }

  ~LabelPtrPair() {}

  Label label;
  const T* ptr;  // May be NULL.

  // The origin of this dependency. This will be null for internally generated
  // dependencies. This happens when a group is automatically expanded and that
  // group's members are added to the target that depends on that group.
  const ParseNode* origin;
};

typedef LabelPtrPair<Config> LabelConfigPair;
typedef LabelPtrPair<Target> LabelTargetPair;

typedef std::vector<LabelConfigPair> LabelConfigVector;
typedef std::vector<LabelTargetPair> LabelTargetVector;

// Comparison and search functions ---------------------------------------------

// To do a brute-force search by label:
// std::find_if(vect.begin(), vect.end(), LabelPtrLabelEquals<Config>(label));
template<typename T>
struct LabelPtrLabelEquals : public std::unary_function<Label, bool> {
  explicit LabelPtrLabelEquals(const Label& l) : label(l) {}

  bool operator()(const LabelPtrPair<T>& arg) const {
    return arg.label == label;
  }

  const Label& label;
};

// To do a brute-force search by object pointer:
// std::find_if(vect.begin(), vect.end(), LabelPtrPtrEquals<Config>(config));
template<typename T>
struct LabelPtrPtrEquals : public std::unary_function<T, bool> {
  explicit LabelPtrPtrEquals(const T* p) : ptr(p) {}

  bool operator()(const LabelPtrPair<T>& arg) const {
    return arg.ptr == ptr;
  }

  const T* ptr;
};

// To sort by label:
// std::sort(vect.begin(), vect.end(), LabelPtrLabelLess<Config>());
template<typename T>
struct LabelPtrLabelLess : public std::binary_function<LabelPtrPair<T>,
                                                       LabelPtrPair<T>,
                                                       bool> {
  bool operator()(const LabelPtrPair<T>& a, const LabelPtrPair<T>& b) const {
    return a.label < b.label;
  }
};

// Default comparison operators -----------------------------------------------
//
// The default hash and comparison operators operate on the label, which should
// always be valid, whereas the pointer is sometimes null.

template<typename T> inline bool operator==(const LabelPtrPair<T>& a,
                                            const LabelPtrPair<T>& b) {
  return a.label == b.label;
}

template<typename T> inline bool operator<(const LabelPtrPair<T>& a,
                                           const LabelPtrPair<T>& b) {
  return a.label < b.label;
}

namespace BASE_HASH_NAMESPACE {

#if defined(COMPILER_GCC)
template<typename T> struct hash< LabelPtrPair<T> > {
  std::size_t operator()(const LabelPtrPair<T>& v) const {
    BASE_HASH_NAMESPACE::hash<Label> h;
    return h(v.label);
  }
};
#elif defined(COMPILER_MSVC)
template<typename T>
inline size_t hash_value(const LabelPtrPair<T>& v) {
  return BASE_HASH_NAMESPACE::hash_value(v.label);
}
#endif  // COMPILER...

}  // namespace BASE_HASH_NAMESPACE

#endif  // TOOLS_GN_LABEL_PTR_H_
