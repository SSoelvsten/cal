#ifndef _CALOBJ
#define _CALOBJ

extern "C" {
#include <cal.h>
#include <calInt.h>
}

// -----------------------------------------------------------------------------
// Type declarations
class BDD;
class Cal;

// -----------------------------------------------------------------------------
// BDD Manager
class Cal {
  friend class BDD;

  // ---------------------------------------------------------------------------
  // Fields
private:
  // TODO: std::shared_ptr for reference counting the Cal_BddManager
  Cal_BddManager _bddManager;

  // ---------------------------------------------------------------------------
  // Constructors
public:
  Cal() = delete;

  Cal(unsigned int numVars) : _bddManager(Cal_BddManagerInit())
  {
    // Create variables
    for (Cal_BddId_t i = 0; i < numVars; ++i){
      Cal_BddManagerCreateNewVarLast(_bddManager);
    }

    // Set reordering method to BF (cannot be set to DF - see Issue #5)
    _bddManager->reorderMethod = CAL_REORDER_METHOD_BF;

    // TODO: Cal_BddManagerSetParameters
  }

  // TODO: allow multiple copies to access Cal
  Cal(const Cal &o) = delete;

  ~Cal()
  {
    Cal_BddManagerQuit(_bddManager);
  }

  // ---------------------------------------------------------------------------
  // Definition of Operations
  enum ReorderTechnique {
    NONE = CAL_REORDER_NONE,
    SIFT = CAL_REORDER_SIFT,
    WINDOW = CAL_REORDER_WINDOW
  };

  void DynamicReordering(ReorderTechnique technique)
  { Cal_BddDynamicReordering(_bddManager, technique); }

  void Reorder()
  { Cal_BddReorder(_bddManager); }

  void SetGCMode(bool enableGC)
  { Cal_BddSetGCMode(_bddManager, enableGC); }

  void GC()
  { Cal_BddManagerGC(_bddManager); }

  unsigned long GetNumNodes()
  { return Cal_BddManagerGetNumNodes(_bddManager); }

  long Vars()
  { return Cal_BddVars(_bddManager); }

  void Stats(FILE* fp = stdout)
  { Cal_BddStats(_bddManager, fp); }

  // ---------------------------------------------------------------------------
  // Declaration of Association List
  template<typename BDD_IT>
  int AssociationInit(BDD_IT begin, const BDD_IT end, const bool pairs = false);

  void AssociationQuit(int i);

  int AssociationSetCurrent(int i);

  // ---------------------------------------------------------------------------
  // Declaration of BDD Constructors
  BDD Null() const;
  BDD One() const;
  BDD Zero() const;
  BDD Id(Cal_BddId_t id) const;
  BDD Index(Cal_BddIndex_t idx) const;

  // Declaration of BDD Predicates
  bool IsNull(BDD f) const;
  bool IsNotNull(BDD f) const;
  bool IsOne(BDD f) const;
  bool IsZero(BDD f) const;
  bool IsConst(BDD f) const;
  bool IsCube(BDD f) const;
  bool IsEqual(BDD f, BDD g) const;

  // Declaration of BDD Operations
  BDD Else(BDD f);
  BDD Not(BDD f);
  BDD Implies(BDD f, BDD g);
  BDD Compose(BDD f, BDD g, BDD h);
  BDD If(BDD f, BDD g, BDD h);
  BDD ITE(BDD f, BDD g, BDD h);
  BDD And(BDD f, BDD g);
  BDD Nand(BDD f, BDD g);
  BDD Or(BDD f, BDD g);
  BDD Nor(BDD f, BDD g);
  BDD Xor(BDD f, BDD g);
  BDD Xnor(BDD f, BDD g);
  BDD Exists(BDD f);
  // BDD RelProd(BDD f, BDD g);
  BDD ForAll(BDD f);
  BDD Cofactor(BDD f, BDD c);
  BDD Between(BDD fMin, BDD fMax);
  BDD Reduce(BDD f, BDD c);
  BDD Regular(BDD f);
  BDD SatisfySupport(BDD f);
  double SatisfyingFraction(BDD f);
  BDD Satisfy(BDD f);
  unsigned long Size(BDD f);
  BDD Then(BDD f);
};

