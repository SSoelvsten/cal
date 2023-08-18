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
  { return BDD::IsNull(this->_bddManager, this->_bdd); }

  bool IsConst() const
  { return Cal_BddIsBddConst(this->_bddManager, this->_bdd); }

  bool IsCube() const
  { return Cal_BddIsCube(this->_bddManager, this->_bdd); }

  bool IsEqualTo(const BDD &other) const
  { return Cal_BddIsEqual(this->_bddManager, this->_bdd, other._bdd); }

  bool DependsOn(BDD var) const
  { return Cal_BddDependsOn(this->_bddManager, this->_bdd, var._bdd); }

  // ---------------------------------------------------------------------------
  // Node traversal and Information

  BDD If() const
  { return BDD(this->_bddManager, Cal_BddIf(this->_bddManager, this->_bdd)); }

  Id_t Id() const
  { return Cal_BddGetIfId(this->_bddManager, this->_bdd); }

  Index_t Index() const
  { return Cal_BddGetIfIndex(this->_bddManager, this->_bdd); }

  BDD Then() const
  { return BDD(this->_bddManager, Cal_BddThen(this->_bddManager, this->_bdd)); }

  BDD Else() const
  { return BDD(this->_bddManager, Cal_BddElse(this->_bddManager, this->_bdd)); }

  enum Type_t
  {
    NONTERMINAL = 0,
    ZERO = 1,
    ONE = 2,
    POSVAR = 3,
    NEGVAR = 4,
    OVERFLOW = 5,
    CONSTANT = 6
  };

  Type_t Type() const
  { return static_cast<Type_t>(Cal_BddType(this->_bddManager, this->_bdd)); }

  unsigned long Size(bool negout = false) const
  { return Cal_BddSize(_bddManager, this->_bdd, 0); }

  double SatisfyingFraction() const
  { return Cal_BddSatisfyingFraction(this->_bddManager, this->_bdd); }

  // ---------------------------------------------------------------------------
  // Operations

  BDD Identity() const
  { return BDD(this->_bddManager, Cal_BddIdentity(this->_bddManager, this->_bdd)); }

  BDD Regular() const
  { return BDD(this->_bddManager, Cal_BddGetRegular(this->_bddManager, this->_bdd)); }

  BDD Not() const
  { return BDD(this->_bddManager, Cal_BddNot(this->_bddManager, this->_bdd)); }

  BDD Compose(const BDD &g, const BDD &h) const
  { return BDD(this->_bddManager, Cal_BddCompose(this->_bddManager, this->_bdd, g._bdd, h._bdd)); }

  BDD Intersects(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddIntersects(this->_bddManager, this->_bdd, g._bdd)); }

  BDD Implies(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddImplies(this->_bddManager, this->_bdd, g._bdd)); }

  BDD ITE(const BDD &g, const BDD &h) const
  { return BDD(this->_bddManager, Cal_BddITE(this->_bddManager, this->_bdd, g._bdd, h._bdd)); }

  BDD And(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddAnd(this->_bddManager, this->_bdd, g._bdd)); }

  BDD Nand(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddNand(this->_bddManager, this->_bdd, g._bdd)); }

  BDD Or(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddOr(this->_bddManager, this->_bdd, g._bdd)); }

  BDD Nor(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddNor(this->_bddManager, this->_bdd, g._bdd)); }

  BDD Xor(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddXor(this->_bddManager, this->_bdd, g._bdd)); }

  BDD Xnor(const BDD &g) const
  { return BDD(this->_bddManager, Cal_BddXnor(this->_bddManager, this->_bdd, g._bdd)); }

  BDD Satisfy() const
  { return BDD(this->_bddManager, Cal_BddSatisfy(this->_bddManager, this->_bdd)); }

  BDD SatisfySupport() const
  { return BDD(this->_bddManager, Cal_BddSatisfySupport(this->_bddManager, this->_bdd)); }

  BDD SwapVars(const BDD &g, const BDD &h) const
  { return BDD(this->_bddManager, Cal_BddSwapVars(this->_bddManager, this->_bdd, g._bdd, h._bdd)); }

  // BDD RelProd(const BDD &g) const

  BDD Cofactor(const BDD &c) const
  { return BDD(this->_bddManager, Cal_BddCofactor(this->_bddManager, this->_bdd, c._bdd)); }

  BDD Reduce(const BDD &c) const
  { return BDD(this->_bddManager, Cal_BddReduce(this->_bddManager, this->_bdd, c._bdd)); }

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
  { return this->IsEqualTo(other); }

  bool  operator!= (const BDD &other) const
  { return !(*this == other); }

  BDD  operator~ () const
  { return this->Not(); }

  BDD  operator& (const BDD &other) const
  { return this->And(other); }

  BDD& operator&= (const BDD &other)
  { return (*this = (*this) & other); }

  BDD  operator| (const BDD &other) const
  { return this->Or(other); }

  BDD& operator|= (const BDD &other)
  { return (*this = (*this) | other); }

  BDD  operator^ (const BDD &other) const
  { return this->Xor(other); }

  BDD& operator^= (const BDD &other)
  { return (*this = (*this) ^ other); }

