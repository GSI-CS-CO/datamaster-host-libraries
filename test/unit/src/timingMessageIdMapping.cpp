#include <gtest/gtest.h>

// C_M[cpu="0", flags="0x00002202", type="tmsg", toffs="0", pattern="C", patentry="true", patexit="false",
// beamproc="undefined", bpentry="false", bpexit="false", fid="1", gid="4048", evtno="2", beamin="0", sid="0", bpid="0",
// reqnobeam="0", vacc="0", id="0x1fd0002000000000", par="0x0000000000000000", tef="0", shape     = "oval", fillcolor =
// "rosybrown1", penwidth=2, color = "darkorange3"];
#include <graphNodes.h>
#include <idformat.h>

template <typename T>
T CreateNBitMask( int8_t n )
{
  return ( 0x1 << ( n + 1 ) ) - 1;
}

TEST( TimingMessageIdMapping, MergeSubIDsIntoID_AllZeroFormatID0 )
{
  myVertex v;
  v.id_fid      = "0";
  v.id_gid      = "0";
  v.id_evtno    = "0";
  v.id_bin      = "0";
  v.id_sid      = "0";
  v.id_bpid     = "0";
  v.id_bpcstart = "0";
  v.id_reqnob   = "0";
  v.id_vacc     = "0";
  v.id_evtidatt = "0";

  auto id = MergeSubIDsIntoID( v );
  EXPECT_EQ( id, "0x0000000000000000" );
}

TEST( TimingMessageIdMapping, MergeSubIDsIntoID_AllZeroFormatID1 )
{
  myVertex v;
  v.id_fid      = "1";
  v.id_gid      = "0";
  v.id_evtno    = "0";
  v.id_bin      = "0";
  v.id_sid      = "0";
  v.id_bpid     = "0";
  v.id_bpcstart = "0";
  v.id_reqnob   = "0";
  v.id_vacc     = "0";
  v.id_evtidatt = "0";

  auto id = MergeSubIDsIntoID( v );
  EXPECT_EQ( id, "0x1000000000000000" );
}

TEST( TimingMessageIdMapping, MergeSubIDsIntoID_MaxValuesFormatID0 )
{
  myVertex v;
  v.id_fid   = "0";
  v.id_gid   = std::to_string( CreateNBitMask<uint32_t>( 12 ) );
  v.id_evtno = std::to_string( CreateNBitMask<uint32_t>( 12 ) );
  v.id_sid   = std::to_string( CreateNBitMask<uint32_t>( 12 ) );
  v.id_bpid  = std::to_string( CreateNBitMask<uint32_t>( 14 ) );

  auto id = MergeSubIDsIntoID( v );
  EXPECT_EQ( id, "0x0ffffffffffffc00" );
}

TEST( TimingMessageIdMapping, MergeSubIDsIntoID_MaxValuesFormatID1 )
{
  myVertex v;
  v.id_fid      = "1";
  v.id_gid      = std::to_string( CreateNBitMask<uint32_t>( 12 ) );
  v.id_evtno    = std::to_string( CreateNBitMask<uint32_t>( 12 ) );
  v.id_bin      = std::to_string( CreateNBitMask<uint32_t>( 12 ) );
  v.id_bpcstart = std::to_string( CreateNBitMask<uint32_t>( 1 ) );
  v.id_sid      = std::to_string( CreateNBitMask<uint32_t>( 12 ) );
  v.id_bpid     = std::to_string( CreateNBitMask<uint32_t>( 14 ) );
  v.id_reqnob   = std::to_string( CreateNBitMask<uint32_t>( 1 ) );
  v.id_vacc     = std::to_string( CreateNBitMask<uint32_t>( 4 ) );
  v.id_evtidatt = std::to_string( CreateNBitMask<uint32_t>( 6 ) );

  auto id = MergeSubIDsIntoID( v );
  EXPECT_EQ( id, "0x1ffffffcffffffff" );
}

TEST( TimingMessageIdMapping, SplitIDIntoSubIDs_AllZeroFormatID0 )
{
  myVertex v;
  v.name = "testNode";
  v.id   = "0x0000000000000000";

  SplitIDIntoSubIDs( v.id, v );

  EXPECT_EQ( v.id_fid, "0" );
  EXPECT_EQ( v.id_gid, "0" );
  EXPECT_EQ( v.id_evtno, "0" );
  EXPECT_EQ( v.id_bin, "0" );
  EXPECT_EQ( v.id_sid, "0" );
  EXPECT_EQ( v.id_bpid, "0" );
  EXPECT_EQ( v.id_bpcstart, "0" );
  EXPECT_EQ( v.id_reqnob, "0" );
  EXPECT_EQ( v.id_vacc, "0" );
  EXPECT_EQ( v.id_evtidatt, "0" );
}

TEST( TimingMessageIdMapping, SplitIDIntoSubIDs_AllZeroFormatID1 )
{
  myVertex v;
  v.name = "testNode";
  v.id   = "0x1000000000000000";

  SplitIDIntoSubIDs( v.id, v );

  EXPECT_EQ( v.id_fid, "1" );
  EXPECT_EQ( v.id_gid, "0" );
  EXPECT_EQ( v.id_evtno, "0" );
  EXPECT_EQ( v.id_bin, "0" );
  EXPECT_EQ( v.id_sid, "0" );
  EXPECT_EQ( v.id_bpid, "0" );
  EXPECT_EQ( v.id_bpcstart, "0" );
  EXPECT_EQ( v.id_reqnob, "0" );
  EXPECT_EQ( v.id_vacc, "0" );
  EXPECT_EQ( v.id_evtidatt, "0" );
}