// -----------------------------------------------------------------------------
// BDD Class
class BDD {
  friend Cal;

private:
  Cal_BddManager _bddManager;
  Cal_Bdd _bdd;

protected:
  BDD(Cal_BddManager bddManager, Cal_Bdd bdd)
    : _bddManager(bddManager), _bdd(bdd)
  {
    // No need to UnFree(), since it already is reference counted on the return
    // from one of Cal's operations.
  }

public:
  BDD()
    : _bddManager(NULL), _bdd(Cal_BddNull(NULL))
  { }

  BDD(const BDD &other)
    : _bddManager(other._bddManager), _bdd(other._bdd)
  {
    this->UnFree();
  }

  BDD(BDD &&other)
    : _bddManager(other._bddManager), _bdd(other._bdd)
  {
    other._bdd = Cal_BddNull(other._bddManager);
  }

  ~BDD()
  {
    this->Free();
  }

  // ---------------------------------------------------------------------------
  // TODO: Operators functions as member functions?

  // ---------------------------------------------------------------------------
  // Predicates
  bool IsNull() const
  { return Cal_BddIsBddNull(this->_bddManager, this->_bdd); }

  bool IsOne() const
  { return Cal_BddIsBddOne(this->_bddManager, this->_bdd); }

  bool IsZero() const
  { return Cal_BddIsBddZero(this->_bddManager, this->_bdd); }

  bool IsConst() const
  { return Cal_BddIsBddConst(this->_bddManager, this->_bdd); }

  bool IsCube() const
  { return Cal_BddIsCube(this->_bddManager, this->_bdd); }

  bool IsEqual(const BDD &other) const
  { return Cal_BddIsEqual(this->_bddManager, this->_bdd, other._bdd); }

  // ---------------------------------------------------------------------------
  // Node traversal
  Cal_BddId_t Id() const
  { return Cal_BddGetIfId(this->_bddManager, this->_bdd); }

  Cal_BddIndex_t Index() const
  { return Cal_BddGetIfIndex(this->_bddManager, this->_bdd); }

  BDD Then() const
  { return BDD(this->_bddManager, Cal_BddThen(this->_bddManager, this->_bdd)); }

  BDD Else() const
  { return BDD(this->_bddManager, Cal_BddElse(this->_bddManager, this->_bdd)); }

  // ---------------------------------------------------------------------------
  // Declaration of Operation overloads

  BDD& operator= (const BDD &other)
  {
    this->Free();

    this->_bdd = other._bdd;
    this->_bddManager = other._bddManager;

    this->UnFree();

    return *this;
  }

  BDD& operator= (BDD &&other)
  {
    this->Free();

    this->_bdd = other._bdd;
    this->_bddManager = other._bddManager;

    other._bdd = Cal_BddNull(other._bddManager);

    return *this;
  }

  bool operator== (const BDD &other) const
  { return Cal_BddIsEqual(this->_bddManager, this->_bdd, other._bdd); }

  bool  operator!= (const BDD &other) const
  { return !(*this == other); }

  BDD  operator~ () const
  { return BDD(this->_bddManager, Cal_BddNot(this->_bddManager, this->_bdd)); }

  BDD  operator& (const BDD &other) const
  { return BDD(this->_bddManager, Cal_BddAnd(this->_bddManager, this->_bdd, other._bdd)); }

  BDD& operator&= (const BDD &other)
  { return (*this = (*this) & other); }

  BDD  operator| (const BDD &other) const
  { return BDD(this->_bddManager, Cal_BddOr(this->_bddManager, this->_bdd, other._bdd)); }

  BDD& operator|= (const BDD &other)
  { return (*this = (*this) | other); }

