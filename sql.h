/**
 * based on the most excellent  https://github.com/six-ddc/sql-builder
 */
#pragma once

#include <QSqlQuery>
#include <QVariantList>
#include <string>
#include <vector>

namespace sql_builder {

class Column;

template <typename T>
inline bool is_empty(const T& /* data */) {
  return false;
}

template <>
inline bool is_empty<int>(const int& data) {
  return data == 0;
}

template <>
inline bool is_empty<std::string>(const std::string& data) {
  return data.empty();
}

//
// variants
//
template <typename T>
inline QVariant to_variant(const T& data) {
  return data;
}

template <>
inline QVariant to_variant<std::string>(const std::string& data) {
  return data.c_str();
}

template <>
inline QVariant to_variant<Column>(const Column& data);

template <typename T>
void join_vector(std::string& result, const std::vector<T>& vec,
                 const char* sep) {
  size_t size = vec.size();
  for (size_t i = 0; i < size; ++i) {
    if (i < size - 1) {
      result.append(vec[i]);
      result.append(sep);
    } else {
      result.append(vec[i]);
    }
  }
}

class Column {
 public:
  Column(const std::string& column) { _cond = column; }
  virtual ~Column() {}

  Column& as(const std::string& s) {
    _cond.append(" as ");
    _cond.append(s);
    return *this;
  }

  Column& is_null() {
    _cond.append(" is null");
    return *this;
  }

  Column& is_not_null() {
    _cond.append(" is not null");
    return *this;
  }

  template <typename T>
  Column& in(const std::vector<T>& args) {
    size_t size = args.size();
    if (size == 1) {
      _cond.append(" = ?");
      _bindings.push_back(to_variant(args[0]));
    } else {
      _cond.append(" in (");
      for (size_t i = 0; i < size; ++i) {
        if (i < size - 1) {
          _cond.append("?, ");
        } else {
          _cond.append("?");
        }
        _bindings.push_back(to_variant(args[i]));
      }
      _cond.append(")");
    }
    return *this;
  }

  template <typename T>
  Column& not_in(const std::vector<T>& args) {
    size_t size = args.size();
    if (size == 1) {
      _cond.append(" != ?");
      _bindings.push_back(to_variant(args[0]));
    } else {
      _cond.append(" not in (");
      for (size_t i = 0; i < size; ++i) {
        if (i < size - 1) {
          _cond.append("?, ");
        } else {
          _cond.append("?");
        }
        _bindings.push_back(to_variant(args[i]));
      }
      _cond.append(")");
    }
    return *this;
  }

  Column& operator&&(Column& condition) {
    std::string str("(");
    str.append(_cond);
    str.append(") and (");
    str.append(condition._cond);
    str.append(")");
    condition._cond = str;
    condition._bindings = _bindings + condition._bindings;
    return condition;
  }

  Column& operator||(Column& condition) {
    std::string str("(");
    str.append(_cond);
    str.append(") or (");
    str.append(condition._cond);
    str.append(")");
    condition._cond = str;
    condition._bindings = _bindings + condition._bindings;
    return condition;
  }

  Column& operator&&(const std::string& condition) {
    _cond.append(" and ");
    _cond.append(condition);
    return *this;
  }

  Column& operator||(const std::string& condition) {
    _cond.append(" or ");
    _cond.append(condition);
    return *this;
  }

  Column& operator&&(const char* condition) {
    _cond.append(" and ");
    _cond.append(condition);
    return *this;
  }

  Column& operator||(const char* condition) {
    _cond.append(" or ");
    _cond.append(condition);
    return *this;
  }

  template <typename T>
  Column& operator==(const T& data) {
    _cond.append(" = ?");
    _bindings.push_back(to_variant(data));
    return *this;
  }

  template <>
  Column& operator==(Column const& data) {
    _cond.append(" = ");
    _cond.append(data.str());
    return *this;
  }

  template <typename T>
  Column& operator!=(const T& data) {
    _cond.append(" != ?");
    _bindings.push_back(to_variant(data));
    return *this;
  }

  template <>
  Column& operator!=(Column const& data) {
    _cond.append(" != ");
    _cond.append(data.str());
    return *this;
  }

  template <typename T>
  Column& operator>=(const T& data) {
    _cond.append(" >= ?");
    _bindings.push_back(to_variant(data));
    return *this;
  }

  template <>
  Column& operator>=(Column const& data) {
    _cond.append(" >= ");
    _cond.append(data.str());
    return *this;
  }

  template <typename T>
  Column& operator<=(const T& data) {
    _cond.append(" <= ?");
    _bindings.push_back(to_variant(data));
    return *this;
  }

