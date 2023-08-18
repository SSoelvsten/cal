#ifndef _CALOBJ
#define _CALOBJ

extern "C" {
#include <cal.h>
#include <calInt.h>
}

// -----------------------------------------------------------------------------
// Forward Declarations
class BDD;
class Cal;

// -----------------------------------------------------------------------------
// BDD Class
class BDD {
  friend Cal;

public:
  using Id_t = Cal_BddId_t;
  using Index_t = Cal_BddIndex_t;

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
  bool IsOne() const
  { return Cal_BddIsBddOne(this->_bddManager, this->_bdd); }

  bool IsZero() const
  { return Cal_BddIsBddZero(this->_bddManager, this->_bdd); }

  bool IsNull() const
  { return Cal_BddIsBddNull(this->_bddManager, this->_bdd); }

  bool IsConst() const
  { return Cal_BddIsBddConst(this->_bddManager, this->_bdd); }

  bool IsCube() const
  { return Cal_BddIsCube(this->_bddManager, this->_bdd); }

  bool IsEqualTo(const BDD &other) const
  { return Cal_BddIsEqual(this->_bddManager, this->_bdd, other._bdd); }

  // ---------------------------------------------------------------------------
  // Node traversal and Information
  Id_t Id() const
  { return Cal_BddGetIfId(this->_bddManager, this->_bdd); }

  Index_t Index() const
  { return Cal_BddGetIfIndex(this->_bddManager, this->_bdd); }

  BDD Then() const
  { return BDD(this->_bddManager, Cal_BddThen(this->_bddManager, this->_bdd)); }

  BDD Else() const
  { return BDD(this->_bddManager, Cal_BddElse(this->_bddManager, this->_bdd)); }


  enum Type
  {
    NONTERMINAL = 0,
    ZERO = 1,
    ONE = 2,
    POSVAR = 3,
    NEGVAR = 4,
    OVERFLOW = 5,
    CONSTANT = 6
  };

  Type BddType()
  { return static_cast<Type>(Cal_BddType(this->_bddManager, this->_bdd)); }

  unsigned long Size()
  { return Cal_BddSize(_bddManager, this->_bdd, 0); }

  // ---------------------------------------------------------------------------
  // Operations

  // TODO

  // ---------------------------------------------------------------------------
  // Operation Overloading

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
  { return IsEqualTo(other); }

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

public:
  // ---------------------------------------------------------------------------
  // Debugging
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
};

// -----------------------------------------------------------------------------
// BDD Manager
class Cal {
  friend BDD;

  // ---------------------------------------------------------------------------
  // Types
  using Bdd_t = BDD;
  using Id_t = BDD::Id_t;
  using Index_t = BDD::Index_t;

  // TODO:

  // ---------------------------------------------------------------------------
  // Fields
private:
  // TODO: std::shared_ptr for reference counting the Cal_BddManager
  Cal_BddManager _bddManager;

  // ---------------------------------------------------------------------------
  // Constructors
public:
  // Reenable after adding NewVarLast to API
  Cal() = delete;

  Cal(unsigned int numVars) : _bddManager(Cal_BddManagerInit())
  {
    // Create variables
    for (Id_t i = 0; i < numVars; ++i){
      Cal_BddManagerCreateNewVarLast(this->_bddManager);
    }

    // TODO: Cal_BddManagerSetParameters
  }

  // TODO: allow multiple copies to access Cal (requires reference counting)
  Cal(const Cal &o) = delete;

  // TODO: move constructor

  ~Cal()
  {
    Cal_BddManagerQuit(this->_bddManager);
  }

  // ---------------------------------------------------------------------------
  // Settings + Statistics

  unsigned long GetNumNodes()
  { return Cal_BddManagerGetNumNodes(this->_bddManager); }

  long Vars()
  { return Cal_BddVars(this->_bddManager); }

  bool Overflow()
  { return Cal_BddOverflow(this->_bddManager); }

  unsigned long TotalSize()
  { return Cal_BddTotalSize(this->_bddManager); }

  void Stats(FILE* fp = stdout)
  { Cal_BddStats(this->_bddManager, fp); }

  // ---------------------------------------------------------------------------
  // Memory and Garbage Collection