  BDD  operator^ (const BDD &other) const
  { return BDD(this->_bddManager, Cal_BddXor(this->_bddManager, this->_bdd, other._bdd)); }

  BDD& operator^= (const BDD &other)
  { return (*this = (*this) ^ other); }

private:
  // ---------------------------------------------------------------------------
  // Memory Management

  inline void
  Free()
  { if (!this->IsNull()) Cal_BddFree(this->_bddManager, this->_bdd); }

  inline void
  UnFree()
  { if (!this->IsNull()) Cal_BddUnFree(this->_bddManager, this->_bdd); }
};

// -----------------------------------------------------------------------------
// Cal member functions
//
// TODO: Sanity check on wrappers
//  - Use the same manager? Otherwise, throw 'not-same-manager' error.
//  - inner Cal_Bdd is not NULL. Otherwise, throw 'out-of-memory' error.

// -----------------------------------------------------------------------------
template<typename BDD_IT>
int Cal::AssociationInit(BDD_IT begin, const BDD_IT end, const bool pairs)
{
  std::vector<Cal_Bdd> c_arg;
  c_arg.reserve(std::distance(begin, end));

  if constexpr(std::is_same_v<typename BDD_IT::value_type, BDD>) {
    while (begin != end) {
      const Cal_Bdd c_bdd = (begin++)->_bdd;
      Cal_BddUnFree(_bddManager, c_bdd);
      c_arg.push_back(c_bdd);
    }
  } else if constexpr (std::is_same_v<typename BDD_IT::value_type, int>) {
    while (begin != end) {
      c_arg.push_back(Cal_BddManagerGetVarWithId(_bddManager, *(begin++)));
    }
  } else {
    static_assert(false, "This type cannot be handled by Cal");
  }

  const BDD end_marker = Null();
  c_arg.push_back(end_marker._bdd);

  const int res = Cal_AssociationInit(_bddManager, c_arg.data(), pairs);

  for (const Cal_Bdd& arg : c_arg) {
    if (Cal_BddIsBddNull(_bddManager, arg) == 0)
      Cal_BddFree(_bddManager, arg);
  }

  return res;
}

void Cal::AssociationQuit(int i)
{ Cal_AssociationQuit(_bddManager, i); }

int Cal::AssociationSetCurrent(int i)
{ return Cal_AssociationSetCurrent(_bddManager, i); }

// -----------------------------------------------------------------------------
BDD Cal::Null() const
{ return BDD(_bddManager, Cal_BddNull(_bddManager)); }

BDD Cal::One() const
{ return BDD(_bddManager, Cal_BddOne(_bddManager)); }

BDD Cal::Zero() const
{ return BDD(_bddManager, Cal_BddZero(_bddManager)); }

BDD Cal::Id(Cal_BddId_t id) const
{ return BDD(_bddManager, Cal_BddManagerGetVarWithId(_bddManager, id)); }

BDD Cal::Index(Cal_BddIndex_t idx) const
{ return BDD(_bddManager, Cal_BddManagerGetVarWithId(_bddManager, idx)); }

// -----------------------------------------------------------------------------
bool Cal::IsNull(BDD f) const
{ return Cal_BddIsBddNull(_bddManager, f._bdd) == 1; }

bool Cal::IsNotNull(BDD f) const
{ return Cal_BddIsBddNull(_bddManager, f._bdd) != 1; }

bool Cal::IsOne(BDD f) const
{ return Cal_BddIsBddOne(_bddManager, f._bdd) == 1; }

bool Cal::IsZero(BDD f) const
{ return Cal_BddIsBddZero(_bddManager, f._bdd) == 1; }

bool Cal::IsConst(BDD f) const
{ return Cal_BddIsBddConst(_bddManager, f._bdd) == 1; }

bool Cal::IsCube(BDD f) const
{ return Cal_BddIsCube(_bddManager, f._bdd) == 1; }

