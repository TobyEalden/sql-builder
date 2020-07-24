# qt-sql-builder

Some slight modifications to [six-ddc/sql-builder](https://github.com/six-ddc/sql-builder) to add Qt support, and
specifically safe parameterised queries via QSqlQuery.

Build up the query as usual, passing the variable data to the various overrides, e.g.

```c++
std::string myKey = "foo';DROP TABLE kvp;";
SelectModel s;
s.select("key", "value").from("kvp").where(Column("key") == myKey);
```

The sql builder will insert placeholders for each variable and then use QSqlQuery::addBindValue after the `prepare`
phase to bind the correct variables to the query in a safe manner.

## Examples:

```c++
  QSqlQuery query;

  InsertModel i;
  i.insert("score", 100)
          ("name", std::string("six"))
          ("age", (unsigned char) 20)
          ("address", "beijing")
          ("create_time", nullptr)
      .into("user");

  std::cout << i.str() << std::endl;

  assert(i.str() == "insert into user(score, name, age, address, create_time) values(?, ?, ?, ?, null)");

  // Will perform the `prepare` followed by binding the values and then `exec`.
  if (!i.exec(query)) {
    assert(false);
  }

  SelectModel s;
  s.select("id", "age", "name", "address")
      .distinct()
      .from("user")
      .join("score")
      .on(Column("user.id") == Column("score.id") and Column("score.id") > 60)
      .where(Column("score") > 60 and (Column("age") >= 20 or Column("address").is_not_null()))
      .group_by("age")
      .having(Column("age") > 10)
      .order_by("age desc")
      .limit(10)
      .offset(1);

  std::cout << s.str() << std::endl;

  assert(s.str() == "select distinct id, age, name, address from user join score on (user.id = score.id) and (score.id > ?) where (score > ?) and ((age >= ?) or (address is not null)) group by age having age > ? order by age desc limit 10 offset 1");

  // Will perform the `prepare` followed by binding the values and then `exec`.
  if (!s.exec(query)) {
    assert(false);
  }

  std::vector<int> a = {1, 2, 3};
  UpdateModel u;
  u.update("user")
      .set("name", "ddc")("age", 18)("score", nullptr)("address", "beijing")
      .where(Column("id").in(a));

  std::cout << u.str() << std::endl;

  assert(u.str() ==
      "update user set name = ?, age = ?, score = null, address = ? where id in (?, ?, ?)");

  // Will perform the `prepare` followed by binding the values and then `exec`.
  if (!u.exec(query)) {
    assert(false);
  }

  DeleteModel d;
  d._delete().from("user").where(Column("id") == 1);

  std::cout << d.str() << std::endl;

  assert(d.str() == "delete from user where id = ?");

  // Will perform the `prepare` followed by binding the values and then `exec`.
  if (!d.exec(query)) {
    assert(false);
  }
```