  template <>
  Column& operator<=(Column const& data) {
    _cond.append(" <= ");
    _cond.append(data.str());
    return *this;
  }

  template <typename T>
  Column& operator>(const T& data) {
    _cond.append(" > ?");
    _bindings.push_back(to_variant(data));
    return *this;
  }

  template <>
  Column& operator>(Column const& data) {
    _cond.append(" > ");
    _cond.append(data.str());
    return *this;
  }

  template <typename T>
  Column& operator<(const T& data) {
    _cond.append(" < ?");
    _bindings.push_back(to_variant(data));
    return *this;
  }

  template <>
  Column& operator<(Column const& data) {
    _cond.append(" < ");
    _cond.append(data.str());
    return *this;
  }

  const std::string& str() const { return _cond; }
  QVariantList const& bindings() const { return _bindings; }

  operator bool() { return true; }

 private:
  std::string _cond;
  QVariantList _bindings;
};

class SqlModel {
 public:
  SqlModel() {}
  virtual ~SqlModel() {}

  virtual const std::string& str() = 0;
  virtual bool exec(QSqlQuery& query) = 0;
  const std::string& last_sql() { return _sql; }

 private:
  SqlModel(const SqlModel& m) = delete;
  SqlModel& operator=(const SqlModel& data) = delete;

 protected:
  std::string _sql;
};

class SelectModel : public SqlModel {
 public:
  SelectModel() : _distinct(false) {}
  virtual ~SelectModel() {}

  template <typename... Args>
  SelectModel& select(const std::string& str, Args&&... columns) {
    _select_columns.push_back(str);
    select(columns...);
    return *this;
  }

  // for recursion
  SelectModel& select() { return *this; }

  SelectModel& distinct() {
    _distinct = true;
    return *this;
  }

  template <typename... Args>
  SelectModel& from(const std::string& table_name, Args&&... tables) {
    if (_table_name.empty()) {
      _table_name = table_name;
    } else {
      _table_name.append(", ");
      _table_name.append(table_name);
    }
    from(tables...);
    return *this;
  }

  // for recursion
  SelectModel& from() { return *this; }

  SelectModel& join(const std::string& table_name) {
    _join_type = "join";
    _join_table = table_name;
    return *this;
  }

  SelectModel& left_join(const std::string& table_name) {
    _join_type = "left join";
    _join_table = table_name;
    return *this;
  }

  SelectModel& left_outer_join(const std::string& table_name) {
    _join_type = "left outer join";
    _join_table = table_name;
    return *this;
  }

  SelectModel& right_join(const std::string& table_name) {
    _join_type = "right join";
    _join_table = table_name;
    return *this;
  }

  SelectModel& right_outer_join(const std::string& table_name) {
    _join_type = "right outer join";
    _join_table = table_name;
    return *this;
  }

  SelectModel& full_join(const std::string& table_name) {
    _join_type = "full join";
    _join_table = table_name;
    return *this;
  }

  SelectModel& full_outer_join(const std::string& table_name) {
    _join_type = "full outer join";
    _join_table = table_name;
    return *this;
  }

  SelectModel& on(const std::string& condition) {
    _join_on_condition.push_back(condition);
    return *this;
  }

  SelectModel& on(const Column& condition) {
    _join_on_condition.push_back(condition.str());
    _join_on_bindings.append(condition.bindings());
    return *this;
  }

  SelectModel& where(const std::string& condition) {
    _where_condition.push_back(condition);
    return *this;
  }

  SelectModel& where(const Column& condition) {
    _where_condition.push_back(condition.str());
    _where_bindings.append(condition.bindings());
    return *this;
  }

  template <typename... Args>
  SelectModel& group_by(const std::string& str, Args&&... columns) {
    _groupby_columns.push_back(str);
    group_by(columns...);
    return *this;
  }

  // for recursion
  SelectModel& group_by() { return *this; }

  SelectModel& having(const std::string& condition) {
    _having_condition.push_back(condition);
    return *this;
  }

  SelectModel& having(const Column& condition) {
    _having_condition.push_back(condition.str());
    _having_bindings.append(condition.bindings());
    return *this;
  }

  SelectModel& order_by(const std::string& order_by) {
    _order_by = order_by;
    return *this;
  }

  template <typename T>
  SelectModel& limit(const T& limit) {
    _limit = std::to_string(limit);
    return *this;
  }

  template <typename T>
  SelectModel& limit(const T& offset, const T& limit) {
    _offset = std::to_string(offset);
    _limit = std::to_string(limit);
    return *this;
  }

  template <typename T>
  SelectModel& offset(const T& offset) {
    _offset = std::to_string(offset);
    return *this;
  }

