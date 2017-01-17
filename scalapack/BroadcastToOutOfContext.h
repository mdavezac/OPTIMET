#ifndef OPTIMET_BROADCAST_TO_OUT_OF_CONTEXT_H
#define OPTIMET_BROADCAST_TO_OUT_OF_CONTEXT_H

#include "Types.h"
#include "mpi/Collectives.h"
#include "mpi/Communicator.h"
#include "scalapack/Context.h"

namespace optimet {
namespace mpi {
#ifdef OPTIMET_SCALAPACK
//! \brief Broadcast data from a proc in the context to procs outside the context
//! \details Usefull if some procs are not part of the context but still require the data.
template <class T>
void broadcast_to_out_of_context(T &inout, scalapack::Context const &context,
                                 mpi::Communicator const &comm) {
  auto const is_in_context = comm.all_gather<int>(context.is_valid());
  auto is_true = [](int input) { return input; };
  auto is_false = [](int input) { return not input; };
  // All procs in context, nothing to do
  if(std::all_of(is_in_context.begin(), is_in_context.end(), is_true))
    return;
  bool const is_root =
      is_in_context[comm.rank()] and
      std::all_of(is_in_context.begin(), is_in_context.begin() + comm.rank(), is_false);
  auto const is_in_group = is_root or not is_in_context[comm.rank()];
  auto const split = comm.split(is_in_group, is_root ? 0 : 1);
  if(is_in_group)
    inout = split.broadcast(inout, 0);
}
#else
//! Broadcast data (vectors, matrices from a proc in the context to procs outside the context)
template <class T>
void broadcast_to_out_of_context(T &, scalapack::Context const &, mpi::Communicator const &) {}
#endif
}
namespace scalapack {
using optimet::mpi::broadcast_to_out_of_context;
}
}
#endif
