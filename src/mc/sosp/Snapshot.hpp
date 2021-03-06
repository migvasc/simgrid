/* Copyright (c) 2007-2020. The SimGrid Team. All rights reserved.          */

/* This program is free software; you can redistribute it and/or modify it
 * under the terms of the license (GNU LGPL) which comes with this package. */

#ifndef SIMGRID_MC_SNAPSHOT_HPP
#define SIMGRID_MC_SNAPSHOT_HPP

#include "src/mc/ModelChecker.hpp"
#include "src/mc/inspect/mc_unw.hpp"
#include "src/mc/remote/RemoteClient.hpp"
#include "src/mc/sosp/Region.hpp"

// ***** MC Snapshot

/** Ignored data
 *
 *  Some parts of the snapshot are ignored by zeroing them out: the real
 *  values is stored here.
 * */
struct s_mc_snapshot_ignored_data_t {
  void* start;
  std::vector<char> data;
};

/** Information about a given stack frame */
struct s_mc_stack_frame_t {
  /** Instruction pointer */
  unw_word_t ip;
  /** Stack pointer */
  unw_word_t sp;
  unw_word_t frame_base;
  simgrid::mc::Frame* frame;
  std::string frame_name;
  unw_cursor_t unw_cursor;
};
typedef s_mc_stack_frame_t* mc_stack_frame_t;

struct s_local_variable_t {
  simgrid::mc::Frame* subprogram;
  unsigned long ip;
  std::string name;
  simgrid::mc::Type* type;
  void* address;
};
typedef s_local_variable_t* local_variable_t;
typedef const s_local_variable_t* const_local_variable_t;

struct XBT_PRIVATE s_mc_snapshot_stack_t {
  std::vector<s_local_variable_t> local_variables;
  simgrid::mc::UnwindContext context;
  std::vector<s_mc_stack_frame_t> stack_frames;
};
typedef s_mc_snapshot_stack_t* mc_snapshot_stack_t;
typedef const s_mc_snapshot_stack_t* const_mc_snapshot_stack_t;

namespace simgrid {
namespace mc {

class XBT_PRIVATE Snapshot final : public AddressSpace {
public:
  /* Initialization */
  Snapshot(int num_state, RemoteClient* process = &mc_model_checker->process());
  ~Snapshot() = default;

  /* Regular use */
  bool on_heap(const void* address) const
  {
    const s_xbt_mheap_t* heap = process()->get_heap();
    return address >= heap->heapbase && address < heap->breakval;
  }

  void* read_bytes(void* buffer, std::size_t size, RemotePtr<void> address,
                   ReadOptions options = ReadOptions::none()) const override;
  Region* get_region(const void* addr) const;
  Region* get_region(const void* addr, Region* hinted_region) const;
  void restore(RemoteClient* process);

  // To be private
  int num_state_;
  std::size_t heap_bytes_used_ = 0;
  std::vector<std::unique_ptr<Region>> snapshot_regions_;
  std::set<pid_t> enabled_processes_;
  std::vector<std::size_t> stack_sizes_;
  std::vector<s_mc_snapshot_stack_t> stacks_;
  std::vector<simgrid::mc::IgnoredHeapRegion> to_ignore_;
  std::uint64_t hash_ = 0;
  std::vector<s_mc_snapshot_ignored_data_t> ignored_data_;

private:
  void add_region(RegionType type, ObjectInformation* object_info, void* start_addr, std::size_t size);
  void snapshot_regions(simgrid::mc::RemoteClient* process);
  void snapshot_stacks(simgrid::mc::RemoteClient* process);
};
} // namespace mc
} // namespace simgrid

#endif
