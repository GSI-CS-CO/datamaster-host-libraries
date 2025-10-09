#include "validation.h"
#include "block.h"
#include "dotstr.h"
#include "event.h"
#include "ftm_common.h"
#include "global.h"
#include "meta.h"
#include "node.h"

#include <string>
#include <unordered_map>

namespace n = DotStr::Node::TypeVal;
namespace e = DotStr::Edge::TypeVal;

namespace Validation
{
struct ruleIndex_t
{
  std::string parent;
  std::string edge;
};
} // namespace Validation

namespace
{
using nodeType_t = Validation::nodeType_t;
using edgeType_t = Validation::edgeType_t;
using children_t = Validation::children_t;

enum MaxOccurrance
{
  META  = 2,
  EVENT = 1000,
  DST   = 110,
  DSTLL = 8,
  REF   = 3
};

struct ConstellationRule
{
  children_t children;
  uint32_t   min;
  uint32_t   max;

  ConstellationRule( children_t children, uint32_t min, uint32_t max )
      : children( children )
      , min( min )
      , max( max )
  {
  }
};

vertex_t FindDefDstNode( const Graph& g, vertex_t v )
{
  auto [out_begin, out_end] = out_edges( v, g );
  for ( auto out_cur = out_begin; out_cur != out_end; ++out_cur )
  {
    if ( g[*out_cur].type == e::sDefDst )
    {
      return target( *out_cur, g );
    }
  }
  return null_vertex;
}

} // namespace

// hash for ruleIndex_t
namespace std
{
template <>
struct hash<Validation::ruleIndex_t>
{
  size_t operator()( const Validation::ruleIndex_t& index ) const
  {
    return std::hash<std::string>()( index.parent ) ^ std::hash<std::string>()( index.edge );
  }
};
} // namespace std

