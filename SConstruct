
env = Environment()

import os
env['ENV']['TERM'] = os.environ['TERM']

env["CXX"] = os.environ.get("CXX","g++")

env.Append(
    CPPPATH=[
        "./src",
    ],
    CCFLAGS=[
        '-O3',
        '-std=c++11',
        '-Wall',
        '-Wextra',
        '-Werror',
    ],
    LIBS=[
        'pthread',
    ],
)

env.Program("runtests",source=Glob("tests/*.cpp"))
env.Command("test_results","runtests","./runtests -a")
