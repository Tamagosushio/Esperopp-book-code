#include "epp/ast/node.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace epp::ast {

NodeId Ast::AddNode(Node node) {
  throw std::logic_error("not yet implemented: Ast::AddNode");
}

const Node& Ast::GetNode(NodeId node_id) const {
  return nodes_.at(node_id.index);
}

std::size_t Ast::NodeCount() const noexcept {
  return nodes_.size();
}

} // namespace epp::ast
