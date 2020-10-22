#include <dbresult.h>
#include <iostream>

namespace fiberhttp {
DbRow::DbRow() {}
DbRow::~DbRow() {}

DbResult::DbResult() {
    // std::cout << "CALLED" << std::endl;
}
DbResult::~DbResult() {}

void DbResult::addRow(const DbRow &&row) { rows.push_back(std::move(row)); }

size_t DbResult::length() { return rows.size(); }

bool DbResult::isEmpty() { return rows.empty(); }

} // namespace fiberhttp