  long NodeLimit(long newLimit)
  { return Cal_BddNodeLimit(this->_bddManager, newLimit); }

  void SetGCMode(bool enableGC)
  { Cal_BddSetGCMode(this->_bddManager, enableGC); }

  void SetGCLimit()
  { Cal_BddManagerSetGCLimit(this->_bddManager); }

  void GC()
  { Cal_BddManagerGC(this->_bddManager); }

  // ---------------------------------------------------------------------------
  // Reordering

  enum ReorderTechnique {
    NONE = CAL_REORDER_NONE,
    SIFT = CAL_REORDER_SIFT,
    WINDOW = CAL_REORDER_WINDOW
  };

  enum ReorderMethod {
    BF = CAL_REORDER_METHOD_BF,
    DF = CAL_REORDER_METHOD_DF
  };

  void DynamicReordering(ReorderTechnique technique, ReorderMethod method = ReorderMethod::DF)
  {
    Cal_BddDynamicReordering(_bddManager, technique, method);
  }

  void Reorder()
  { Cal_BddReorder(_bddManager); }

  // SwapVars(BDD x, BDD y);

  // ---------------------------------------------------------------------------
  // Declaration of Association List

  template<typename BDD_IT>
  int AssociationInit(BDD_IT begin, const BDD_IT end, const bool pairs = false)
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

  void AssociationQuit(int i)
  { Cal_AssociationQuit(_bddManager, i); }

  int AssociationSetCurrent(int i)
  { return Cal_AssociationSetCurrent(_bddManager, i); }

  // void TempAssociationInit(BDD_IT begin, const BDD_IT end, const bool pairs = false)

  // void TempAssociationAugment(BDD_IT begin, const BDD_IT end, const bool pairs = false)

  // void TempAssociationQuit()

  // ---------------------------------------------------------------------------
  // Save / Load BDDs

  // BDD UndumpBdd(IT vars_begin, IT vars_end, FILE *f, int &error)
  // BDD DumpBdd(BDD f, IT vars_begin, IT vars_end, FILE *f, int &error)

  // ---------------------------------------------------------------------------
  // BDD Constructors
  BDD Null() const
  { return BDD(this->_bddManager, Cal_BddNull(this->_bddManager)); }

  BDD One() const
  { return BDD(this->_bddManager, Cal_BddOne(this->_bddManager)); }

  BDD Zero() const
  { return BDD(this->_bddManager, Cal_BddZero(this->_bddManager)); }

  BDD Id(Id_t id) const
  { return BDD(this->_bddManager, Cal_BddManagerGetVarWithId(this->_bddManager, id)); }

  BDD Index(Index_t idx) const
  { return BDD(this->_bddManager, Cal_BddManagerGetVarWithId(this->_bddManager, idx)); }

  // BDD CreateNewVarFirst();
  // BDD CreateNewVarFirst();
  // BDD CreateNewVarBefore(BDD x);
  // BDD CreateNewVarAfter(BDD x);
  // BDD CreateNewVarWithIndex(BDD x, Index_t index);
  // BDD CreateNewVarWithId(Id_t id);

  // ---------------------------------------------------------------------------
  // BDD Predicates
  bool IsNull(BDD f) const
  { return f.IsNull(); }

  bool IsOne(BDD f) const
  { return f.IsOne(); }

  bool IsZero(BDD f) const
  { return f.IsZero(); }

  bool IsConst(BDD f) const
  { return f.IsConst(); }

  bool IsCube(BDD f) const
  { return f.IsCube(); }

  bool IsEqual(BDD f, BDD g) const
  { return f.IsEqualTo(g); }

  // bool DependsOn(BDD f, BDD vars) const;

  // ---------------------------------------------------------------------------
  // BDD Information
  double SatisfyingFraction(BDD f)
  { return Cal_BddSatisfyingFraction(this->_bddManager, f._bdd); }

  unsigned long Size(BDD f)
  { return f.Size(); }

  // unsigned long Size(IT begin, IT end, bool negout); (using MultipleSize)

  // container_t<BDD> Support(BDD f);

  // ---------------------------------------------------------------------------
  // Manipulation

  // BDD Identity(BDD f) const

  BDD Regular(BDD f)
  { return BDD(this->_bddManager, Cal_BddGetRegular(this->_bddManager, f._bdd)); }

