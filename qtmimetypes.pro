TEMPLATE = subdirs

module_lib.subdir = lib
module_lib.target = module_lib

module_tests.file = tests/tests.pro
module_tests.target = module_tests
module_tests.depends = module_lib

module_examples.file = examples/examples.pro
module_examples.target = module_examples
module_examples.depends = module_lib

exists(lib/lib.pro): SUBDIRS += module_lib
exists(tests/tests.pro): SUBDIRS += module_tests
exists(examples/examples.pro): SUBDIRS += module_examples