namespace Validation
{

// operator == for ruleIndex_t
bool operator==( const ruleIndex_t& lhs, const ruleIndex_t& rhs )
{
  return std::hash<ruleIndex_t>()( lhs ) == std::hash<ruleIndex_t>()( rhs );
}

using ConstellationCnt_set = std::unordered_map<ruleIndex_t, int32_t>;
std::unordered_map<ruleIndex_t, ConstellationRule> cRules;

decltype( cRules )::const_iterator getConstellationRule( const ruleIndex_t& index )
{
  auto it = cRules.find( index );
  if ( it != cRules.end() )
  {
    return it;
  }
  return cRules.end();
}

void addConstellationRule( ruleIndex_t index, ConstellationRule rule )
{
  cRules.emplace( std::move( index ), std::move( rule ) );
}

void addConstellationCnt( ConstellationCnt_set& s, const ruleIndex_t& index )
{
  auto it = s.find( index );
  if ( it == s.end() )
  {
    s.emplace( std::make_pair( index, 1 ) );
  }
  else
  {
    it->second++;
  }
}

int32_t getConstellationCnt( const ConstellationCnt_set& s, const ruleIndex_t& index )
{
  auto it = s.find( index );
  if ( it != s.end() )
  {
    return it->second;
  }
  return 0;
}

void init()
{
  const children_t cNonMeta = { n::sTMsg,     n::sCmdNoop, n::sCmdFlow,    n::sOrigin,     n::sStartThread, n::sSwitch,
                                n::sCmdFlush, n::sCmdWait, n::sBlockFixed, n::sBlockAlign, n::sGlobal };

  // TMsg
  addConstellationRule( ruleIndex_t{ n::sTMsg, e::sDefDst }, ConstellationRule( cNonMeta, 1, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sTMsg, e::sDynPar0 }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sTMsg, e::sDynPar1 }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sTMsg, e::sDyn[DYN_MODE_ADR] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  addConstellationRule( ruleIndex_t{ n::sTMsg, e::sDyn[DYN_MODE_REF] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  addConstellationRule( ruleIndex_t{ n::sTMsg, e::sDyn[DYN_MODE_REF2] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  addConstellationRule( ruleIndex_t{ n::sTMsg, e::sAdr }, ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  addConstellationRule( ruleIndex_t{ n::sTMsg, e::sWrite }, ConstellationRule( cNonMeta, 0, 1 ) );

  // CmdNoop
  addConstellationRule( ruleIndex_t{ n::sCmdNoop, e::sDefDst }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sCmdNoop, e::sCmdTarget },
                        ConstellationRule( { n::sBlock, n::sBlockFixed, n::sBlockAlign }, 0, 1 ) );

  // CmdFlow
  addConstellationRule( ruleIndex_t{ n::sCmdFlow, e::sDefDst }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sCmdFlow, e::sCmdTarget },
                        ConstellationRule( { n::sBlock, n::sBlockFixed, n::sBlockAlign }, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sCmdFlow, e::sCmdFlowDst }, ConstellationRule( cNonMeta, 0, 1 ) );

  // Switch
  addConstellationRule( ruleIndex_t{ n::sSwitch, e::sDefDst }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sSwitch, e::sSwitchTarget },
                        ConstellationRule( { n::sBlock, n::sBlockFixed, n::sBlockAlign }, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sSwitch, e::sSwitchDst }, ConstellationRule( cNonMeta, 0, 1 ) );

  // Origin
  addConstellationRule( ruleIndex_t{ n::sOrigin, e::sDefDst }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sOrigin, e::sOriginDst }, ConstellationRule( cNonMeta, 1, 1 ) );

  addConstellationRule( ruleIndex_t{ n::sStartThread, e::sDefDst }, ConstellationRule( cNonMeta, 0, 1 ) );

  // Flush
  addConstellationRule( ruleIndex_t{ n::sCmdFlush, e::sDefDst }, ConstellationRule( cNonMeta, 1, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sCmdFlush, e::sCmdTarget },
                        ConstellationRule( { n::sBlock, n::sBlockFixed, n::sBlockAlign }, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sCmdFlush, e::sCmdFlushOvr }, ConstellationRule( cNonMeta, 0, 1 ) );

  // Wait
  addConstellationRule( ruleIndex_t{ n::sCmdWait, e::sDefDst }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sCmdWait, e::sCmdTarget },
                        ConstellationRule( { n::sBlock, n::sBlockFixed, n::sBlockAlign }, 0, 1 ) );

  // Block
  addConstellationRule( ruleIndex_t{ n::sBlockFixed, e::sDefDst }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sBlockFixed, e::sAltDst },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::DST ) );
  addConstellationRule( ruleIndex_t{ n::sBlockFixed, e::sDyn[DYN_MODE_ADR] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  addConstellationRule( ruleIndex_t{ n::sBlockFixed, e::sDyn[DYN_MODE_REF] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  addConstellationRule( ruleIndex_t{ n::sBlockFixed, e::sDyn[DYN_MODE_REF2] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  // addConstellationRule(ConstellationRule(n::sBlockFixed,  e::sDstList,    {n::sDstList}, 0, 1
  // ));
  addConstellationRule( ruleIndex_t{ n::sBlockFixed, e::sQPrio[PRIO_IL] }, ConstellationRule( { n::sQInfo }, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sBlockFixed, e::sQPrio[PRIO_HI] }, ConstellationRule( { n::sQInfo }, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sBlockFixed, e::sQPrio[PRIO_LO] }, ConstellationRule( { n::sQInfo }, 0, 1 ) );

  // Block Align
  addConstellationRule( ruleIndex_t{ n::sBlockAlign, e::sDefDst }, ConstellationRule( cNonMeta, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sBlockAlign, e::sAltDst },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::DST ) );
  addConstellationRule( ruleIndex_t{ n::sBlockAlign, e::sDyn[DYN_MODE_ADR] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  addConstellationRule( ruleIndex_t{ n::sBlockAlign, e::sDyn[DYN_MODE_REF] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  addConstellationRule( ruleIndex_t{ n::sBlockAlign, e::sDyn[DYN_MODE_REF2] },
                        ConstellationRule( cNonMeta, 0, MaxOccurrance::REF ) );
  // addConstellationRule(ConstellationRule(n::sBlockAlign,  e::sDstList,    {n::sDstList}, 0, 1
  // ));
  addConstellationRule( ruleIndex_t{ n::sBlockAlign, e::sQPrio[PRIO_IL] }, ConstellationRule( { n::sQInfo }, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sBlockAlign, e::sQPrio[PRIO_HI] }, ConstellationRule( { n::sQInfo }, 0, 1 ) );
  addConstellationRule( ruleIndex_t{ n::sBlockAlign, e::sQPrio[PRIO_LO] }, ConstellationRule( { n::sQInfo }, 0, 1 ) );

  addConstellationRule( ruleIndex_t{ n::sQInfo, e::sMeta },
                        ConstellationRule( { n::sQBuf }, MaxOccurrance::META, MaxOccurrance::META ) );
  addConstellationRule( ruleIndex_t{ n::sDstList, e::sDefDst },
                        ConstellationRule( { n::sBlock, n::sBlockFixed, n::sBlockAlign }, 1, 1 ) );
}

// check if all outedge (nodetype/edgetype/childtype) tupels are valid and occurrence count is within valid bounds
void neighbourhoodCheck( vertex_t v, Graph& g )
{
  // ConstellationCnt_set     cCnt;
  Validation::ConstellationCnt_set cCnt;
  const std::string                exIntro = "Neighbourhood: Node '" + g[v].name + "' of type '" + g[v].type;

  if ( g[v].np == nullptr )
    throw std::runtime_error( exIntro + "' was found unallocated\n" );

  auto [out_begin, out_end] = out_edges( v, g );
  if ( ( out_begin == out_end ) && ( g[v].np->isEvent() || ( g[v].type == n::sQInfo ) || g[v].type == n::sDstList ) )
  { // found a childless node. Events and certain meta nodes cannot exist like this
    throw std::runtime_error( exIntro + "' cannot be childless\n" );
  }

  auto [in_begin, in_end] = in_edges( v, g );
  if ( ( in_begin == in_end ) && g[v].np->isMeta() && ( g[v].type != n::sDstList ) )
  { // found an orphan node. most meta nodes cannot exist like this
    throw std::runtime_error( exIntro + "' cannot be an orphan\n" );
  }

  // Check connection duplicates
  for ( auto out_cur = out_begin; out_cur != out_end; ++out_cur )
  {
    vertex_t   vChk = target( *out_cur, g );
    edgeType_t et   = g[*out_cur].type;
    unsigned   cnt  = 0;
    for ( auto out_chk = out_begin; out_chk != out_end; ++out_chk )
    {
      if ( ( vChk == target( *out_chk, g ) ) && ( et == g[*out_chk].type ) )
      {
        cnt++;
      }

      // Check if a command's assigned priority matches a queue on the target block.
      if ( ( g[v].np->isCmd() ) && ( g[vChk].np->isBlock() ) && ( et == e::sCmdTarget ) )
      {
        // check if the target block has an outedge of corresponding type. we cannot allow commands to non existent
        // queues
        uint16_t   priolvl2chk       = std::dynamic_pointer_cast<Command>( g[v].np )->getPrio();
        edgeType_t priochk_et        = e::sQPrio[std::dynamic_pointer_cast<Command>( g[v].np )->getPrio()];
        bool       foundMatchingPrio = false;

        auto [priochk_begin, priochk_end] = out_edges( vChk, g );
        for ( auto priochk_cur = priochk_begin; priochk_cur != priochk_end; ++priochk_cur )
        {
          if ( g[*priochk_cur].type == e::sQPrio[priolvl2chk] )
          {
            foundMatchingPrio = true;
            break;
          }
        }
        if ( !foundMatchingPrio )
          throw std::runtime_error( exIntro + "' must not target non existing queue priority '" +
                                    std::to_string( priolvl2chk ) + "' on block '" + g[vChk].name + "'\n" );
      }
    }
    /// Important ! ///
    if ( cnt > 3 )
      throw std::runtime_error( exIntro + "' must not have multiple edges of type '" + et + "' to Node '" +
                                g[vChk].name + "' of type '" + g[vChk].type + "'\n" );
  }

  for ( auto out_cur = out_begin; out_cur != out_end; ++out_cur )
  {
    ruleIndex_t constellationRuleIndex{ g[v].type, g[*out_cur].type };
    auto        it = Validation::getConstellationRule( constellationRuleIndex );
    // if node type/edge type combo is not found, this is already invalid
    if ( it == cRules.end() )
    {
      throw std::runtime_error( exIntro + "' must not have edge of type '" + g[*out_cur].type + "'\n" );
    }
    // if necessary, add the counter for this constellation
    addConstellationCnt( cCnt, constellationRuleIndex );

    // if the child's type is not in the set of allowable children, this is invalid
    if ( it->second.children.count( g[target( *out_cur, g )].type ) < 1 )
    {
      throw std::runtime_error( exIntro + "' with edge of type '" + g[*out_cur].type +
                                "' must not have children of type '" + g[target( *out_cur, g )].type + "'\n" );
    }
  }
  // check all exisiting constellation counts against rule min/max
  for ( auto [index, count] : cCnt )
  {
    auto itRules = Validation::getConstellationRule( index );

    std::string possibleChildren;
    for ( auto itPCh : itRules->second.children )
      possibleChildren += ( itPCh + ", " );

    if ( ( count < itRules->second.min ) | ( count > itRules->second.max ) )
    {
      throw std::runtime_error( exIntro + "' must have between " + std::to_string( itRules->second.min ) + " and " +
                                std::to_string( itRules->second.max ) + " edge(s) of type '" + index.edge +
                                "'' connected to children of type(s) '" + possibleChildren + "', found " +
                                std::to_string( count ) + "\n" );
    }
  }
}

void neighbourhoodCheckCpu( vertex_t v, Graph& g )
{
  auto [out_begin, out_end] = out_edges( v, g );
  std::string cpuSource     = g[v].cpu;
  for ( auto out_cur = out_begin; out_cur != out_end; ++out_cur )
  {
    if ( g[*out_cur].type == e::sDefDst || g[*out_cur].type == e::sAltDst || g[*out_cur].type == e::sDynPar0 ||
         g[*out_cur].type == e::sDynPar1 || g[*out_cur].type == e::sDynId || g[*out_cur].type == e::sOriginDst )
    {
      if ( g[target( *out_cur, g )].cpu != cpuSource )
      {
        throw std::runtime_error( "Neighbourhood: Nodes '" + g[v].name + "' (CPU " + g[v].cpu + ") and '" +
                                  g[target( *out_cur, g )].name + "' (CPU " + g[target( *out_cur, g )].cpu +
                                  ") must have the same CPU." );
      }
    }
  }
}

// check if event sequence is well behaved
void eventSequenceCheck( vertex_t v, Graph& g, bool force )
{
  vertex_t     vcurrent          = v;
  unsigned int infiniteLoopGuard = 0;

  std::string exIntroBase = "Event Sequence: Node '";
  std::string exIntro;

  while ( infiniteLoopGuard < MaxOccurrance::EVENT )
  {
    // find the child connected to this node's defdest
    auto defDstNodeIndex = FindDefDstNode( g, vcurrent );
    if ( defDstNodeIndex == null_vertex )
    {
      return;
    }

    exIntro = exIntroBase + g[vcurrent].name + "' of type '" + g[vcurrent].type + "' must not ";
    // check for forbidden properties:
    if ( ( g[defDstNodeIndex].type == n::sBlockFixed ) || ( g[defDstNodeIndex].type == n::sBlockAlign ) )
    { // found a block
      // check for forbidden properties:

      if ( g[vcurrent].np == nullptr )
        throw std::runtime_error( g[vcurrent].name + " is not allocated\n" );
      if ( g[defDstNodeIndex].np == nullptr )
        throw std::runtime_error( g[defDstNodeIndex].name + " is not allocated\n" );
      if ( !force )
      { // this allows negative values for time offsets to provoke specific late events
        // assumption: successors are either of class Event or Block (neighbourhoodCheck ensures this)
        if ( std::dynamic_pointer_cast<Event>( g[vcurrent].np )->getTOffs() >=
             std::dynamic_pointer_cast<Block>( g[defDstNodeIndex].np )->getTPeriod() )
        { // time offset greater or equal block period
          throw std::runtime_error( exIntro +
                                    "have a time offset greater of equal than the period of its terminating block\n" );
        }
      }
      return; // found a valid block, we're done.
    }

    if ( vcurrent == defDstNodeIndex )
    { // self reference of event-class nodes
      throw std::runtime_error( exIntro + "loop back on itself\n" );
    }
    // assumption: successors are of class events (neighbourhoodCheck and block checks above ensure this)
    if ( defDstNodeIndex == v )
    { // loop to original vertex without encountering block termination
      throw std::runtime_error( exIntro + "be part of a loop without a terminating block\n" );
    }
    if ( !force )
    { // this allows negative values for time offsets to provoke specific late events
      if ( std::dynamic_pointer_cast<Event>( g[vcurrent].np )->getTOffs() >
           std::dynamic_pointer_cast<Event>( g[defDstNodeIndex].np )->getTOffs() )
      { // non monotonically increasing time offsets
        throw std::runtime_error( exIntro + "have a time offset greater than its successor's\n" );
      }
    }

    vcurrent = defDstNodeIndex;
    infiniteLoopGuard++;
  }
  throw std::runtime_error( exIntroBase + g[v].name + "' of type '" + g[v].type +
                            "' is probably part of an infinite loop ( iteration cnt > " +
                            std::to_string( MaxOccurrance::EVENT ) + ")\n" );

  // useless, but eases my mind.
  return;
}

namespace Aux
{
void metaSequenceCheckAux( vertex_t v, vertex_t vcurrent, Graph& g, unsigned int recursionLvl /*= 0*/ )
{
  if ( recursionLvl + 1 > MaxOccurrance::META )
    throw std::runtime_error( "have more than " + std::to_string( MaxOccurrance::META ) +
                              " levels of children\n" ); // too many child levels
  auto [out_begin, out_end] = out_edges( vcurrent, g );
  for ( auto out_cur = out_begin; out_cur != out_end; ++out_cur )
  {
    auto vnext = target( *out_cur, g );
    if ( vnext == vcurrent || vnext == v )
    {
      throw std::runtime_error( "be part of a loop\n" );
    } // loop to original vertex
    metaSequenceCheckAux( v, vnext, g, recursionLvl + 1 );
  }

  return;
}
} // namespace Aux

void metaSequenceCheck( vertex_t v, Graph& g )
{
  std::string exIntroBase = "Meta Sequence: Node '" + g[v].name + "' of type '" + g[v].type + "' must not ";
  std::string exIntro;

  try
  {
    Aux::metaSequenceCheckAux( v, v, g );
  }
  catch ( const std::runtime_error& err )
  {
    throw std::runtime_error( exIntroBase + std::string( err.what() ) );
  }
}

} // namespace Validation
