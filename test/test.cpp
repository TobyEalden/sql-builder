#include <cassert>
#include <iostream>
#include <sstream>

#include "sql.h"

/*

create table if not exists user (
        `id` int(10) unsigned not null auto_increment,
        `age` tinyint(8) unsigned,
        `score` int(10) unsigned not null default 0,
    `name` varchar(128) not null default '',
    `address` varchar(256),
    `create_time` datetime not null,
    primary key(`id`)
)

*/

using namespace sql_builder;

int main() {
  InsertModel i;
  i.insert("score", 100)
          ("name", std::string("six"))
          ("age", (unsigned char) 20)
          ("address", "beijing")
          ("create_time", nullptr)
      .into("user");

  std::cout << i.str() << std::endl;

  assert(i.str() == "insert into user(score, name, age, address, create_time) values(?, ?, ?, ?, null)");

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

  assert(s.str()
             == "select distinct id, age, name, address from user join score on (user.id = score.id) and (score.id > ?) where (score > ?) and ((age >= ?) or (address is not null)) group by age having age > ? order by age desc limit 10 offset 1");

  std::vector<int> a = {1, 2, 3};
  UpdateModel u;
  u.update("user")
      .set("name", "ddc")("age", 18)("score", nullptr)("address", "beijing")
      .where(Column("id").in(a));

  std::cout << u.str() << std::endl;

  assert(u.str() == "update user set name = ?, age = ?, score = null, address = ? where id in (?, ?, ?)");

  DeleteModel d;
  d._delete().from("user").where(Column("id") == 1);

  std::cout << d.str() << std::endl;

  assert(d.str() == "delete from user where id = ?");

  return 0;
}
