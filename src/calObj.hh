/**CPPHeaderFile***************************************************************

  FileName    [calObj.hh]

  PackageName [cal]

  Synopsis    [C++ Header CAL file for exported data structures and functions.]

  Description []

  SeeAlso     []

  Author      [Steffan Sølvsten (soelvsten@cs.au.dk)]

  Copyright   [Copyright (c) 2023, Aarhus University]

  All rights reserved.

  Permission is hereby granted, without written agreement and without license
  or royalty fees, to use, copy, modify, and distribute this software and its
  documentation for any purpose, provided that the above copyright notice and
  the following two paragraphs appear in all copies of this software.

  IN NO EVENT SHALL AARHUS UNIVERSITY OR THE UNIVERSITY OF CALIFORNIA BE LIABLE
  TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
  DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN IF
  EITHER UNIVERSITY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

  BOTH UNIVERSITIES SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE. THE SOFTWARE PROVIDED HEREUNDER IS ON AN "AS IS" BASIS,
  AND THE UNIVERSITY OF CALIFORNIA HAS NO OBLIGATION TO PROVIDE MAINTENANCE,
  SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.]

  Revision    [$Id: calObj.h,v 3.0 2023/08/19 17:48:05 soelvsten Exp $]

******************************************************************************/

#ifndef _CALOBJ
#define _CALOBJ

extern "C" {
#include "cal.h"
#include "calInt.h"
}

#include <vector>
#include <sstream>
#include <string>

class BDD;
class Cal;

////////////////////////////////////////////////////////////////////////////////
/// \defgroup module__cpp C++ API
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
/// \addtogroup module__cpp
///
/// \{

////////////////////////////////////////////////////////////////////////////////
/// \brief C++ wrapper for `Cal_Bdd`, managing reference count with *RAII*.
///
/// \details These BDDs are created from a `Cal` object which owns the unique
/// node table. Hence, the life-time of any `BDD` object should not outlast its
/// parent `Cal` object. Doing so leads to *undefined behaviour*.
///
/// \see Cal
////////////////////////////////////////////////////////////////////////////////
class BDD
{
  friend Cal;

  //////////////////////////////////////////////////////////////////////////////
  // Types
public:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type of BDD identifiers, i.e. the variable name independent of the
  /// current variable ordering.
  //////////////////////////////////////////////////////////////////////////////
  using Id_t = Cal_BddId_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type of BDD indices, i.e. their placement in the current variable
  /// ordering.
  //////////////////////////////////////////////////////////////////////////////
  using Index_t = Cal_BddIndex_t;

  //////////////////////////////////////////////////////////////////////////////
  // Members
private:
  /// \brief C API reference to BDD Manager (needed to call the C API).
  Cal_BddManager _bddManager;

  /// \brief C API reference to BDD node to be managed.
  Cal_Bdd _bdd;

