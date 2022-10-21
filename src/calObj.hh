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

  /*
  Cal(const Cal &o) : _bddManager(o._bddManager)
  { }
  */

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
  // Declaration of BDD Constructors
  BDD One();
  BDD Zero();

  BDD Id(Cal_BddId_t id);
  BDD Index(Cal_BddIndex_t idx);

  // Declaration of BDD Operations
  BDD Not(BDD f);
  BDD Implies(BDD f, BDD g);
  BDD Compose(BDD f, BDD g, BDD h);
  BDD ITE(BDD f, BDD g, BDD h);
  BDD And(BDD f, BDD g);
  BDD Nand(BDD f, BDD g);
  BDD Or(BDD f, BDD g);
  BDD Nor(BDD f, BDD g);
  BDD Xor(BDD f, BDD g);
  BDD Xnor(BDD f, BDD g);
  // BDD Exists(BDD f);
  // BDD RelProd(BDD f, BDD g);
  // BDD ForAll(BDD f);
  BDD Cofactor(BDD f, BDD c);
  BDD Between(BDD fMin, BDD fMax);
  BDD Reduce(BDD f, BDD c);

  // Declaration of other BDD functions
  double SatisfyingFraction(BDD f);
  unsigned long Size(BDD f);
};

// -----------------------------------------------------------------------------
// BDD Class
class BDD {
  friend Cal;

private:
  static constexpr Cal_Bdd NIL = (Cal_Bdd) 0;

  Cal_Bdd _bdd = NIL;
  Cal_BddManager _bddManager = NULL;

protected:
  BDD(Cal_BddManager bddManager, Cal_Bdd bdd) : _bdd(bdd), _bddManager(bddManager)
  {
    // No need to call Cal_BddUnFree, since it already is reference counted on
    // the return from one of Cal's operations.
  }

public:
  BDD()
  { }

  BDD(const BDD &o) : _bdd(o._bdd), _bddManager(o._bddManager)
  { Cal_BddUnFree(_bddManager, _bdd); }

  // TODO: Move Constructor?

  ~BDD()
  { Cal_BddFree(_bddManager, _bdd); }

  // ---------------------------------------------------------------------------
  // TODO: Operators functions as member functions?

  // ---------------------------------------------------------------------------
  // Declaration of Operation overloads

  BDD& operator= (const BDD &other)
  {
    Cal_BddFree(this->_bddManager, this->_bdd);
    this->_bdd = other._bdd;
    Cal_BddUnFree(this->_bddManager, this->_bdd);
    return *this;
  }

  // TODO: Move Assignment?

  bool operator== (const BDD &other) const
  { return Cal_BddIsEqual(this->_bddManager, this->_bdd, other._bdd); }

  bool  operator!= (const BDD &other) const
  { return !Cal_BddIsEqual(this->_bddManager, this->_bdd, other._bdd); }

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
};

// -----------------------------------------------------------------------------
// Cal member functions
//
// TODO: Sanity check on wrappers
//  - Use the same manager? Otherwise, throw 'not-same-manager' error.
//  - inner Cal_Bdd is not BDD::NIL. Otherwise, throw 'out-of-memory' error.

BDD Cal::One()
{ return BDD(_bddManager, Cal_BddOne(_bddManager)); }

BDD Cal::Zero()
{ return BDD(_bddManager, Cal_BddZero(_bddManager)); }

BDD Cal::Id(Cal_BddId_t id)
{ return BDD(_bddManager, Cal_BddManagerGetVarWithId(_bddManager, id)); }

BDD Cal::Index(Cal_BddIndex_t idx)
{ return BDD(_bddManager, Cal_BddManagerGetVarWithId(_bddManager, idx)); }

BDD Cal::Not(BDD f)
{ return BDD(_bddManager, Cal_BddNot(_bddManager, f._bdd)); }

BDD Cal::Implies(BDD f, BDD g)
{ return BDD(_bddManager, Cal_BddImplies(_bddManager, f._bdd, g._bdd)); }

BDD Cal::Compose(BDD f, BDD g, BDD h)
{ return BDD(_bddManager, Cal_BddCompose(_bddManager, f._bdd, g._bdd, h._bdd)); }

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

BDD Cal::Cofactor(BDD f, BDD c)
{ return BDD(_bddManager, Cal_BddCofactor(_bddManager, f._bdd, c._bdd)); }

BDD Cal::Between(BDD fMin, BDD fMax)
{ return BDD(_bddManager, Cal_BddCofactor(_bddManager, fMin._bdd, fMax._bdd)); }

BDD Cal::Reduce(BDD f, BDD c)
{ return BDD(_bddManager, Cal_BddReduce(_bddManager, f._bdd, c._bdd)); }

double Cal::SatisfyingFraction(BDD f)
{ return Cal_BddSatisfyingFraction(_bddManager, f._bdd); }

unsigned long Cal::Size(BDD f)
{ return Cal_BddSize(_bddManager, f._bdd, 0); }

#endif // _CALOBJ
