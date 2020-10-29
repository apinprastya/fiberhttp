#ifndef DBRESULT_H
#define DBRESULT_H

#include <any>
#include <string>
#include <vector>

namespace fiberhttp {
class DbRow {
  public:
    DbRow();
    ~DbRow();

    template <typename T> inline void addColumn(const T &t) { data.push_back(t); }

    inline std::any &column(int index) { return data[index]; }

  private:
    std::vector<std::any> data;
};

class DbResult {
  public:
    DbResult();
    ~DbResult();

    inline void addRow(const DbRow &&row) { rows.push_back(std::move(row)); }

    inline size_t length() { return rows.size(); }

    inline bool isEmpty() { return rows.empty(); }

    inline void setLastError(const std::string &error) { mLastError = error; }

    inline std::string lastError() { return mLastError; }

    inline void setLastInsertedId(size_t t) { mLastInsertedId = t; }

    inline size_t lastInsertedId() { return mLastInsertedId; }

    inline DbRow rowAt(int index) const { return rows[index]; }

    inline bool success() { return mSuccess; }

    inline void setSuccess(bool val) { mSuccess = val; }

    inline void setError(const std::string &err) {
        mLastError = err;
        mSuccess = false;
    }

    inline void setAffectedRows(size_t t) { mAffectedRows = t; }
    size_t affectedRows() { return mAffectedRows; }

    inline void pushColumnName(const std::string &name) { columnNames.push_back(name); }

    private:
    size_t mLastInsertedId{};
    size_t mAffectedRows{};
    std::vector<std::string> columnNames;
    std::vector<DbRow> rows{};
    std::string mLastError{};
    bool mSuccess = true;
};
} // namespace fiberhttp
#endif // DBRESULT_H
