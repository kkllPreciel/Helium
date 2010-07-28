#ifndef litesql_gen_ruby_activerecord_hpp
#define litesql_gen_ruby_activerecord_hpp
#include "litesql_char.hpp"
#include "generator.hpp"

namespace litesql {

  class RubyActiveRecordGenerator : public CompositeGenerator {
  public:
    RubyActiveRecordGenerator();
  };

  class ActiveRecordClassGenerator : public CodeGenerator {
  public:
    ActiveRecordClassGenerator(): CodeGenerator(LITESQL_L("ruby-activerecord-class")) { };
    bool generateCode(const ObjectModel* model);
  protected:   
    bool generate(xml::Object* const object);
  };

  class RubyMigrationsGenerator : public CodeGenerator {
  public:
    RubyMigrationsGenerator(): CodeGenerator(LITESQL_L("ruby-activerecord-migrations")) { };
    bool generateCode(const ObjectModel* model);
  };

}

#endif