private:
  // ---------------------------------------------------------------------------
  // Memory Management

  static inline bool
  IsNull(Cal_BddManager bddManager, Cal_Bdd f)
  { return Cal_BddIsBddNull(bddManager, f); }

  static inline void
  Free(Cal_BddManager bddManager, Cal_Bdd f)
  { if (!BDD::IsNull(bddManager, f)) Cal_BddFree(bddManager, f); }

  inline void
  Free()
  { BDD::Free(this->_bddManager, this->_bdd); }

  template<typename IT>
  static inline void
  Free(IT begin, IT end)
  {
    static_assert(std::is_same_v<typename IT::value_type, BDD>,
                  "Must be called with iterator for BDD");

    while (begin != end) (begin++)->Free();
  }

  template<typename IT>
  static inline void
  Free(Cal_BddManager bddManager, IT begin, IT end)
  {
    static_assert(std::is_same_v<typename IT::value_type, Cal_Bdd>,
                  "Must be called with iterator for Cal_Bdd");

    while (begin != end) BDD::Free(bddManager, *(begin++));
  }

  static inline void
  UnFree(Cal_BddManager bddManager, Cal_Bdd f)
  { if (!BDD::IsNull(bddManager, f)) Cal_BddUnFree(bddManager, f); }

  inline void
  UnFree()
  { BDD::UnFree(this->_bddManager, this->_bdd); }

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