bool Cal::IsEqual(BDD f, BDD g) const
{ return Cal_BddIsEqual(_bddManager, f._bdd, g._bdd) == 1; }

// -----------------------------------------------------------------------------
BDD Cal::Else(BDD f)
{ return BDD(_bddManager, Cal_BddElse(_bddManager, f._bdd)); }

BDD Cal::Not(BDD f)
{ return BDD(_bddManager, Cal_BddNot(_bddManager, f._bdd)); }

BDD Cal::Implies(BDD f, BDD g)
{ return BDD(_bddManager, Cal_BddImplies(_bddManager, f._bdd, g._bdd)); }

BDD Cal::Compose(BDD f, BDD g, BDD h)
{ return BDD(_bddManager, Cal_BddCompose(_bddManager, f._bdd, g._bdd, h._bdd)); }

BDD Cal::If(BDD f, BDD g, BDD h)
{ return BDD(_bddManager, Cal_BddIf(_bddManager, f._bdd)); }

BDD Cal::ITE(BDD f, BDD g, BDD h)
{ return BDD(_bddManager, Cal_BddITE(_bddManager, f._bdd, g._bdd, h._bdd)); }

BDD Cal::And(BDD f, BDD g)
{ return BDD(_bddManager, Cal_BddAnd(_bddManager, f._bdd, g._bdd)); }

BDD Cal::Nand(BDD f, BDD g)
{ return BDD(_bddManager, Cal_BddNand(_bddManager, f._bdd, g._bdd)); }

BDD Cal::Or(BDD f, BDD g)
{ return BDD(_bddManager, Cal_BddOr(_bddManager, f._bdd, g._bdd)); }

BDD Cal::Nor(BDD f, BDD g)
{ return BDD(_bddManager, Cal_BddNor(_bddManager, f._bdd, g._bdd)); }

BDD Cal::Xor(BDD f, BDD g)
{ return BDD(_bddManager, Cal_BddXor(_bddManager, f._bdd, g._bdd)); }

BDD Cal::Xnor(BDD f, BDD g)
{ return BDD(_bddManager, Cal_BddXnor(_bddManager, f._bdd, g._bdd)); }

BDD Cal::Exists(BDD f)
{ return BDD(_bddManager, Cal_BddExists(_bddManager, f._bdd)); }

// BDD Cal::RelProd(BDD f, BDD g) { TODO }

BDD Cal::ForAll(BDD f)
{ return BDD(_bddManager, Cal_BddForAll(_bddManager, f._bdd)); }

BDD Cal::Cofactor(BDD f, BDD c)
{ return BDD(_bddManager, Cal_BddCofactor(_bddManager, f._bdd, c._bdd)); }

BDD Cal::Between(BDD fMin, BDD fMax)
{ return BDD(_bddManager, Cal_BddCofactor(_bddManager, fMin._bdd, fMax._bdd)); }

BDD Cal::Reduce(BDD f, BDD c)
{ return BDD(_bddManager, Cal_BddReduce(_bddManager, f._bdd, c._bdd)); }

BDD Cal::Regular(BDD f)
{ return BDD(_bddManager, Cal_BddGetRegular(_bddManager, f._bdd)); }

BDD Cal::SatisfySupport(BDD f)
{ return BDD(_bddManager, Cal_BddSatisfySupport(_bddManager, f._bdd)); }

double Cal::SatisfyingFraction(BDD f)
{ return Cal_BddSatisfyingFraction(_bddManager, f._bdd); }

BDD Cal::Satisfy(BDD f)
{ return BDD(_bddManager, Cal_BddSatisfy(_bddManager, f._bdd)); }

unsigned long Cal::Size(BDD f)
{ return Cal_BddSize(_bddManager, f._bdd, 0); }

BDD Cal::Then(BDD f)
{ return BDD(_bddManager, Cal_BddThen(_bddManager, f._bdd)); }

#endif // _CALOBJ
