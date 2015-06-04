
env = Environment()

import os
env['ENV']['TERM'] = os.environ['TERM']

env["CXX"] = os.environ.get("CXX","clang++")

env.Append(
    CPPPATH=[
        "./src",
    ],
    CPPDEFINES=[
        'GTEST_LANG_CXX11',
    ],
    CCFLAGS=[
        '-O3',
        '-std=c++11',
        '-Wall',
        '-Wextra',
        '-Werror',
    ],
    LIBS=[
        'gtest',
        'pthread',
    ],
)

env.Program("runtests",source='tests/test.cpp')
env.Command("test_results","runtests","./runtests")