protected:
  // ---------------------------------------------------------------------------
  // C interface helper functions
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

    return std::move(out);
  }

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

    return std::move(res);
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
  Cal()
    :_bddManager(Cal_BddManagerInit())
  { }

  Cal(unsigned int numVars)
    : Cal()
  {
    // Create variables
    for (Id_t i = 0; i < numVars; ++i) {
      this->CreateNewVarLast();
    }

    // TODO: Cal_BddManagerSetParameters
  }

  // TODO: copy constructor (requires reference counting)
  Cal(const Cal &o) = delete;

  // TODO: move constructor
  Cal(Cal &&o) = delete;

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

  template<typename IT>
  int AssociationInit(IT begin, IT end, const bool pairs = false)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const int res = Cal_AssociationInit(this->_bddManager, c_arg.data(), pairs);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  void AssociationQuit(int i)
  { Cal_AssociationQuit(_bddManager, i); }

  int AssociationSetCurrent(int i)
  { return Cal_AssociationSetCurrent(_bddManager, i); }

  template<typename IT>
  void TempAssociationInit(IT begin, const IT end, const bool pairs = false)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    Cal_TempAssociationInit(this->_bddManager, c_arg.data(), pairs);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());
  }

  template<typename IT>
  void TempAssociationAugment(IT begin, const IT end, const bool pairs = false)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    Cal_TempAssociationAugment(this->_bddManager, c_arg.data(), pairs);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());
  }

  void TempAssociationQuit()
  { Cal_TempAssociationQuit(this->_bddManager); }

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
  { return BDD(this->_bddManager, Cal_BddManagerGetVarWithIndex(this->_bddManager, idx)); }

  BDD CreateNewVarFirst()
  { return BDD(this->_bddManager, Cal_BddManagerCreateNewVarFirst(this->_bddManager)); }

  BDD CreateNewVarLast()
  { return BDD(this->_bddManager, Cal_BddManagerCreateNewVarLast(this->_bddManager)); }

  BDD CreateNewVarBefore(BDD x)
  { return BDD(this->_bddManager, Cal_BddManagerCreateNewVarBefore(this->_bddManager, x._bdd)); }

  BDD CreateNewVarAfter(BDD x)
  { return BDD(this->_bddManager, Cal_BddManagerCreateNewVarAfter(this->_bddManager, x._bdd)); }

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

  bool DependsOn(BDD f, BDD var) const
  { return f.DependsOn(var); }

  // ---------------------------------------------------------------------------
  // BDD Information
  double SatisfyingFraction(const BDD &f)
  { return f.SatisfyingFraction(); }

  unsigned long Size(const BDD &f, bool negout = false)
  { return f.Size(negout); }

  template<typename IT>
  unsigned long Size(IT begin, IT end, bool negout)
  {
    static_assert(std::is_same_v<typename IT::value_type, BDD>,
                  "Must be called with iterator for BDD");

    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const unsigned long res = Cal_BddSizeMultiple(this->_bddManager, c_arg.data(), negout);

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  template<typename Container>
  unsigned long Size(Container c, bool negout)
  { return Size(std::begin(c), std::end(c), negout); }

  // container_t<BDD> Support(BDD f);

  // ---------------------------------------------------------------------------
  // Manipulation

  BDD Identity(const BDD &f)
  { return f.Identity(); }

  BDD Regular(const BDD &f)
  { return f.Regular(); }

  BDD Not(const BDD &f)
  { return f.Not(); }

  BDD Compose(const BDD &f, const BDD &g, const BDD &h)
  { return f.Compose(g, h); }

  BDD Intersects(const BDD &f, const BDD &g)
  { return f.Intersects(g); }

  BDD Implies(const BDD &f, const BDD &g)
  { return f.Implies(g); }

  BDD ITE(const BDD &f, const BDD &g, const BDD &h)
  { return f.ITE(g, h); }

  BDD And(const BDD &f, const BDD &g)
  { return f.And(g); }

  template<typename IT>
  BDD And(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const BDD res = Cal_BddMultiwayAnd(this->_bddManager, c_arg.data());

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  template<typename Container>
  BDD And(const Container &c)
  { return And(std::begin(c), std::end(c)); }

  BDD Nand(const BDD &f, const BDD &g)
  { return f.Nand(g); }

  BDD Or(const BDD &f, const BDD &g)
  { return f.Or(g); }

  template<typename IT>
  BDD Or(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const BDD res = Cal_BddMultiwayOr(this->_bddManager, c_arg.data());

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  template<typename Container>
  BDD Or(const Container &c)
  { return Or(std::begin(c), std::end(c)); }

  BDD Nor(const BDD &f, const BDD &g)
  { return f.Nor(g); }

  BDD Xor(const BDD &f, const BDD &g)
  { return f.Xor(g); }

  template<typename IT>
  BDD Xor(IT begin, IT end)
  {
    std::vector<Cal_Bdd> c_arg =
      BDD::C_Bdd_vector(this->_bddManager, std::move(begin), std::move(end));

    const BDD res = Cal_BddMultiwayXor(this->_bddManager, c_arg.data());

    BDD::Free(this->_bddManager, c_arg.begin(), c_arg.end());

    return res;
  }

  template<typename Container>
  BDD Xor(const Container &c)
  { return Xor(std::begin(c), std::end(c)); }

  BDD Xnor(const BDD &f, const BDD &g)
  { return f.Xnor(g); }

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

  template<typename Container>
  std::vector<BDD>
  PairwiseAnd(const Container &c)
  { return PairwiseAnd(std::begin(c), std::end(c)); }

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

  template<typename Container>
  std::vector<BDD>
  PairwiseOr(const Container &c)
  { return PairwiseOr(std::begin(c), std::end(c)); }

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

  template<typename Container>
  std::vector<BDD>
  PairwiseXor(const Container &c)
  { return PairwiseXor(std::begin(c), std::end(c)); }

  BDD Satisfy(const BDD &f)
  { return f.Satisfy(); }

  BDD SatisfySupport(const BDD &f)
  { return f.SatisfySupport(); }

  // BDD Substitute(BDD f)

  // BDD VarSubstitute(BDD f)

  BDD SwapVars(const BDD &f, const BDD &g, const BDD &h)
  { return f.SwapVars(g, h); }

  BDD Exists(const BDD &f)
  { return BDD(this->_bddManager, Cal_BddExists(this->_bddManager, f._bdd)); }

  BDD ForAll(const BDD &f)
  { return BDD(this->_bddManager, Cal_BddForAll(this->_bddManager, f._bdd)); }

  BDD RelProd(BDD f, BDD g)
  { return BDD(this->_bddManager, Cal_BddRelProd(this->_bddManager, f._bdd, g._bdd)); }

  BDD Cofactor(const BDD &f, const BDD &c)
  { return f.Cofactor(c); }

  BDD Reduce(const BDD &f, const BDD &c)
  { return f.Reduce(c); }

  BDD Between(const BDD &fMin, const BDD &fMax)
  { return BDD(this->_bddManager, Cal_BddBetween(this->_bddManager, fMin._bdd, fMax._bdd)); }

  // ---------------------------------------------------------------------------
  // BDD Node Access / Traversal

  BDD If(const BDD &f) const
  { return f.If(); }

  Id_t IfId(const BDD &f) const
  { return f.Id(); }

  Index_t IfIndex(const BDD &f) const
  { return f.Index(); }

  BDD Then(const BDD &f)
  { return f.Then(); }

  BDD Else(const BDD &f)
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