  BDD Not(BDD f)
  { return BDD(this->_bddManager, Cal_BddNot(this->_bddManager, f._bdd)); }

  BDD Compose(BDD f, BDD g, BDD h)
  { return BDD(this->_bddManager, Cal_BddCompose(this->_bddManager, f._bdd, g._bdd, h._bdd)); }

  // BDD Intersects(BDD f, BDD g)

  BDD Implies(BDD f, BDD g)
  { return BDD(this->_bddManager, Cal_BddImplies(this->_bddManager, f._bdd, g._bdd)); }

  BDD ITE(BDD f, BDD g, BDD h)
  { return BDD(this->_bddManager, Cal_BddITE(this->_bddManager, f._bdd, g._bdd, h._bdd)); }

  BDD And(BDD f, BDD g)
  { return BDD(this->_bddManager, Cal_BddAnd(this->_bddManager, f._bdd, g._bdd)); }

  // BDD And(IT begin, IT end); (using MultiwayAnd)

  BDD Nand(BDD f, BDD g)
  { return BDD(this->_bddManager, Cal_BddNand(this->_bddManager, f._bdd, g._bdd)); }

  BDD Or(BDD f, BDD g)
  { return BDD(this->_bddManager, Cal_BddOr(this->_bddManager, f._bdd, g._bdd)); }

  // BDD Or(IT begin, IT end); (using MultiwayOr)

  BDD Nor(BDD f, BDD g)
  { return BDD(this->_bddManager, Cal_BddNor(this->_bddManager, f._bdd, g._bdd)); }

  BDD Xor(BDD f, BDD g)
  { return BDD(this->_bddManager, Cal_BddXor(this->_bddManager, f._bdd, g._bdd)); }

  // BDD Xor(IT begin, IT end); (using MuliwayXor)

  BDD Xnor(BDD f, BDD g)
  { return BDD(this->_bddManager, Cal_BddXnor(this->_bddManager, f._bdd, g._bdd)); }

  // container_t<BDD> PairwiseAnd(IT begin, IT end)
  // container_t<BDD> PairwiseOr(IT begin, IT end)
  // container_t<BDD> PairwiseXor(IT begin, IT end)

  BDD Satisfy(BDD f)
  { return BDD(this->_bddManager, Cal_BddSatisfy(this->_bddManager, f._bdd)); }

  BDD SatisfySupport(BDD f)
  { return BDD(this->_bddManager, Cal_BddSatisfySupport(this->_bddManager, f._bdd)); }

  // BDD Substitute(BDD f)

  // BDD VarSubstitute(BDD f)

  // BDD SwapVars(BDD f, BDD x, BDD y)

  BDD Exists(BDD f)
  { return BDD(this->_bddManager, Cal_BddExists(this->_bddManager, f._bdd)); }

  BDD ForAll(BDD f)
  { return BDD(this->_bddManager, Cal_BddForAll(this->_bddManager, f._bdd)); }

  // BDD RelProd(BDD f, BDD g)

  BDD Cofactor(BDD f, BDD c)
  { return BDD(this->_bddManager, Cal_BddCofactor(this->_bddManager, f._bdd, c._bdd)); }

  BDD Reduce(BDD f, BDD c)
  { return BDD(this->_bddManager, Cal_BddReduce(this->_bddManager, f._bdd, c._bdd)); }

  BDD Between(BDD fMin, BDD fMax)
  { return BDD(this->_bddManager, Cal_BddBetween(this->_bddManager, fMin._bdd, fMax._bdd)); }

  // ---------------------------------------------------------------------------
  // BDD Node Access / Traversal

  BDD If(BDD f)
  { return BDD(this->_bddManager, Cal_BddIf(this->_bddManager, f._bdd)); }

  // Id_t IfId(BDD f) const

  // Index_t IfIndex(BDD f) const

  BDD Then(BDD f)
  { return f.Then(); }

  BDD Else(BDD f)
  { return f.Else(); }

  // ---------------------------------------------------------------------------
  // BDD Pipelining

  // TODO: class Pipeline

  // ---------------------------------------------------------------------------
  // NOTE: These should never be exposed (hidden inside of the BDD class)

  // void Free();
  // void UnFree();
};

#endif /* _CALOBJ */