TEST( TimingMessageIdMapping, SplitIDIntoSubIDs_MaxValuesFormatID0 )
{
  myVertex v;
  v.name = "testNode";
  v.id   = "0x0ffffffffffffc00";

  SplitIDIntoSubIDs( v.id, v );

  EXPECT_EQ( v.id_fid, "0" );
  EXPECT_EQ( v.id_gid, "4095" );
  EXPECT_EQ( v.id_evtno, "4095" );
  EXPECT_EQ( v.id_bin, "0" );
  EXPECT_EQ( v.id_sid, "4095" );
  EXPECT_EQ( v.id_bpid, "16383" );
  EXPECT_EQ( v.id_bpcstart, "0" );
  EXPECT_EQ( v.id_reqnob, "0" );
  EXPECT_EQ( v.id_vacc, "0" );
  EXPECT_EQ( v.id_evtidatt, "0" );
}

TEST( TimingMessageIdMapping, SplitIDIntoSubIDs_MaxValuesFormatID1 )
{
  myVertex v;
  v.name = "testNode";
  v.id   = "0x1fffffffffffffff";

  SplitIDIntoSubIDs( v.id, v );

  EXPECT_EQ( v.id_fid, "1" );
  EXPECT_EQ( v.id_gid, "4095" );
  EXPECT_EQ( v.id_evtno, "4095" );
  EXPECT_EQ( v.id_bin, "1" );
  EXPECT_EQ( v.id_sid, "4095" );
  EXPECT_EQ( v.id_bpid, "16383" );
  EXPECT_EQ( v.id_bpcstart, "1" );
  EXPECT_EQ( v.id_reqnob, "1" );
  EXPECT_EQ( v.id_vacc, "15" );
  EXPECT_EQ( v.id_evtidatt, "63" );
}

TEST( TimingMessageIdMapping, MergeSubIDsIntoID_RandomExampleFormatID1 )
{
  myVertex v;
  v.id_fid      = "1";
  v.id_gid      = "4048";
  v.id_evtno    = "2";
  v.id_bin      = "0";
  v.id_sid      = "0";
  v.id_bpid     = "0";
  v.id_bpcstart = "0";
  v.id_reqnob   = "0";
  v.id_vacc     = "0";
  v.id_evtidatt = "0";

  auto id = MergeSubIDsIntoID( v );
  EXPECT_EQ( id, "0x1fd0002000000000" );
}

TEST( TimingMessageIdMapping, SplitIDIntoSubIDs_RandomExampleFormatID1 )
{
  myVertex v;
  v.name = "testNode";
  v.id   = "0x1fd0002000000000";

  SplitIDIntoSubIDs( v.id, v );

  EXPECT_EQ( v.id_fid, "1" );
  EXPECT_EQ( v.id_gid, "4048" );
  EXPECT_EQ( v.id_evtno, "2" );
  EXPECT_EQ( v.id_bin, "0" );
  EXPECT_EQ( v.id_sid, "0" );
  EXPECT_EQ( v.id_bpid, "0" );
  EXPECT_EQ( v.id_bpcstart, "0" );
  EXPECT_EQ( v.id_reqnob, "0" );
  EXPECT_EQ( v.id_vacc, "0" );
  EXPECT_EQ( v.id_evtidatt, "0" );
}

TEST( TimingMessageIdMapping, SyncIdSubIds_EmptyID )
{
  myVertex v;
  v.id_fid      = "0";
  v.id_gid      = "0";
  v.id_evtno    = "0";
  v.id_bin      = "0";
  v.id_sid      = "0";
  v.id_bpid     = "0";
  v.id_bpcstart = "0";
  v.id_reqnob   = "0";
  v.id_vacc     = "0";
  v.id_evtidatt = "0";

  SyncIdSubIds( v );
  EXPECT_EQ( v.id, "0x0000000000000000" );
}

TEST( TimingMessageIdMapping, SyncIdSubIds_EmptySubIdsFormatID0 )
{
  myVertex v;
  v.id     = "0x1fffffffffffffff";
  v.id_fid = "0";

  SyncIdSubIds( v );

  EXPECT_EQ( v.id_fid, "1" );
  EXPECT_EQ( v.id_gid, "4095" );
  EXPECT_EQ( v.id_evtno, "4095" );
  EXPECT_EQ( v.id_sid, "4095" );
  EXPECT_EQ( v.id_bpid, "16383" );
}

TEST( TimingMessageIdMapping, SyncIdSubIds_EmptySubIdsFormatID1 )
{
  myVertex v;
  v.name   = "testNode";
  v.id     = "0x1fffffffffffffff";
  v.id_fid = "1";

  SyncIdSubIds( v );

  EXPECT_EQ( v.id_fid, "1" );
  EXPECT_EQ( v.id_gid, "4095" );
  EXPECT_EQ( v.id_evtno, "4095" );
  EXPECT_EQ( v.id_bin, "1" );
  EXPECT_EQ( v.id_sid, "4095" );
  EXPECT_EQ( v.id_bpid, "16383" );
  EXPECT_EQ( v.id_bpcstart, "1" );
  EXPECT_EQ( v.id_reqnob, "1" );
  EXPECT_EQ( v.id_vacc, "15" );
  EXPECT_EQ( v.id_evtidatt, "63" );
}