  virtual const std::string& str() override {
    _sql.clear();
    _sql.append("select ");
    if (_distinct) {
      _sql.append("distinct ");
    }
    join_vector(_sql, _select_columns, ", ");
    _sql.append(" from ");
    _sql.append(_table_name);
    if (!_join_type.empty()) {
      _sql.append(" ");
      _sql.append(_join_type);
      _sql.append(" ");
      _sql.append(_join_table);
    }
    if (!_join_on_condition.empty()) {
      _sql.append(" on ");
      join_vector(_sql, _join_on_condition, " and ");
    }
    if (!_where_condition.empty()) {
      _sql.append(" where ");
      join_vector(_sql, _where_condition, " and ");
    }
    if (!_groupby_columns.empty()) {
      _sql.append(" group by ");
      join_vector(_sql, _groupby_columns, ", ");
    }
    if (!_having_condition.empty()) {
      _sql.append(" having ");
      join_vector(_sql, _having_condition, " and ");
    }
    if (!_order_by.empty()) {
      _sql.append(" order by ");
      _sql.append(_order_by);
    }
    if (!_limit.empty()) {
      _sql.append(" limit ");
      _sql.append(_limit);
    }
    if (!_offset.empty()) {
      _sql.append(" offset ");
      _sql.append(_offset);
    }

    return _sql;
  }

  bool exec(QSqlQuery& query) override {
    auto const& sql = str();
    if (!query.prepare(sql.c_str())) {
      return false;
    }

    for (auto const& it : _join_on_bindings) {
      query.addBindValue(it);
    }

    for (auto const& it : _where_bindings) {
      query.addBindValue(it);
    }

    for (auto const& it : _having_bindings) {
      query.addBindValue(it);
    }

    return query.exec();
  }

  SelectModel& reset() {
    _select_columns.clear();
    _distinct = false;
    _groupby_columns.clear();
    _table_name.clear();
    _join_type.clear();
    _join_table.clear();
    _join_on_condition.clear();
    _join_on_bindings.clear();
    _where_condition.clear();
    _where_bindings.clear();
    _having_condition.clear();
    _having_bindings.clear();
    _order_by.clear();
    _limit.clear();
    _offset.clear();
    return *this;
  }
  friend inline std::ostream& operator<<(std::ostream& out, SelectModel& mod) {
    out << mod.str();
    return out;
  }

 protected:
  std::vector<std::string> _select_columns;
  bool _distinct;
  std::vector<std::string> _groupby_columns;
  std::string _table_name;
  std::string _join_type;
  std::string _join_table;
  std::vector<std::string> _join_on_condition;
  QVariantList _join_on_bindings;
  std::vector<std::string> _where_condition;
  QVariantList _where_bindings;
  std::vector<std::string> _having_condition;
  QVariantList _having_bindings;
  std::string _order_by;
  std::string _limit;
  std::string _offset;
};

class InsertModel : public SqlModel {
 public:
  InsertModel() {}
  virtual ~InsertModel() {}

  template <typename T>
  InsertModel& insert(const std::string& c, const T& data) {
    _columns.push_back(c);
    _values.push_back("?");
    _value_bindings.push_back(to_variant(data));
    return *this;
  }

  template <typename T>
  InsertModel& operator()(const std::string& c, const T& data) {
    return insert(c, data);
  }

  InsertModel& into(const std::string& table_name) {
    _table_name = table_name;
    return *this;
  }

  InsertModel& replace(bool var) {
    _replace = var;
    return *this;
  }

  virtual const std::string& str() override {
    _sql.clear();
    std::string v_ss;

    if (_replace) {
      _sql.append("insert or replace into ");
    } else {
      _sql.append("insert into ");
    }

    _sql.append(_table_name);
    _sql.append("(");
    v_ss.append(" values(");
    size_t size = _columns.size();
    for (size_t i = 0; i < size; ++i) {
      if (i < size - 1) {
        _sql.append(_columns[i]);
        _sql.append(", ");
        v_ss.append(_values[i]);
        v_ss.append(", ");
      } else {
        _sql.append(_columns[i]);
        _sql.append(")");
        v_ss.append(_values[i]);
        v_ss.append(")");
      }
    }
    _sql.append(v_ss);

    return _sql;
  }

  bool exec(QSqlQuery& query) override {
    auto const& sql = str();
    if (!query.prepare(sql.c_str())) {
      return false;
    }

    for (auto const& it : _value_bindings) {
      query.addBindValue(it);
    }

    return query.exec();
  }

  InsertModel& reset() {
    _table_name.clear();
    _columns.clear();
    _values.clear();
    _value_bindings.clear();
    return *this;
  }