  //////////////////////////////////////////////////////////////////////////////
  // Constructors
protected:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Wrap C API `Cal_Bdd` to be managed by this new `BDD` object.
  ///
  /// \details This function does not increment the reference count of the given
  /// `Cal_Bdd`, assuming the C API operation already has done so.
  //////////////////////////////////////////////////////////////////////////////
  BDD(Cal_BddManager bddManager, Cal_Bdd bdd)
    : _bddManager(bddManager), _bdd(bdd)
  { }

public:
  //////////////////////////////////////////////////////////////////////////////
  /// \brief The `NULL` BDD.
  //////////////////////////////////////////////////////////////////////////////
  BDD()
    : _bddManager(NULL), _bdd(Cal_BddNull(NULL))
  { }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Copies ownership of another BDD.
  //////////////////////////////////////////////////////////////////////////////
  BDD(const BDD &other)
    : _bddManager(other._bddManager), _bdd(other._bdd)
  {
    this->UnFree();
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Copy ownership of another BDD during assignment.
  //////////////////////////////////////////////////////////////////////////////
  BDD& operator= (const BDD &other)
  {
    this->Free();

    this->_bdd = other._bdd;
    this->_bddManager = other._bddManager;

    this->UnFree();

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Move ownership of another BDD.
  //////////////////////////////////////////////////////////////////////////////
  BDD(BDD &&other)
    : _bddManager(other._bddManager), _bdd(other._bdd)
  {
    other._bdd = Cal_BddNull(other._bddManager);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Move ownership of another BDD during assignment.
  //////////////////////////////////////////////////////////////////////////////
  BDD& operator= (BDD &&other)
  {
    this->Free();

    this->_bdd = other._bdd;
    this->_bddManager = other._bddManager;

    other._bdd = Cal_BddNull(other._bddManager);

    return *this;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Decrement reference count upon leaving scope.
  //////////////////////////////////////////////////////////////////////////////
  ~BDD()
  {
    this->Free();
  }

  //////////////////////////////////////////////////////////////////////////////
  // Predicates
public:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the this BDD is constant one, `false `otherwise.
  ///
  /// \see BDD::IsZero(), BDD::IsConst()
  //////////////////////////////////////////////////////////////////////////////
  bool IsOne() const
  { return Cal_BddIsBddOne(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the this BDD is constant zero, `false `otherwise.
  ///
  /// \see BDD::IsOne(), BDD::IsConst()
  //////////////////////////////////////////////////////////////////////////////
  bool IsZero() const
  { return Cal_BddIsBddZero(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the this BDD is NULL, `false` otherwise.
  //////////////////////////////////////////////////////////////////////////////
  bool IsNull() const
  { return BDD::IsNull(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the this BDD is either constant one or constant zero,
  /// otherwise returns `false`.
  ///
  /// \see BDD::IsOne(), BDD::IsZero()
  //////////////////////////////////////////////////////////////////////////////
  bool IsConst() const
  { return Cal_BddIsBddConst(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the this BDD is a cube, `false` otherwise.
  //////////////////////////////////////////////////////////////////////////////
  bool IsCube() const
  { return Cal_BddIsCube(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if this BDD is equal to `other`, `false` otherwise.
  //////////////////////////////////////////////////////////////////////////////
  bool IsEqualTo(const BDD &other) const
  { return Cal_BddIsEqual(this->_bddManager, this->_bdd, other._bdd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if argument BDDs are equal, `false` otherwise.
  ///
  /// \see BDD::IsEqualTo()
  //////////////////////////////////////////////////////////////////////////////
  bool operator== (const BDD &other) const
  { return this->IsEqualTo(other); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `false` if argument BDDs are equal, `true` otherwise.
  ///
  /// \see BDD::IsEqualTo()
  //////////////////////////////////////////////////////////////////////////////
  bool operator!= (const BDD &other) const
  { return !(*this == other); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if this depends on `var`, `false` otherwise.
  //////////////////////////////////////////////////////////////////////////////
  bool DependsOn(const BDD &var) const
  { return Cal_BddDependsOn(this->_bddManager, this->_bdd, var._bdd); }

  //////////////////////////////////////////////////////////////////////////////
  // Node traversal and Information
public:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief BDD corresponding to the top variable of the this BDD.
  ///
  /// \see Bdd::Id(), BDD::Index()
  //////////////////////////////////////////////////////////////////////////////
  BDD If() const
  { return BDD(this->_bddManager, Cal_BddIf(this->_bddManager, this->_bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Returns the id of this BDD's top variable.
  ///
  /// \see BDD::Index()
  //////////////////////////////////////////////////////////////////////////////
  Id_t Id() const
  { return Cal_BddGetIfId(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Returns the index of this BDD's top variable.
  ///
  /// \see BDD::Id()
  //////////////////////////////////////////////////////////////////////////////
  Index_t Index() const
  { return Cal_BddGetIfIndex(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The positive cofactor of this BDD with respect to its top
  /// variable.
  //////////////////////////////////////////////////////////////////////////////
  BDD Then() const
  { return BDD(this->_bddManager, Cal_BddThen(this->_bddManager, this->_bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The negative cofactor of this BDD with respect to its top variable.
  //////////////////////////////////////////////////////////////////////////////
  BDD Else() const
  { return BDD(this->_bddManager, Cal_BddElse(this->_bddManager, this->_bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Possible types a BDD can be.
  ///
  /// \see BDD::Type()
  //////////////////////////////////////////////////////////////////////////////
  enum Type_t
  {
    /** The Zero terminal */
    Zero        = CAL_BDD_TYPE_ZERO,
    /** The One terminal */
    One         = CAL_BDD_TYPE_ONE,
    /** Constant (Non-Boolean) */
    Constant    = CAL_BDD_TYPE_CONSTANT,
    /** Positive variable */
    Posvar      = CAL_BDD_TYPE_POSVAR,
    /** Negative variable */
    Negvar      = CAL_BDD_TYPE_NEGVAR,
    /** Invalid/Missing result due to Overflow */
    Overflow    = CAL_BDD_TYPE_OVERFLOW,
    /** BDD type encompassing all other cases */
    NonTerminal = CAL_BDD_TYPE_NONTERMINAL
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD type (0, 1, +var, -var, overflow, nonterminal).
  //////////////////////////////////////////////////////////////////////////////
  Type_t Type() const
  { return static_cast<Type_t>(Cal_BddType(this->_bddManager, this->_bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief This BDD's size.
  ///
  /// \param negout If `false` then counting pretends the BDD does not have
  ///               negative-output pointers (complement edges).
  //////////////////////////////////////////////////////////////////////////////
  unsigned long Size(bool negout = true) const
  { return Cal_BddSize(this->_bddManager, this->_bdd, negout); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Fraction of valuations which make this BDD true.
  ///
  /// \remark This fraction is independent of whatever set of variables `f` is
  /// supposed to be a function of.
  //////////////////////////////////////////////////////////////////////////////
  double SatisfyingFraction() const
  { return Cal_BddSatisfyingFraction(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  // Operations
public:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Duplicate of this BDD.
  ///
  /// \see BDD::Not()
  //////////////////////////////////////////////////////////////////////////////
  BDD Identity() const
  { return BDD(this->_bddManager, Cal_BddIdentity(this->_bddManager, this->_bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Returns a BDD in positive form (regardless of this BDDs phase).
  ///
  /// \see BDD::Not()
  //////////////////////////////////////////////////////////////////////////////
  BDD Regular() const
  {
    // Unlike other BDD operations of CAL, `Cal_BddGetRegular` does not
    // increment the reference count of its output. Hence, we have to do so here
    // to compensate for the `Free(...)` in `~BDD()`.
    BDD res(this->_bddManager, Cal_BddGetRegular(this->_bddManager, this->_bdd));
    BDD::UnFree(res._bddManager, res._bdd);
    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Complement of this BDD.
  //////////////////////////////////////////////////////////////////////////////
  BDD Not() const
  { return BDD(this->_bddManager, Cal_BddNot(this->_bddManager, this->_bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \see BDD::Not()
  //////////////////////////////////////////////////////////////////////////////
  BDD operator~ () const
  { return this->Not(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Substitute a BDD variable by a function.
  ///
  /// \param g Variable to be substituted.
  ///
  /// \param h Function to substitute.
  //////////////////////////////////////////////////////////////////////////////
  BDD Compose(const BDD &g, const BDD &h) const
  { return BDD(this->_bddManager, Cal_BddCompose(this->_bddManager, this->_bdd, g._bdd, h._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Computes a BDD that implies conjunction of this and `g`.
  ///
  /// \see BDD::Implies()
  //////////////////////////////////////////////////////////////////////////////
  BDD Intersects(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddIntersects(this->_bddManager, this->_bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Computes a BDD that implies conjunction of this and `g.Not()`.
  ///
  /// \see BDD::Intersects()
  //////////////////////////////////////////////////////////////////////////////
  BDD Implies(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddImplies(this->_bddManager, this->_bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical If-Then-Else.
  ///
  /// \details Returns the BDD for the logical operation `f ? g : h`, i.e.
  /// `f&g | ~f&h`.
  ///
  /// \see BDD::And(), BDD::Nand(), BDD::Or(), BDD::Nor(), BDD::Xor(),
  /// BDD::Xnor()
  //////////////////////////////////////////////////////////////////////////////
  BDD ITE(const BDD &g, const BDD &h) const
  { return BDD(this->_bddManager, Cal_BddITE(this->_bddManager, this->_bdd, g._bdd, h._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical AND operation.
  //////////////////////////////////////////////////////////////////////////////
  BDD And(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddAnd(this->_bddManager, this->_bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \see BDD::And()
  //////////////////////////////////////////////////////////////////////////////
  BDD  operator& (const BDD &other) const
  { return this->And(other); }

  //////////////////////////////////////////////////////////////////////////////
  /// \see BDD::And()
  //////////////////////////////////////////////////////////////////////////////
  BDD& operator&= (const BDD &other)
  { return (*this = (*this) & other); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical Negated AND operation.
  //////////////////////////////////////////////////////////////////////////////
  BDD Nand(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddNand(this->_bddManager, this->_bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical OR operation.
  //////////////////////////////////////////////////////////////////////////////
  BDD Or(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddOr(this->_bddManager, this->_bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \see BDD::Or()
  //////////////////////////////////////////////////////////////////////////////
  BDD  operator| (const BDD &other) const
  { return this->Or(other); }

  //////////////////////////////////////////////////////////////////////////////
  /// \see BDD::Or()
  //////////////////////////////////////////////////////////////////////////////
  BDD& operator|= (const BDD &other)
  { return (*this = (*this) | other); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical Negated OR operation.
  //////////////////////////////////////////////////////////////////////////////
  BDD Nor(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddNor(this->_bddManager, this->_bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical XOR operation.
  //////////////////////////////////////////////////////////////////////////////
  BDD Xor(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddXor(this->_bddManager, this->_bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \see BDD::Xor()
  //////////////////////////////////////////////////////////////////////////////
  BDD  operator^ (const BDD &other) const
  { return this->Xor(other); }

  //////////////////////////////////////////////////////////////////////////////
  /// \see BDD::Xor()
  //////////////////////////////////////////////////////////////////////////////
  BDD& operator^= (const BDD &other)
  { return (*this = (*this) ^ other); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical Negated XOR operation.
  //////////////////////////////////////////////////////////////////////////////
  BDD Xnor(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddXnor(this->_bddManager, this->_bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A satisfying assignment of this BDD.
  ///
  /// \details Returns a BDD which implies this, true for some valuation on
  /// which f is true, and which has at most one node at each level.
  //////////////////////////////////////////////////////////////////////////////
  BDD Satisfy() const
  { return BDD(this->_bddManager, Cal_BddSatisfy(this->_bddManager, this->_bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Returns a special cube contained in this
  ///
  /// \details The returned BDD which implies this, is true for some valuation
  /// on which f is true, which has at most one node at each level, and which
  /// has exactly one node corresponding to each variable which is associated
  /// with something in the current variable association.
  //////////////////////////////////////////////////////////////////////////////
  BDD SatisfySupport() const
  { return BDD(this->_bddManager, Cal_BddSatisfySupport(this->_bddManager, this->_bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The function obtained by swapping two variables.
  ///
  /// \details Returns the BDD obtained by simultaneously substituting variable
  /// `g` by variable `h` and variable `h` and variable `g` in this BDD.
  ///
  /// \see BDD::Substitute()
  //////////////////////////////////////////////////////////////////////////////
  BDD SwapVars(const BDD &g, const BDD &h) const
  { return BDD(this->_bddManager, Cal_BddSwapVars(this->_bddManager, this->_bdd, g._bdd, h._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // BDD Exists(...) const

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // BDD ForAll(...) const

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // BDD RelProd(const BDD &g) const

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Generalized cofactor of this with respect to `c`.
  ///
  /// \details The constrain operator given by Coudert et al (ICCAD90) is used
  /// to find the generalized cofactor.
  ///
  /// \see BDD::Reduce()
  //////////////////////////////////////////////////////////////////////////////
  BDD Cofactor(const BDD &c) const
  { return BDD(this->_bddManager, Cal_BddCofactor(this->_bddManager, this->_bdd, c._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A function that agrees with this for all valuations which satisfy
  /// `c`.
  ///
  /// \details The result is usually smaller in terms of number of BDD nodes
  /// than this. This operation is typically used in state space searches to
  /// simplify the representation for the set of states wich will be expanded at
  /// each step.
  ///
  /// \see BDD::Cofactor()
  //////////////////////////////////////////////////////////////////////////////
  BDD Reduce(const BDD &c) const
  { return BDD(this->_bddManager, Cal_BddReduce(this->_bddManager, this->_bdd, c._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  // Debugging
public:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Prints this BDD in the human readable form.
  //////////////////////////////////////////////////////////////////////////////
  void Print(FILE *fp = stdout) const
  {
    // TODO: Extend to use your own NamingFn and TerminalIdFn
    //       (and their environment).
    Cal_BddPrintBdd(this->_bddManager, this->_bdd,
                    Cal_BddNamingFnNone, Cal_BddTerminalIdFnNone, NULL,
                    fp);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Prints the function implemented by this BDD.
  //////////////////////////////////////////////////////////////////////////////
  void FunctionPrint(std::string &name) const
  { Cal_BddFunctionPrint(this->_bddManager, this->_bdd, name.data()); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain a vector with the number of nodes at each level in f.
  ///
  /// \param negout If `false` then counting pretends the BDD does not have
  ///               negative-output pointers (complement edges).
  //////////////////////////////////////////////////////////////////////////////
  std::vector<long> Profile(bool negout = true) const
  {
    std::vector<long> results;
    results.reserve(Cal_BddVars(this->_bddManager)+1);
    Cal_BddProfile(this->_bddManager, this->_bdd, results.data(), negout);
    return results;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Prints a BDD in the human readable form.
  ///
  /// \param lineLength The maximum line length.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  //////////////////////////////////////////////////////////////////////////////
  void PrintProfile(int lineLength = 79, FILE *fp = stdout) const
  {
    // TODO: Extend to use your own NamingFn (and its environment).
    Cal_BddPrintProfile(this->_bddManager, this->_bdd,
                        Cal_BddNamingFnNone, NULL,
                        lineLength, fp);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of subfunctions of this which may be obtained by
  /// restricting variables with an index lower than *n*.
  ///
  /// \details The nth entry of the function profile array is the number of
  /// subfunctions of this which may be obtained by restricting the variables
  /// whose index is less than n. An entry of zero indicates that this is
  /// independent of the variable with the corresponding index.
  //////////////////////////////////////////////////////////////////////////////
  std::vector<long> FunctionProfile() const
  {
    std::vector<long> results;
    results.reserve(Cal_BddVars(this->_bddManager)+1);
    Cal_BddFunctionProfile(this->_bddManager, this->_bdd, results.data());
    return results;
  }


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `BDD::PrintProfile()` but displays a function profile
  /// for this.
  ///
  /// \param lineLength The maximum line length.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  //////////////////////////////////////////////////////////////////////////////
  void PrintFunctionProfile(int lineLength = 79, FILE *fp = stdout) const
  {
    // TODO: Extend to use your own NamingFn (and its environment).
    Cal_BddPrintFunctionProfile(this->_bddManager, this->_bdd,
                                Cal_BddNamingFnNone, NULL,
                                lineLength, fp);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The reference count of this BDD
  ///
  /// \details The reference count is a value between 0 and 255. If a reference
  /// count is 255 then incrementing or decrementing it has no effect. This is
  /// to safe-guard constants and variables from being garbage collected.
  //////////////////////////////////////////////////////////////////////////////
  int RefCount() const
  {
    if (this->IsNull()) {
      return 255; // <-- Simulate NULL has fixed MAX reference count.
    }

    CalBddNode_t *internal_node = CAL_BDD_POINTER(this->_bdd);

    int res;
    CalBddNodeGetRefCount(internal_node, res);

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief String representation of this BDD node.
  //////////////////////////////////////////////////////////////////////////////
  std::string ToString() const
  {
    if (this->IsNull()) { return "NULL"; }
    if (this->IsZero()) { return "(0)"; }
    if (this->IsOne())  { return "(1)"; }

    std::stringstream ss;
    ss << "("
       << this->Id() << ", "
       << this->Then()._bdd << ", "
       << this->Else()._bdd
       << ")";

    return ss.str();
  }

  //////////////////////////////////////////////////////////////////////////////
  // Conversion back and from C null-terminated arrays.
protected:

  /// \cond internal

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Convert an iterator of `BDD`s (or `int`s) into a `std::vector` of
  ///        `Cal_Bdd`.
  ///
  /// \detail By calling `.data()` on the result one exactly gets a list usable
  ///         in the C API.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  static std::vector<Cal_Bdd>
  C_Bdd_vector(Cal_BddManager bddManager, IT begin, IT end)
  {
    // TODO: tidy up with template overloading

    std::vector<Cal_Bdd> out;
    out.reserve(std::distance(begin, end));

    while (begin != end) {
      const typename IT::value_type &x = *(begin++);

      if constexpr (std::is_same_v<typename IT::value_type, BDD>) {
        // TODO: assert same 'bddManager'...

        BDD::UnFree(x._bddManager, x._bdd);
        out.push_back(x._bdd);
      } else if constexpr (std::is_same_v<typename IT::value_type, int>) {
        out.push_back(Cal_BddManagerGetVarWithId(bddManager, x));
      }
    }

    out.push_back(Cal_BddNull(bddManager));

    return out;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Convert a null-terminated list of `Cal_Bdd`s from the C API to a
  ///        C++ vector of `BDD` classes.
  //////////////////////////////////////////////////////////////////////////////
  static std::vector<BDD>
  From_C_Array(Cal_BddManager bddManager, Cal_Bdd * bddArray)
  {
    std::vector<BDD> res;

    for (int i = 0; Cal_BddIsBddNull(bddManager, bddArray[i]) == 0; i++){
      if (CalBddPreProcessing(bddManager, 1, bddArray[i]) == 0){
        return std::vector<BDD>();
      }
      res.push_back(BDD(bddManager, bddArray[i]));
    }

    return res;
  }

  /// \endcond

  //////////////////////////////////////////////////////////////////////////////
  // Memory Management
private:

  /// \cond internal

  // TODO: move further down

  //////////////////////////////////////////////////////////////////////////////
  static inline bool
  IsNull(Cal_BddManager bddManager, Cal_Bdd f)
  { return Cal_BddIsBddNull(bddManager, f); }

  //////////////////////////////////////////////////////////////////////////////
  static inline void
  Free(Cal_BddManager bddManager, Cal_Bdd f)
  { if (!BDD::IsNull(bddManager, f)) Cal_BddFree(bddManager, f); }

  //////////////////////////////////////////////////////////////////////////////
  inline void
  Free()
  { BDD::Free(this->_bddManager, this->_bdd); }

  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  static inline void
  Free(IT begin, IT end)
  {
    static_assert(std::is_same_v<typename IT::value_type, BDD>,
                  "Must be called with iterator for BDD");

    while (begin != end) (begin++)->Free();
  }

  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  static inline void
  Free(Cal_BddManager bddManager, IT begin, IT end)
  {
    static_assert(std::is_same_v<typename IT::value_type, Cal_Bdd>,
                  "Must be called with iterator for Cal_Bdd");

    while (begin != end) BDD::Free(bddManager, *(begin++));
  }

  //////////////////////////////////////////////////////////////////////////////
  static inline void
  UnFree(Cal_BddManager bddManager, Cal_Bdd f)
  { if (!BDD::IsNull(bddManager, f)) Cal_BddUnFree(bddManager, f); }

  //////////////////////////////////////////////////////////////////////////////
  inline void
  UnFree()
  { BDD::UnFree(this->_bddManager, this->_bdd); }

  /// \endcond
};

////////////////////////////////////////////////////////////////////////////////
/// \brief Core Manager of everything BDDs, variables, and more.
///
/// \remark The `Cal` class is designed based on *non-sharing ownership*. You
/// can move the ownership to someone else, but you cannot create a copy of this
/// class to have multiple owners of the same BDD Manager. Please use C++
/// references instead.
////////////////////////////////////////////////////////////////////////////////
class Cal
{
  friend BDD;

  //////////////////////////////////////////////////////////////////////////////
  // Types

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type of Cal's C++ BDD wrapping class.
  //////////////////////////////////////////////////////////////////////////////
  using Bdd_t = BDD;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type of BDD identifiers, i.e. the variable name independent of the
  /// current variable ordering.
  //////////////////////////////////////////////////////////////////////////////
  using Id_t = BDD::Id_t;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Type of BDD indices, i.e. their placement in the current variable
  /// ordering.
  //////////////////////////////////////////////////////////////////////////////
  using Index_t = BDD::Index_t;

  // ---------------------------------------------------------------------------
  // Fields
  //           TODO -----------------------------------------------
  //              Multiple Cal objects for the same Cal_BddManager
  //             ----------------------------------------------- TODO
  //
  //   Use a 'std::shared_ptr' for reference counting this 'Cal_BddManager'
  //   pointer. The 'Cal_BddManagerQuit' function is then the managed pointer's
  //   deleter.
  //
  //           NOTE -------------------------------------------- NOTE
  //
  //   If so, should all BDD objects also be part of this reference counting?
  //   Otherwise, if a BDD object survives for longer than the BDD manager, then
  //   it will result in Segmentation Faults. Arguably this is already an
  //   issue...
  Cal_BddManager _bddManager;

  //////////////////////////////////////////////////////////////////////////////
  // Constructors
public:

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Initialize a new BDD Manager.
  //////////////////////////////////////////////////////////////////////////////
  Cal()
    : _bddManager(Cal_BddManagerInit())
  { }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Initialize a new BDD Manager with variables `[1, numVars]`.
  //////////////////////////////////////////////////////////////////////////////
  Cal(unsigned int numVars)
    : Cal()
  {
    // Create variables
    for (Id_t i = 0; i < numVars; ++i) {
      this->CreateNewVarLast();
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // TODO: Copy constructor (requires reference counting)
  Cal(const Cal &o) = delete;

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Move ownership of C object.
  //////////////////////////////////////////////////////////////////////////////
  Cal(Cal &&o)
    : _bddManager(o._bddManager)
  {
    o._bddManager = nullptr;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Clear memory of BDD Manager.
  ///
  /// \details If this object owns a `Cal_BddManager` from the C API, then that
  /// one is properly reset and freed.
  //////////////////////////////////////////////////////////////////////////////
  ~Cal()
  {
    if (this->_bddManager)
      Cal_BddManagerQuit(this->_bddManager);
  }

  //////////////////////////////////////////////////////////////////////////////
  // Settings + Statistics

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sets appropriate fields of BDD Manager.
  ///
  /// \param reorderingThreshold The number of nodes below which reordering will
  ///                            NOT beinvoked.
  ///
  /// \param maxForwardedNodes The maximum number of forwarded nodes which are
  ///                          allowed (at that point the cleanup must be done)
  ///
  /// \param repackAfterGCThreshold The fraction of the page utilized that
  ///                               garbage collection has to achieve.
  ///
  /// \param tableRepackThreshold The fraction of the page utilized below which
  ///                             repacking has to be invoked.
  //////////////////////////////////////////////////////////////////////////////
  void SetParameters(long reorderingThreshold,
                     long maxForwardedNodes,
                     double repackAfterGCThreshold,
                     double tableRepackThreshold)
  {
    return Cal_BddManagerSetParameters(this->_bddManager,
                                       reorderingThreshold,
                                       maxForwardedNodes,
                                       repackAfterGCThreshold,
                                       tableRepackThreshold);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of BDD nodes.
  ///
  /// \see Cal::TotalSize()
  //////////////////////////////////////////////////////////////////////////////
  unsigned long Nodes() const
  { return Cal_BddManagerGetNumNodes(this->_bddManager); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of BDD variables.
  //////////////////////////////////////////////////////////////////////////////
  long Vars() const
  { return Cal_BddVars(this->_bddManager); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the node limit has been exceeded, `false` otherwise.
  ///
  /// \sideeffect The overflow flag is cleared.
  ///
  /// \see Cal::NodeLimit()
  //////////////////////////////////////////////////////////////////////////////
  bool Overflow() const
  { return Cal_BddOverflow(this->_bddManager); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of nodes in the Unique table.
  ///
  /// \see Cal::Nodes()
  //////////////////////////////////////////////////////////////////////////////
  unsigned long TotalSize() const
  { return Cal_BddTotalSize(this->_bddManager); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Prints miscellaneous BDD statistics.
  //////////////////////////////////////////////////////////////////////////////
  void Stats(FILE* fp = stdout) const
  { Cal_BddStats(this->_bddManager, fp); }

  // TODO: obtain error

  //////////////////////////////////////////////////////////////////////////////
  // Memory and Garbage Collection

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sets the node limit to `newLimit` and returns the previous limit.
  ///
  /// \sideeffect Threshold for garbage collection may change.
  ///
  /// \see Cal::SetGCLimit(), Cal::GC()
  //////////////////////////////////////////////////////////////////////////////
  long NodeLimit(long newLimit)
  { return Cal_BddNodeLimit(this->_bddManager, newLimit); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Enable or Disable garbage collection.
  //////////////////////////////////////////////////////////////////////////////
  void SetGCMode(bool enableGC)
  { Cal_BddSetGCMode(this->_bddManager, enableGC); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sets the limit of the garbage collection.
  ///
  /// \details It tries to set the limit at twice the number of nodes in the
  /// manager at the current point. However, the limit is not allowed to fall
  /// below the `MIN_GC_LIMIT` or to exceed the value of node limit (if one
  /// exists).
  ///
  /// \see Cal::NodeLimit()
  //////////////////////////////////////////////////////////////////////////////
  void SetGCLimit()
  { Cal_BddManagerSetGCLimit(this->_bddManager); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Invokes the garbage collection at the manager level.
  ///
  /// \details For each variable in the increasing id free nodes with reference
  /// count equal to zero freeing a node results in decrementing reference count
  /// of then and else nodes by one.
  //////////////////////////////////////////////////////////////////////////////
  void GC()
  { Cal_BddManagerGC(this->_bddManager); }

  //////////////////////////////////////////////////////////////////////////////
  // Reordering

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The possible methods (algorithms) for variable reordering.
  ///
  /// \see Cal::DynamicReordering, Cal::ReorderMethod
  //////////////////////////////////////////////////////////////////////////////
  enum ReorderTechnique {
    None = CAL_REORDER_NONE,
    Sift = CAL_REORDER_SIFT,
    Window = CAL_REORDER_WINDOW
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The possible technique to be used to execute each method (c.f.
  /// `Cal::ReorderTechnique`).
  ///
  /// \see Cal::DynamicReordering, Cal::ReorderTechnique
  //////////////////////////////////////////////////////////////////////////////
  enum ReorderMethod {
    BF = CAL_REORDER_METHOD_BF,
    DF = CAL_REORDER_METHOD_DF
  };

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Specify dynamic reordering technique and method.
  ///
  /// \see Cal::Reorder()
  //////////////////////////////////////////////////////////////////////////////
  void DynamicReordering(ReorderTechnique technique, ReorderMethod method = ReorderMethod::DF)
  {
    Cal_BddDynamicReordering(this->_bddManager, technique, method);
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Invoke the current dynamic reodering method.
  ///
  /// \sideeffect Indexes of a variable may change due to reodering.
  ///
  /// \see Cal::DynamicReordering()
  //////////////////////////////////////////////////////////////////////////////
  void Reorder()
  { Cal_BddReorder(this->_bddManager); }

  //////////////////////////////////////////////////////////////////////////////
  // Association List

  // TODO: use RAII to hide association identifiers
  // TODO: container-based functions

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Creates or finds a variable association.
  ///
  /// \param begin Iterator pointing to the start of the given range.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \param pairs If `false`, the array assumed to be an array of variables.
  ///              If `true`, it is interpreted as consecutive pairs of
  ///              variables (and hence it must be of even length).
  ///
  /// \returns An integer identifier for this association. If the given
  ///          association is equivalent to one which already exists, the same
  ///          identifier is used for both, and the reference count of the
  ///          association is increased by one.
  ///
  /// \see Cal::AssociationSetCurrent()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  int AssociationInit(IT begin, IT end, const bool pairs = false)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const int res = Cal_AssociationInit(this->_bddManager, c_arg.data(), pairs);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sets the current variable association to the one given.
  ///
  /// \returns ID of the prior association (if any). A return value of -1
  /// indicates the temporary association.
  ///
  /// \see Cal::AssociationQuit()
  //////////////////////////////////////////////////////////////////////////////
  int AssociationSetCurrent(int i)
  { return Cal_AssociationSetCurrent(this->_bddManager, i); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Deletes the variable association given by id.
  ///
  /// \details Decrements the reference count of the variable association with
  ///          identifier id, and frees it if the reference count becomes zero.
  ///
  /// \see Cal::AssociationInit()
  //////////////////////////////////////////////////////////////////////////////
  void AssociationQuit(int i)
  { Cal_AssociationQuit(this->_bddManager, i); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Sets the temporary variable association.
  ///
  /// \param begin Iterator pointing to the start of the given range.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \param pairs Similar to `Cal::AssociationInit()`.
  ///
  /// \see Cal::AssociationInit()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  void TempAssociationInit(IT begin, const IT end, const bool pairs = false)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    Cal_TempAssociationInit(this->_bddManager, c_arg.data(), pairs);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Adds to the temporary variable association.
  ///
  /// \param begin Iterator pointing to the start of the given range.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \param pairs Similar to `Cal::AssociationInit()`.
  ///
  /// \see Cal::TempAssociationInit(), Cal::AssociationInit()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  void TempAssociationAugment(IT begin, const IT end, const bool pairs = false)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    Cal_TempAssociationAugment(this->_bddManager, c_arg.data(), pairs);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Cleans up temporary association.
  ///
  /// \see Cal::TempAssociationInit()
  //////////////////////////////////////////////////////////////////////////////
  void TempAssociationQuit()
  { Cal_TempAssociationQuit(this->_bddManager); }

  //////////////////////////////////////////////////////////////////////////////
  // Save / Load BDDs

  // BDD UndumpBdd(IT vars_begin, IT vars_end, FILE *f, int &error)
  // BDD DumpBdd(const BDD &f, IT vars_begin, IT vars_end, FILE *f, int &error)

  //////////////////////////////////////////////////////////////////////////////
  // BDD Constructors

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The NULL BDD.
  //////////////////////////////////////////////////////////////////////////////
  BDD Null() const
  { return BDD(this->_bddManager, Cal_BddNull(this->_bddManager)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD for the constant one.
  ///
  /// \see Cal::Zero()
  //////////////////////////////////////////////////////////////////////////////
  BDD One() const
  { return BDD(this->_bddManager, Cal_BddOne(this->_bddManager)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The BDD for the constant zero.
  ///
  /// \see Cal::One()
  //////////////////////////////////////////////////////////////////////////////
  BDD Zero() const
  { return BDD(this->_bddManager, Cal_BddZero(this->_bddManager)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The variable with the specified id, null if no such variable exists
  ///
  /// \see Cal::Index()
  //////////////////////////////////////////////////////////////////////////////
  BDD Id(Id_t id) const
  { return BDD(this->_bddManager, Cal_BddManagerGetVarWithId(this->_bddManager, id)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The variable with the specified index, null if no such variable
  /// exists.
  ///
  /// \see Cal::Id()
  //////////////////////////////////////////////////////////////////////////////
  BDD Index(Index_t idx) const
  { return BDD(this->_bddManager, Cal_BddManagerGetVarWithIndex(this->_bddManager, idx)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create and obtain a new variable at the start of the variable order.
  //////////////////////////////////////////////////////////////////////////////
  BDD CreateNewVarFirst()
  { return BDD(this->_bddManager, Cal_BddManagerCreateNewVarFirst(this->_bddManager)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create and obtain a new variable at the end of the variable order.
  //////////////////////////////////////////////////////////////////////////////
  BDD CreateNewVarLast()
  { return BDD(this->_bddManager, Cal_BddManagerCreateNewVarLast(this->_bddManager)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create and obtain a new variable before the specified one in the
  /// variable order.
  //////////////////////////////////////////////////////////////////////////////
  BDD CreateNewVarBefore(const BDD &x)
  { return BDD(this->_bddManager, Cal_BddManagerCreateNewVarBefore(this->_bddManager, x._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Create and obtain a new variable after the specified one in the
  /// variable order.
  //////////////////////////////////////////////////////////////////////////////
  BDD CreateNewVarAfter(const BDD &x)
  { return BDD(this->_bddManager, Cal_BddManagerCreateNewVarAfter(this->_bddManager, x._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  // BDD Predicates

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the given BDD is NULL, `false` otherwise.
  //////////////////////////////////////////////////////////////////////////////
  bool IsNull(const BDD &f) const
  { return f.IsNull(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the given BDD is constant one, `false `otherwise.
  ///
  /// \see BDD::IsOne, Cal::IsZero(), Cal::IsConst()
  //////////////////////////////////////////////////////////////////////////////
  bool IsOne(const BDD &f) const
  { return f.IsOne(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the given BDD is constant zero, `false `otherwise.
  ///
  /// \see BDD::IsZero(), Cal::IsOne(), Cal::IsConst()
  //////////////////////////////////////////////////////////////////////////////
  bool IsZero(const BDD &f) const
  { return f.IsZero(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the given BDD is either constant one or constant zero,
  /// otherwise returns `false`.
  ///
  /// \see BDD::IsConst(), Cal::IsOne(), Cal::isZero()
  //////////////////////////////////////////////////////////////////////////////
  bool IsConst(const BDD &f) const
  { return f.IsConst(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the given BDD is a cube, `false` otherwise.
  //////////////////////////////////////////////////////////////////////////////
  bool IsCube(const BDD &f) const
  { return f.IsCube(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the two BDDs are equal, `false` otherwise.
  ///
  /// \see BDD::IsEqualTo()
  //////////////////////////////////////////////////////////////////////////////
  bool IsEqual(const BDD &f, const BDD &g) const
  { return f.IsEqualTo(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief `true` if the BDD `f` depends on `var`, `false` otherwise.
  ///
  /// \see BDD::DependsOn()
  //////////////////////////////////////////////////////////////////////////////
  bool DependsOn(const BDD &f, const BDD &var) const
  { return f.DependsOn(var); }

  //////////////////////////////////////////////////////////////////////////////
  // BDD Information

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Fraction of valuations which make this BDD true.
  ///
  /// \remark This fraction is independent of whatever set of variables `f` is
  /// supposed to be a function of.
  //////////////////////////////////////////////////////////////////////////////
  double SatisfyingFraction(const BDD &f)
  { return f.SatisfyingFraction(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief This BDD's size.
  ///
  /// \param f BDD of interest.
  ///
  /// \param negout If `false` then counting pretends the BDD does not have
  ///               negative-output pointers (complement edges).
  //////////////////////////////////////////////////////////////////////////////
  unsigned long Size(const BDD &f, bool negout = true)
  { return f.Size(negout); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::Size()` for an iterator of BDDs. But, this
  /// accounts for sharing of nodes.
  ///
  /// \param begin Iterator pointing to the start of the set of BDDs.
  ///
  /// \param end Iterator pointing to the end.
  ///
  /// \param negout If `false` then counting pretends the BDD does not have
  ///               negative-output pointers (complement edges).
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  unsigned long Size(IT begin, IT end, bool negout = true)
  {
    static_assert(std::is_same_v<typename IT::value_type, BDD>,
                  "Must be called with iterator for BDD");

    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const unsigned long res = Cal_BddSizeMultiple(this->_bddManager, c_arg.data(), negout);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::Size()` for a container of BDDs. But, this
  /// accounts for sharing of nodes.
  ///
  /// \param c Container of BDDs which supports `begin()` and `end()` iterators.
  ///
  /// \param negout If `false` then counting pretends the BDD does not have
  ///               negative-output pointers (complement edges).
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  unsigned long Size(Container c, bool negout = true)
  { return Size(std::begin(c), std::end(c), negout); }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // container_t<BDD> Support(const BDD &f);

  //////////////////////////////////////////////////////////////////////////////
  // Manipulation

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Duplicate of given BDD.
  ///
  /// \see BDD::Identity(), Cal::Not()
  //////////////////////////////////////////////////////////////////////////////
  BDD Identity(const BDD &f)
  { return f.Identity(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The given BDD in positive form (regardless of its phase).
  ///
  /// \see BDD::Regular()
  //////////////////////////////////////////////////////////////////////////////
  BDD Regular(const BDD &f)
  { return f.Regular(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Complement of the given BDD.
  //////////////////////////////////////////////////////////////////////////////
  BDD Not(const BDD &f)
  { return f.Not(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Substitute a BDD variable by a function, i.e. *f[h/g]*.
  ///
  /// \see BDD::Compose()
  //////////////////////////////////////////////////////////////////////////////
  BDD Compose(const BDD &f, const BDD &g, const BDD &h)
  { return f.Compose(g, h); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Computes a BDD that implies the conjunction of both BDDs.
  ///
  /// \see BDD::Intersects(), Cal::Implies()
  //////////////////////////////////////////////////////////////////////////////
  BDD Intersects(const BDD &f, const BDD &g)
  { return f.Intersects(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Computes a BDD that implies the conjunction of `f` and `g.Not()`.
  ///
  /// \see BDD::Implies(), Cal::Intersects()
  //////////////////////////////////////////////////////////////////////////////
  BDD Implies(const BDD &f, const BDD &g)
  { return f.Implies(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical If-Then-Else.
  ///
  /// \see BDD::ITE()
  //////////////////////////////////////////////////////////////////////////////
  BDD ITE(const BDD &f, const BDD &g, const BDD &h)
  { return f.ITE(g, h); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical AND operation.
  ///
  /// \see BDD::And()
  //////////////////////////////////////////////////////////////////////////////
  BDD And(const BDD &f, const BDD &g)
  { return f.And(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical AND of iterator of `BDD` or `int`.
  ///
  /// \details If `IT::value_type` is `int` then the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs or
  /// integers.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \see BDD::And()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  BDD And(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const BDD res(this->_bddManager, Cal_BddMultiwayAnd(this->_bddManager, c_arg.data()));

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical AND of container with `BDD`s or `int`s.
  ///
  /// \details If the container is of `int`s, the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param c Container of BDDs or integers which supports `begin()` and
  /// `end()` iterators.
  ///
  /// \see BDD::And()
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  BDD And(const Container &c)
  { return And(std::begin(c), std::end(c)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical Negated AND operation.
  ///
  /// \see BDD::Nand()
  //////////////////////////////////////////////////////////////////////////////
  BDD Nand(const BDD &f, const BDD &g)
  { return f.Nand(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical OR operation.
  //////////////////////////////////////////////////////////////////////////////
  BDD Or(const BDD &f, const BDD &g)
  { return f.Or(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical OR of iterator of `BDD` or `int`.
  ///
  /// \details If `IT::value_type` is `int` then the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs or
  /// integers.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \see BDD::Or()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  BDD Or(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const BDD res(this->_bddManager, Cal_BddMultiwayOr(this->_bddManager, c_arg.data()));

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical OR of container with `BDD`s or `int`s.
  ///
  /// \details If the container is of `int`s, the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param c Container of BDDs or integers which supports `begin()` and
  /// `end()` iterators.
  ///
  /// \see BDD::Or()
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  BDD Or(const Container &c)
  { return Or(std::begin(c), std::end(c)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical Negated OR operation.
  ///
  /// \see BDD::Nor()
  //////////////////////////////////////////////////////////////////////////////
  BDD Nor(const BDD &f, const BDD &g)
  { return f.Nor(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical XOR operation.w
  //////////////////////////////////////////////////////////////////////////////
  BDD Xor(const BDD &f, const BDD &g)
  { return f.Xor(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical XOR of iterator of `BDD` or `int`.
  ///
  /// \details If the container is of `int`s, the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs or
  /// integers.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \see BDD::Xor()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  BDD Xor(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const BDD res(this->_bddManager, Cal_BddMultiwayXor(this->_bddManager, c_arg.data()));

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical XOR of container with `BDD`s or `int`s.
  ///
  /// \details If the container is of `int`s, the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param c Container of BDDs or integers which supports `begin()` and
  /// `end()` iterators.
  ///
  /// \see BDD::Xor()
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  BDD Xor(const Container &c)
  { return Xor(std::begin(c), std::end(c)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical Negated XOR operation.
  //////////////////////////////////////////////////////////////////////////////
  BDD Xnor(const BDD &f, const BDD &g)
  { return f.Xnor(g); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Pairwise AND of `BDD`s or `int`s provided by an iterator.
  ///
  /// \details If `IT::value_type` is `int` then the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs or
  /// integers.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \see BDD::And()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  std::vector<BDD>
  PairwiseAnd(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    std::vector<BDD> res =
      BDD::From_C_Array(this->_bddManager, Cal_BddPairwiseAnd(this->_bddManager, c_arg.data()));

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Pairwise AND of `BDD`s or `int`s within a container.
  ///
  /// \details If the container is of `int`s, the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param c Container of BDDs or integers which supports `begin()` and
  /// `end()` iterators.
  ///
  /// \see BDD::And()
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  std::vector<BDD>
  PairwiseAnd(const Container &c)
  { return PairwiseAnd(std::begin(c), std::end(c)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Pairwise OR of `BDD`s or `int`s provided by an iterator.
  ///
  /// \details If `IT::value_type` is `int` then the integers are converted into
  /// a `BDD` with `Call::Id()`.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs or
  /// integers.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \see BDD::Or()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  std::vector<BDD>
  PairwiseOr(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    std::vector<BDD> res =
      BDD::From_C_Array(this->_bddManager, Cal_BddPairwiseOr(this->_bddManager, c_arg.data()));

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Pairwise OR of `BDD`s or `int`s within a container.
  ///
  /// \details If the container is of `int`s, the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param c Container of BDDs or integers which supports `begin()` and
  /// `end()` iterators.
  ///
  /// \see BDD::Or()
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  std::vector<BDD>
  PairwiseOr(const Container &c)
  { return PairwiseOr(std::begin(c), std::end(c)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Pairwise XOR of `BDD`s or `int`s provided by an iterator.
  ///
  /// \details If `IT::value_type` is `int` then the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  std::vector<BDD>
  PairwiseXor(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    std::vector<BDD> res =
      BDD::From_C_Array(this->_bddManager, Cal_BddPairwiseXor(this->_bddManager, c_arg.data()));

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Pairwise XOR of `BDD`s or `int`s within a container.
  ///
  /// \details If the container is of `int`s, the integers are converted into
  /// a `BDD` with `Cal::Id()`.
  ///
  /// \param c Container of BDDs or integers which supports `begin()` and
  /// `end()` iterators.
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  std::vector<BDD>
  PairwiseXor(const Container &c)
  { return PairwiseXor(std::begin(c), std::end(c)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief A satisfying assignment of this BDD.
  ///
  /// \copydetails BDD::Satisfy()
  ///
  /// \see BDD::Satisfy()
  //////////////////////////////////////////////////////////////////////////////
  BDD Satisfy(const BDD &f)
  { return f.Satisfy(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Returns a special cube contained in this
  ///
  /// \copydetails BDD::Satisfy()
  ///
  /// \see BDD::SatisfySupport()
  //////////////////////////////////////////////////////////////////////////////
  BDD SatisfySupport(const BDD &f)
  { return f.SatisfySupport(); }

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // BDD Substitute(const BDD &f)

  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // BDD VarSubstitute(const BDD &f)

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The function obtained by swapping two variables.
  ///
  /// \details Returns the BDD obtained by simultaneously substituting variable
  /// `g` by variable `h` and variable `h` and variable `g` in `f`.
  ///
  /// \see BDD::SwapVars(), Cal::Substitute()
  //////////////////////////////////////////////////////////////////////////////
  BDD SwapVars(const BDD &f, const BDD &g, const BDD &h)
  { return f.SwapVars(g, h); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Existentially quantification of some variables from the given BDD.
  ///
  /// \details Returns the BDD for `f` with all the variables that are
  /// paired with something in the current variable association
  /// existentially quantified out.
  ///
  /// \see Cal::AssociationInit(), Cal::TempAssociationInit()
  //////////////////////////////////////////////////////////////////////////////
  BDD Exists(const BDD &f)
  { return BDD(this->_bddManager, Cal_BddExists(this->_bddManager, f._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Universal quantification of some variables from the given BDD.
  ///
  /// \details Returns the BDD for `f` with all the variables that are
  /// paired with something in the current variable association
  /// universally quantified out.
  ///
  /// \see Cal::AssociationInit(), Cal::TempAssociationInit()
  //////////////////////////////////////////////////////////////////////////////
  BDD ForAll(const BDD &f)
  { return BDD(this->_bddManager, Cal_BddForAll(this->_bddManager, f._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Logical AND of the argument BDDs and existentially quantifying some
  /// variables from the product.
  ///
  /// \copydetails Cal_BddRelProd
  ///
  /// \see Cal::And(), Cal::Exists(), Cal::AssociationInit(),
  /// Cal::AssociationSetCurrent(), Cal::TempAssociationInit()
  //////////////////////////////////////////////////////////////////////////////
  BDD RelProd(const BDD &f, const BDD &g)
  { return BDD(this->_bddManager, Cal_BddRelProd(this->_bddManager, f._bdd, g._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Generalized cofactor of `f` with respect to `c`.
  ///
  /// \details Returns the generalized cofactor of `f` with respect to BDD `c`.
  /// The constrain operator given by Coudert et al (ICCAD90) is used to find
  /// the generalized cofactor.
  ///
  /// \see BDD::Cofactor(), Cal::Reduce()
  //////////////////////////////////////////////////////////////////////////////
  BDD Cofactor(const BDD &f, const BDD &c)
  { return f.Cofactor(c); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Function that agrees with `f` for all valuations that satisfy `c`.
  ///
  /// \details The result is usually smaller in terms of number of BDD nodes
  /// than this. This operation is typically used in state space searches to
  /// simplify the representation for the set of states wich will be expanded at
  /// each step.
  ///
  /// \see BDD::Reduce(), Cal::Cofactor()
  //////////////////////////////////////////////////////////////////////////////
  BDD Reduce(const BDD &f, const BDD &c)
  { return f.Reduce(c); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Function that contains `fMin` and is contained in `fMax`.
  ///
  /// \details Returns a minimal BDD `f` which is contains `fMin` and is
  /// contained in `fMax` (`fMin <= f <= fMax`). This operation is typically
  /// used in state space searches to simplify the representation for the set of
  /// states wich will be expanded at each step (`Rk Rk-1' <= f <= Rk`).
  ///
  /// \see Cal::Cofactor(), Cal::Reduce()
  //////////////////////////////////////////////////////////////////////////////
  BDD Between(const BDD &fMin, const BDD &fMax)
  { return BDD(this->_bddManager, Cal_BddBetween(this->_bddManager, fMin._bdd, fMax._bdd)); }

  //////////////////////////////////////////////////////////////////////////////
  // BDD Node Access / Traversal

  //////////////////////////////////////////////////////////////////////////////
  /// \brief BDD corresponding to the top variable of the given BDD.
  ///
  /// \see Bdd::If(), Cal::IfId(), Cal::IfIndex()
  //////////////////////////////////////////////////////////////////////////////
  BDD If(const BDD &f) const
  { return f.If(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Returns the id of the given BDD's top variable.
  ///
  /// \see BDD::Id(), Cal::IfIndex()
  //////////////////////////////////////////////////////////////////////////////
  Id_t IfId(const BDD &f) const
  { return f.Id(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Returns the index of this BDD's top variable.
  ///
  /// \see BDD::Index(), Cal::IfId()
  //////////////////////////////////////////////////////////////////////////////
  Index_t IfIndex(const BDD &f) const
  { return f.Index(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The positive cofactor of the given BDD with respect to its top
  /// variable.
  ///
  /// \see BDD::Then()
  //////////////////////////////////////////////////////////////////////////////
  BDD Then(const BDD &f)
  { return f.Then(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The negative cofactor of the given BDD with respect to its top
  /// variable.
  ///
  /// \see BDD::Else()
  //////////////////////////////////////////////////////////////////////////////
  BDD Else(const BDD &f)
  { return f.Else(); }

  //////////////////////////////////////////////////////////////////////////////
  // Debugging

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Prints a BDD in the human readable form.
  ///
  /// \param f BDD to be printed.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  ///
  /// \see BDD::Print()
  //////////////////////////////////////////////////////////////////////////////
  void Print(const BDD &f, FILE *fp = stdout) const
  { f.Print(fp); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Prints the function implemented by the given BDD.
  ///
  /// \see BDD::FunctionPrint()
  //////////////////////////////////////////////////////////////////////////////
  void FunctionPrint(const BDD &f, std::string &name) const
  { f.FunctionPrint(name); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Obtain a vector with the number of nodes at each level in f.
  ///
  /// \param f BDD of interest.
  ///
  /// \param negout If `false` then counting pretends the BDD does not have
  ///               negative-output pointers (complement edges).
  ///
  /// \see BDD::Profile()
  //////////////////////////////////////////////////////////////////////////////
  std::vector<long> Profile(const BDD &f, bool negout = true) const
  { return f.Profile(negout); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::Profile()` for an iterator of BDDs. But, this
  /// accounts for sharing of nodes.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \param negout If `false` then counting pretends the BDD does not have
  ///               negative-output pointers (complement edges).
  ///
  /// \see BDD::Profile()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  std::vector<long> Profile(IT begin, IT end, bool negout = true) const
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    std::vector<long> levelCounts;
    levelCounts.reserve(this->Vars()+1);

    Cal_BddProfileMultiple(this->_bddManager,
                           c_arg.data(),
                           levelCounts.data(),
                           negout);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return levelCounts;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::Profile()` for a container of BDDs. But, this
  /// accounts for sharing of nodes.
  ///
  /// \param negout If `false` then counting pretends the BDD does not have
  ///               negative-output pointers (complement edges).
  ///
  /// \param c Container of BDDs which supports `begin()` and `end()` iterators.
  ///
  /// \see BDD::Profile()
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  std::vector<long> Profile(const Container &c, bool negout = true) const
  { return Profile(c.begin(), c.end(), negout); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Prints a BDD in the human readable form.
  ///
  /// \param f BDD to be printed.
  ///
  /// \param lineLength The maximum line length.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  //////////////////////////////////////////////////////////////////////////////
  void PrintProfile(const BDD &f, int lineLength = 79, FILE *fp = stdout) const
  { return f.PrintProfile(lineLength, fp); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::PrintProfile()` for an iterator of BDDs. But, this
  /// accounts for sharing of nodes.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \param lineLength The maximum line length.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  void PrintProfile(IT begin, IT end,
                    int lineLength = 79,
                    FILE *fp = stdout) const
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    // TODO: Extend to use your own NamingFn (and its environment).
    Cal_BddPrintProfileMultiple(this->_bddManager, c_arg.data(),
                                Cal_BddNamingFnNone, NULL,
                                lineLength, fp);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::PrintProfile` for a container of BDDs. But, this
  /// accounts for sharing of nodes.AssociationSetCurrent
  ///
  /// \param c Container of BDDs which supports `begin()` and `end()` iterators.
  ///
  /// \param lineLength The maximum line length.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  void PrintProfile(const Container &c,
                    int lineLength = 79,
                    FILE *fp = stdout) const
  { PrintProfile(c.begin(), c.end(), lineLength, fp); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief The number of subfunctions of the given BDD which may be obtained
  /// by restricting variables with an index lower than *n*.
  ///
  /// \copydetails BDD::FunctionProfile()
  ///
  /// \see BDD::FunctionProfile()
  //////////////////////////////////////////////////////////////////////////////
  std::vector<long> FunctionProfile(const BDD &f) const
  { return f.FunctionProfile(); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::FunctionProfile` for an iterator of BDDs. But,
  /// this accounts for sharing of nodes.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \see BDD::FunctionProfile()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  std::vector<long> FunctionProfile(IT begin, IT end) const
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    std::vector<long> funcCounts;
    funcCounts.reserve(Vars()+1);

    Cal_BddFunctionProfileMultiple(this->_bddManager, c_arg.data(), funcCounts.data());

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return funcCounts;
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::FunctionProfile` for a container of BDDs. But,
  /// this accounts for sharing of nodes.
  ///
  /// \param c Container of BDDs which supports `begin()` and `end()` iterators.
  ///
  /// \see BDD::FunctionProfile()
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  std::vector<long> FunctionProfile(const Container &c) const
  { return FunctionProfile(c.begin(), c.end()); }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::PrintProfile()` but displays a function profile
  /// for the given BDD.
  ///
  /// \param f BDD to be printed.
  ///
  /// \param lineLength The maximum line length.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  ///
  /// \see BDD::PrintFunctionProfile()
  //////////////////////////////////////////////////////////////////////////////
  void PrintFunctionProfile(const BDD &f,
                            int lineLength = 79,
                            FILE *fp = stdout) const
  { return f.PrintFunctionProfile(lineLength, fp); }


  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::PrintFunctionProfile` for an iterator of BDDs.
  /// But, this accounts for sharing of nodes.
  ///
  /// \param begin Iterator pointing to the start of the given range of BDDs.
  ///
  /// \param end Iterator pointing to the end of the given range.
  ///
  /// \param lineLength The maximum line length.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  ///
  /// \see BDD::PrintFunctionProfile()
  //////////////////////////////////////////////////////////////////////////////
  template<typename IT>
  void PrintFunctionProfile(IT begin, IT end,
                    int lineLength = 79,
                    FILE *fp = stdout) const
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    // TODO: Extend to use your own NamingFn (and its environment).
    Cal_BddPrintFunctionProfileMultiple(this->_bddManager, c_arg.data(),
                                        Cal_BddNamingFnNone, NULL,
                                        lineLength, fp);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());
  }

  //////////////////////////////////////////////////////////////////////////////
  /// \brief Similar to `Cal::PrintFunctionProfile` for a container of BDDs.
  /// But, this accounts for sharing of nodes.
  ///
  /// \param c Container of BDDs which supports `begin()` and `end()` iterators.
  ///
  /// \param lineLength The maximum line length.
  ///
  /// \param fp Pointer to the `FILE` to output to (default `stdout`).
  ///
  /// \see BDD::PrintFunctionProfile()
  //////////////////////////////////////////////////////////////////////////////
  template<typename Container>
  void PrintFunctionProfile(const Container &c,
                            int lineLength = 79,
                            FILE *fp = stdout) const
  { PrintFunctionProfile(c.begin(), c.end(), lineLength, fp); }

  //////////////////////////////////////////////////////////////////////////////
  // BDD Pipelining

  // TODO: class Pipeline using RAII and operator overloading

  //////////////////////////////////////////////////////////////////////////////
  // NOTE: These should never be exposed (hidden inside of the BDD class)

  // void Free();
  // void UnFree();
};

/// \}
////////////////////////////////////////////////////////////////////////////////

#endif /* _CALOBJ */
