#include "my_types.h"

#include <set>
#include <vector>

namespace ct {

enum class ParseStruct {
  Finished,
  Incomplete
};

// тут строка нашего запроса. до этого писал через size_t index, но показалось что через итератор как-то полегче и
// поестественнее, поэтому через итератор
struct TextView {
  std::string_view sv;
  size_t it = 0;

  TextView(std::string_view s)
      : sv(s) {}

  bool eof() const {
    return it >= sv.size();
  }

  char peek() const {
    return sv[it];
  }

  void advance() {
    if (!eof()) {
      ++it;
    }
  }
};

void skip_spaces(TextView& txt);

std::string collect_until_any(TextView& txt, const std::set<char>& stops);

// в строке out  нас лежит наше дополнение
// функцией getfunc идём до '(', собирая имя функции. Если не было '(', то дополняем имя функции
std::optional<std::string> get_func(TextView& txt, const std::vector<std::string>& funcs, std::string& out);

// функцией getarg делаем что-то похожее, но идём до знака =
// в целом их можно объединить, но в целях читабельности и понятности сделал их разными
std::optional<std::string> get_arg(TextView& txt, std::set<std::string>& funcs, std::string& out);

// либо дополняем символом, который щас должен быть, либо переходим к следующему символу
bool append_if_missing(TextView& t, char ch, std::string& s);

// у parse-struct инвариант такой: начинаем её парсить, когда мы на первом символе после = (которое до структуры)
// и выходим из этой функции, когда указатель будет на следующем за закрывающей для этой структуры '}'
// тем самым в этой функции корректно распарситя/дополнится только эта структура и ничего лишнего.
// возвращаемый тип тут означает, закончилась ли эта структура/дополнили ли мы её до конца, чтобы ожидали
// или дополнили запятой или закрывающей скобкой
ParseStruct fill_structure(std::string& out, std::string_view type, TextView& txt, const Schema& sch);

// если у нас число, то по умолчанию считаю, что оно не закончено
ParseStruct fill_primitive(std::string_view type, TextView& txt, std::string& out, const Schema& sch);

// аналогично parse_struct
void parse_function(std::string& s, std::string_view fnName, TextView& text, const Schema& sch);

std::string autocomplete(std::string_view input, const Schema& sch);
} // namespace ct
