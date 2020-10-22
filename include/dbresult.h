#ifndef DBRESULT_H
#define DBRESULT_H

#include <any>
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

    void addRow(const DbRow &&row);

    size_t length();
    bool isEmpty();

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

  private:
    size_t mLastInsertedId{};
    size_t mAffectedRows{};
    std::vector<DbRow> rows{};
    std::string mLastError{};
    bool mSuccess = true;
};
} // namespace fiberhttp
#endif // DBRESULT_H