  friend inline std::ostream& operator<<(std::ostream& out, InsertModel& mod) {
    out << mod.str();
    return out;
  }

 protected:
  bool _replace = false;
  std::string _table_name;
  std::vector<std::string> _columns;
  std::vector<std::string> _values;
  QVariantList _value_bindings;
};

template <>
inline InsertModel& InsertModel::insert(const std::string& c,
                                        const std::nullptr_t&) {
  _columns.push_back(c);
  _values.push_back("null");
  return *this;
}

class UpdateModel : public SqlModel {
 public:
  UpdateModel() {}
  virtual ~UpdateModel() {}

  UpdateModel& update(const std::string& table_name) {
    _table_name = table_name;
    return *this;
  }

  template <typename T>
  UpdateModel& set(const std::string& c, const T& data,
                   bool ignoreIfEmpty = false) {
    if (ignoreIfEmpty && is_empty(data)) {
      return *this;
    }

    std::string str(c);
    str.append(" = ?");
    _set_columns.push_back(str);
    _set_bindings.push_back(to_variant(data));
    return *this;
  }

  template <typename T>
  UpdateModel& operator()(const std::string& c, const T& data) {
    return set(c, data);
  }

  UpdateModel& where(const std::string& condition) {
    _where_condition.push_back(condition);
    return *this;
  }

  UpdateModel& where(const Column& condition) {
    _where_condition.push_back(condition.str());
    _where_bindings.append(condition.bindings());
    return *this;
  }

  virtual const std::string& str() override {
    _sql.clear();
    _sql.append("update ");
    _sql.append(_table_name);
    _sql.append(" set ");
    join_vector(_sql, _set_columns, ", ");
    size_t size = _where_condition.size();
    if (size > 0) {
      _sql.append(" where ");
      join_vector(_sql, _where_condition, " and ");
    }

    return _sql;
  }

  bool exec(QSqlQuery& query) override {
    auto const& sql = str();
    if (!query.prepare(sql.c_str())) {
      return false;
    }

    for (auto const& it : _set_bindings) {
      query.addBindValue(it);
    }

    for (auto const& it : _where_bindings) {
      query.addBindValue(it);
    }

    return query.exec();
  }

  UpdateModel& reset() {
    _table_name.clear();
    _set_columns.clear();
    _set_bindings.clear();
    _where_condition.clear();
    _where_bindings.clear();
    return *this;
  }
  friend inline std::ostream& operator<<(std::ostream& out, UpdateModel& mod) {
    out << mod.str();
    return out;
  }

 protected:
  std::vector<std::string> _set_columns;
  QVariantList _set_bindings;
  std::string _table_name;
  std::vector<std::string> _where_condition;
  QVariantList _where_bindings;
};

template <>
inline UpdateModel& UpdateModel::set(const std::string& c,
                                     const std::nullptr_t&, bool) {
  std::string str(c);
  str.append(" = null");
  _set_columns.push_back(str);
  return *this;
}

class DeleteModel : public SqlModel {
 public:
  DeleteModel() {}
  virtual ~DeleteModel() {}

  DeleteModel& _delete() { return *this; }

  template <typename... Args>
  DeleteModel& from(const std::string& table_name, Args&&... tables) {
    if (_table_name.empty()) {
      _table_name = table_name;
    } else {
      _table_name.append(", ");
      _table_name.append(table_name);
    }
    from(tables...);
    return *this;
  }

  // for recursion
  DeleteModel& from() { return *this; }

  DeleteModel& where(const std::string& condition) {
    _where_condition.push_back(condition);
    return *this;
  }

  DeleteModel& where(const Column& condition) {
    _where_condition.push_back(condition.str());
    _where_bindings.append(condition.bindings());
    return *this;
  }

  virtual const std::string& str() override {
    _sql.clear();
    _sql.append("delete from ");
    _sql.append(_table_name);
    size_t size = _where_condition.size();
    if (size > 0) {
      _sql.append(" where ");
      join_vector(_sql, _where_condition, " and ");
    }

    return _sql;
  }

  bool exec(QSqlQuery& query) override {
    auto const& sql = str();
    if (!query.prepare(sql.c_str())) {
      return false;
    }

    for (auto const& it : _where_bindings) {
      query.addBindValue(it);
    }

    return query.exec();
  }

  DeleteModel& reset() {
    _table_name.clear();
    _where_condition.clear();
    _where_bindings.clear();
    return *this;
  }
  friend inline std::ostream& operator<<(std::ostream& out, DeleteModel& mod) {
    out << mod.str();
    return out;
  }

 protected:
  std::string _table_name;
  std::vector<std::string> _where_condition;
  QVariantList _where_bindings;
};

}  // namespace sql_builder